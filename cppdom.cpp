/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil c-basic-offset: 3 -*- */
// vim:cindent:ts=3:sw=3:et:tw=80:sta:
/*************************************************************** cppdom-cpr beg
 *
 * cppdom was forked from the original xmlpp version 0.6 under the LGPL. This
 * new, branched xmlpp is under the same LGPL (of course) and is being
 * maintained by:
 *      Kevin Meinert   <subatomic@users.sourceforge.net>
 *      Allen Bierbaum  <allenb@users.sourceforge.net>
 *      Ben Scott       <nonchocoboy@users.sourceforge.net>
 *
 * -----------------------------------------------------------------
 *
 * xmlpp - an xml parser and validator written in C++
 * copyright (c) 2000-2001 Michael Fink
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ************************************************************ cppdom-cpr-end */
/*! \file cppdom.cpp

  contains the methods of the xmlpp classes

*/

#include <iostream>
#include <fstream>

// needed includes
#include <cppdom/cppdom.h>
#include <cppdom/xmlparser.h>
#include <cppdom/predicates.h>

// namespace declaration
namespace cppdom
{
   // Error methods

   Error::Error(ErrorCode code, std::string localDesc, std::string location)
      : mErrorCode(code), mLocalDesc(localDesc), mLocation(location)
   {}

   Error::Error(ErrorCode code, std::string localDesc, std::string file, unsigned line_num)
      : mErrorCode(code), mLocalDesc(localDesc)
   {
      std::stringstream location_stream;
      location_stream << file << ":" << line_num;
      mLocation = location_stream.str();
   }

   ErrorCode Error::getError() const
   {
      return mErrorCode;
   }

   std::string Error::getStrError() const
   {
   // macro for keeping the errorcode switch short and easy
#define XMLERRORCODE(x,y)  case x: err = y; break;

      const char *err;
      switch(mErrorCode)
      {
         XMLERRORCODE(xml_unknown,"unspecified or unknown error");
         XMLERRORCODE(xml_instream_error,"error in the infile stream");
         XMLERRORCODE(xml_opentag_expected,"expected an open tag literal '<'");
         XMLERRORCODE(xml_opentag_cdata_expected,"expected a '<' or cdata");
         XMLERRORCODE(xml_closetag_expected,"expected an '>' closing tag literal");
         XMLERRORCODE(xml_pi_doctype_expected,"expected a processing instruction or doctype tag");
         XMLERRORCODE(xml_tagname_expected,"expected a tag name after '<' or '</'");
         XMLERRORCODE(xml_closetag_slash_expected,"expected a '/' after closing tag literal '<'");
         XMLERRORCODE(xml_tagname_close_mismatch,"tag name from start and end tag mismatch");
         XMLERRORCODE(xml_attr_equal_expected,"expected '=' after attribute name");
         XMLERRORCODE(xml_attr_value_expected,"expected value after 'a' in attribute");
         XMLERRORCODE(xml_save_invalid_nodetype,"invalid nodetype encountered while saving");
         XMLERRORCODE(xml_filename_invalid,"invalid file name");
         XMLERRORCODE(xml_file_access,"could not access file");

         XMLERRORCODE(xml_invalid_operation, "attempted to execute command that would cause invalid structure");
         XMLERRORCODE(xml_invalid_argument,  "attempted to use an invalid argument");

         XMLERRORCODE(xml_dummy,"dummy error code (this error should never been seen)");
      }
#undef XMLERRORCODE
      return std::string(err);
   }

   std::string Error::getString() const
   {
      return getStrError() + std::string(": ") + mLocalDesc;
   }

   std::string Error::getInfo() const
   {
      return mLocation;
   }

   //LLocation methods

   Location::Location()
   {
      reset();
   }

   int Location::getLine() const
   {
      return mLine;
   }

   int Location::getPos() const
   {
      return mPos;
   }

   void Location::step(int chars)
   {
      mPos += chars;
   }

   void Location::newline()
   {
      ++mLine;
      mPos = 1;
   }

   void Location::reset()
   {
      mLine = mPos = 0;
   }

   // Context methods

   Context::Context()
      : mEventHandler(new EventHandler)
   {
      mInit = false;
      mNextHandle = 0;
      mHandleEvents = false;
   }

   Context::~Context()
   {
   }

   std::string Context::getEntity(const std::string& entName)
   {
      if (!mInit)
      {
         initContext();
      }

      EntityMap::const_iterator iter = mEntities.find(entName);
      return (iter == mEntities.end() ? entName : iter->second);
   }

   std::string Context::getTagname(TagNameHandle handle)
   {
      if (!mInit)
      {
         initContext();
      }
      TagNameMap::const_iterator iter = mTagNames.find(handle);

      std::string empty("");
      return (iter == mTagNames.end() ? empty : iter->second);
   }

   TagNameHandle Context::insertTagname(const std::string& tagName)
   {
      if (!mInit)
      {
         initContext();
      }

      // bugfix: first search, if the tagname is already in the list
      // since usually there are not much different tagnames, the search
      // shouldn't slow down parsing too much
      TagNameMap::const_iterator iter,stop;
      iter = mTagNames.begin();
      stop = mTagNames.end();

      for(; iter!=stop; ++iter)
      {
         if (iter->second == tagName)
         {
            return iter->first;
         }
      }

      // insert new tagname
      TagNameMap::value_type keyvaluepair(mNextHandle, tagName);
      mTagNames.insert(keyvaluepair);

      return mNextHandle++;
   }

   Location& Context::getLocation()
   {
      return mLocation;
   }

   void Context::initContext()
   {
      mInit = true;
   }

   void Context::setEventHandler(EventHandlerPtr ehptr)
   {
      mEventHandler = ehptr;
      mHandleEvents = true;
   }

   EventHandler& Context::getEventHandler()
   {
      return *mEventHandler.get();
   }

   bool Context::handleEvents() const
   {
      return mHandleEvents;
   }


   // Attributes methods

   Attribute::Attribute()
      : mData("")
   {}

   Attribute::Attribute(const Attribute& attr)
      : mData(attr.mData)
   {}

   Attribute::Attribute(const std::string& val)
      : mData(val)
   {}

   const std::string& Attribute::getString() const
   {
      return mData;
   }

   Attribute::operator std::string() const
   {
      return getString();
   }

   // Attributes methods

   Attributes::Attributes()
   {}

   std::string Attributes::get(const std::string& key) const
   {
      attr_map_t::const_iterator iter;

      // try to find the key in the map
      iter = mMap.find(key);
      std::string empty("");
      return ((iter == mMap.end()) ? empty : iter->second);
   }

   void Attributes::set(const std::string& key, const std::string& value)
   {
      attr_map_t::iterator iter;

      // try to find the key in the map
      iter = mMap.find(key);
      if (iter != mMap.end())
      {
         (*iter).second = value;
      }
      else
      {
         // insert, because the key-value pair was not found
         attr_map_t::value_type pa(key,value);
         mMap.insert(pa);
      }
   }

   bool Attributes::has(const std::string& key) const
   {
      attr_map_t::const_iterator iter;

      // try to find the key in the map
      iter = mMap.find(key);
      return (iter != mMap.end());
   }


   // Node methods

   Node::Node()
      : mNodeType(xml_nt_node), mParent(0)
   {}

   Node::Node(ContextPtr ctx)
      : mContext(ctx), mNodeType(xml_nt_node), mParent(0)
   {}

   Node::Node(std::string nodeName, ContextPtr ctx)
      : mContext(ctx), mNodeType(xml_nt_node), mParent(0)
   { setName(nodeName); }

   Node::Node(const Node& node)
      : mNodeNameHandle(node.mNodeNameHandle)
#ifdef CPPDOM_DEBUG
      , mNodeName_debug(node.mNodeName_debug)
#endif
      , mContext(node.mContext)
      , mNodeType(node.mNodeType)
      , mAttributes(node.mAttributes)
      , mCdata(node.mCdata)
      , mNodeList(node.mNodeList)
      , mParent(node.mParent)
   {}

   Node::~Node()
   {
      for (NodeList::iterator i = mNodeList.begin(); i != mNodeList.end(); ++i)
      {
         (*i)->mParent = NULL;
      }
   }

   Node& Node::operator=(const Node& node)
   {
      mNodeNameHandle = node.mNodeNameHandle;
#ifdef CPPDOM_DEBUG
      mNodeName_debug = node.mNodeName_debug;
#endif
      mContext = node.mContext;
      mNodeType = node.mNodeType;
      mAttributes = node.mAttributes;
      mCdata = node.mCdata;
      mNodeList = node.mNodeList;
      mParent = node.mParent;
      return *this;
   }

   /** Returns true if the nodes are equal
   * @param ignoreAttribs - Attributes to ignore in the comparison
   * @param ignoreElements - Elements to ignore in the comparison
   */
   bool Node::isEqual(NodePtr otherNode, const std::vector<std::string>& ignoreAttribs,
                      const std::vector<std::string>& ignoreElements,
                      bool dbgit, const unsigned debugIndent)
   {
      std::string indent(debugIndent, char(' '));     // Construct buffer of size debugIndent
      const std::string my_name(getName());           // Get element names
      const std::string other_name(otherNode->getName());

      if(dbgit) std::cout << indent << "isEqual: me:" << my_name << "  other:" << other_name << std::endl;

      // If we are supposed to ignore this element type, then return immediately
      if( (std::find(ignoreElements.begin(), ignoreElements.end(), my_name) != ignoreElements.end()) &&
          (std::find(ignoreElements.begin(), ignoreElements.end(), other_name) != ignoreElements.end()))
      { return true; }

      // Check current node's element type (ie. name)
      if(other_name != my_name)
      {
         if(dbgit) std::cout << indent << "Different elt types: not equal.\n";
         return false;
      }

      // Check attributes
      Attributes& other_attribs = otherNode->getAttrMap();

      if(other_attribs.size() !=  mAttributes.size())
      {
         if(dbgit) std::cout << indent << "Different number of attributes: not equal.\n";
         return false;
      }

      // Check attribute values
      for(Attributes::iterator cur_attrib = other_attribs.begin();
          cur_attrib != other_attribs.end(); cur_attrib++)
      {
         std::string attrib_name = (*cur_attrib).first;
         std::string attrib_value = (*cur_attrib).second;
         if(dbgit) std::cout << indent << "Comparing attribute: " << attrib_name << std::endl;

         // If not in ignore list
         if(std::find(ignoreAttribs.begin(), ignoreAttribs.end(), attrib_name)
                        == ignoreAttribs.end())
         {
            // if (not have attribute) OR attrib != needed
            if( !mAttributes.has(attrib_name) ||
                (mAttributes.get(attrib_name) != attrib_value))
            {
               if(dbgit) std::cout << indent << "Attributes [" << attrib_name << "] are different: not equal.\n";
               return false;
            }
         }
         else
         {
            if(dbgit) std::cout << indent << "Ignoring attribute: " << attrib_name << std::endl;
         }
      }

      if(dbgit) std::cout << indent << "Checking cdata.\n";
      // Check cdata
      if(otherNode->getFullCdata() != getFullCdata())
      {
         if(dbgit) std::cout << indent << "Cdata different: not equal\n";
         return false;
      }

      // -- Check children -- //
      NodeList other_children = otherNode->getChildren();
      /*
      unsigned num_children = mNodeList.size();
      unsigned num_other_children = other_children.size();
      std::cout << "children: me:" << num_children << "  other:" << num_other_children << std::endl;
      */
      if(mNodeList.size() != other_children.size())
      {
         if(dbgit) std::cout << indent << "Different number of children: not equal\n";
         return false;
      }

      if(dbgit) std::cout << indent << "Comparing children:\n";
      // Recurse into each element
      NodeList::iterator my_child, other_child;
      for(my_child = mNodeList.begin(), other_child = other_children.begin();
          my_child != mNodeList.end(), other_child != other_children.end();
          my_child++, other_child++)
      {
         if(false == (*my_child)->isEqual((*other_child), ignoreAttribs, ignoreElements, dbgit, debugIndent+3))
         {
            if(dbgit) std::cout << indent << "Childrent different: not equal\n";
            return false;
         }
      }

      return true;
   }


   NodeType Node::getType() const
   {
      return mNodeType;
   }

   std::string Node::getName()
   {
      return mContext->getTagname(mNodeNameHandle);
   }

   Attributes& Node::getAttrMap()
   {
      return mAttributes;
   }

   const Attributes& Node::getAttrMap() const
   {
      return mAttributes;
   }

   Attribute Node::getAttribute(const std::string& name) const
   {
      return mAttributes.get(name);
   }

   bool Node::hasAttribute(const std::string& name) const
   {
      return mAttributes.has(name);
   }

   /** Return cdata for node.
   * If node is cdata type, then return local data
   * else, return contents of first cdata child
   */
   std::string Node::getCdata()
   {
      std::string ret_val;

      if(getType() == cppdom::xml_nt_cdata)
      {
         ret_val = mCdata;
      }
      else
      {
         cppdom::NodeList nl = getChildrenPred( cppdom::IsNodeTypePredicate( cppdom::xml_nt_cdata ));
         if(!nl.empty())
         {
            ret_val = (*(nl.begin()))->getCdata();
         }
      }
      return ret_val;
   }

   /** Return a copy of all the cdata
   */
   std::string Node::getFullCdata()
   {
      std::string ret_val;

      if(getType() == cppdom::xml_nt_cdata)
      {
         ret_val = mCdata;
      }
      else
      {
         cppdom::NodeList nl = getChildrenPred( cppdom::IsNodeTypePredicate( cppdom::xml_nt_cdata ));
         for(cppdom::NodeList::iterator n=nl.begin(); n!=nl.end(); ++n)
         {
            ret_val += (*n)->getCdata();
         }
      }
      return ret_val;
   }


   void Node::setType(NodeType type)
   {
      mNodeType = type;
   }

   void Node::setName(const std::string& name)
   {
      mNodeNameHandle = mContext->insertTagname(name);
#ifdef CPPDOM_DEBUG
      mNodeName_debug = name;
#endif
   }

   /** Set the element cdata.
   * If not cdata type, then try to find first cdata.
   * If we don't have a cdata child, then add one and set it
   */
   void Node::setCdata(const std::string& cdata)
   {
      if(getType() == cppdom::xml_nt_cdata)
      {
         mCdata = cdata;
      }
      else
      {
         cppdom::NodeList nl = getChildrenPred( cppdom::IsNodeTypePredicate( cppdom::xml_nt_cdata ));
         if(!nl.empty())
         {
            (*(nl.begin()))->setCdata(cdata);
         }
         else  // Create a child
         {
            cppdom::NodePtr new_cdata(new cppdom::Node("cdata", getContext()));
            new_cdata->setType(cppdom::xml_nt_cdata);
            new_cdata->setCdata(cdata);
            addChild(new_cdata);
         }
      }
   }

   void Node::setAttribute(const std::string& attr, const Attribute& value)
   {
      // Check for valid input
      if(std::string::npos != attr.find(" "))  // If found space
      {
         throw CPPDOM_ERROR(xml_invalid_argument, "Attempted to use attribute name with a space");
      }

      mAttributes.set(attr, value.getString());
   }

   void Node::addChild(NodePtr& node)
   {
      // Check for invalid call
      if(node.get() == NULL)
      {
         throw CPPDOM_ERROR(xml_invalid_argument, "Attempted to add NULL node as child.");
      }
      if(xml_nt_document == getType())
      {
         if(!mNodeList.empty())
         {  throw CPPDOM_ERROR(xml_invalid_operation, "Attempted to add second child to a document node"); }
      }

      node->mParent = this;      // Tell the child who their daddy is
      mNodeList.push_back(node);
   }

   bool Node::removeChild(NodePtr& node)
   {
      cppdom::ignore_unused_variable_warning(node);
      std::cout << "cppdom::Node::removeChild:  not implemented\n";
      return false;
   }

   bool Node::removeChild(std::string& childName)
   {
      cppdom::ignore_unused_variable_warning(childName);
      std::cout << "cppdom::Node::removeChild:  not implemented\n";
      return false;
   }

   bool Node::removeChildren(std::string& childName)
   {
      cppdom::ignore_unused_variable_warning(childName);
      std::cout << "cppdom::Node::removeChildren:  not implemented\n";
      return false;
   }

   NodeList& Node::getChildren()
   {
      return mNodeList;
   }

   /** \note currently no path-like childname can be passed, like in e.g. msxml */
   NodePtr Node::getChild(const std::string& name)
   {
      // possible speedup: first search if a handle to the childname is existing
      NodeList::const_iterator iter;

      // search for first occurance of node
      for(iter = mNodeList.begin(); iter != mNodeList.end(); ++iter)
      {
         NodePtr np = (*iter);
         if (np->getName() == name)
         {
            return np;
         }
      }

      // no valid child found
      return NodePtr();
   }

   NodeList Node::getChildren(const std::string& name)
   {
      NodeList result(0);
      NodeList::const_iterator iter;

      // search for all occurances of nodename and insert them into the new list
      for(iter = mNodeList.begin(); iter != mNodeList.end(); ++iter)
      {
         if ((*iter)->getName() == name)
         {
            result.push_back(*iter);
         }
      }

      return result;
   }

   NodeList Node::getChildren(const char* name)
   {
      return getChildren(std::string(name));
   }

   Node* Node::getParent() const
   {
      return mParent;
   }

   /** \exception throws cppdom::Error when a streaming or parsing error occur */
   void Node::load(std::istream& in, ContextPtr& context)
   {
      Parser parser(in, context->getLocation());
      parser.parseNode(*this, context);
   }

   /** \exception throws cppdom::Error when a streaming or parsing error occur */
   void Node::save(std::ostream& out, int indent, bool doIndent, bool doNewline)
   {
      // output indendation spaces
      if(doIndent)
      {
         for(int i=0; i<indent; ++i)
         {  out << ' '; }
      }

      // output cdata
      if (mNodeType == xml_nt_cdata)
      {
         out << mCdata.c_str();
         if(doNewline)
            out << std::endl;
         return;
      }

      // output tag name
      out << '<' << mContext->getTagname(mNodeNameHandle);

      // output all attributes
      Attributes::const_iterator iter, stop;
      iter = mAttributes.begin();
      stop = mAttributes.end();

      for(; iter!=stop; ++iter)
      {
         Attributes::value_type attr = *iter;
         out << ' ' << attr.first << '=' << '\"' << attr.second << '\"';
      }

      // depending on the nodetype, do output
      switch(mNodeType)
      {
      case xml_nt_node:
         {
            out << '>';
            if(doNewline)
               out << std::endl;

            // output all subnodes
            NodeList::const_iterator iter,stop;
            iter = mNodeList.begin();
            stop = mNodeList.end();

            for(; iter!=stop; ++iter)
            {
               (*iter)->save(out, indent+1, doIndent, doNewline);
            }

            // output indendation spaces
            if(doIndent)
            {
               for(int i=0;i<indent;i++)
               { out << ' '; }
            }

            // output closing tag
            out << '<' << '/' << mContext->getTagname(mNodeNameHandle) << '>';
            if(doNewline)
               out << std::endl;
         }
         break;
      case xml_nt_leaf:
         // a leaf has no subnodes
         out << '/' << '>';
         if(doNewline)
            out << std::endl;
         break;
      default:
         // unknown nodetype
         throw CPPDOM_ERROR(xml_save_invalid_nodetype, "Tried to save an unknown node type");
      }
   }

   ContextPtr Node::getContext()
   {
      return mContext;
   }


   // Document methods

   Document::Document()
   {
      mNodeType = xml_nt_document;
   }

   Document::Document(ContextPtr context)
      : Node(context)
   {
      mNodeType = xml_nt_document;
   }

   Document::Document(std::string docName, ContextPtr context)
      : Node(docName, context)
   {
      mNodeType = xml_nt_document;
   }


   NodeList& Document::getPiList()
   {
      return mProcInstructions;
   }

   NodeList& Document::getDtdList()
   {
      return mDtdRules;
   }

   /** \exception throws cppdom::Error when a streaming or parsing error occur */
   void Document::load(std::istream& in, ContextPtr& context)
   {
      Parser parser(in, context->getLocation());
      parser.parseDocument(*this, context);
   }

   /** \todo implement: print <!doctype> tag;
   * \exception throws cppdom::Error when a streaming or parsing error occur
   */
   void Document::save(std::ostream& out, bool doIndent, bool doNewline)
   {
      // output all processing instructions
      NodeList::const_iterator iter, stop;
      iter = mProcInstructions.begin();
      stop = mProcInstructions.end();

      for(; iter!=stop; ++iter)
      {
         NodePtr np = *iter;

         // output pi tag
         out << "<?" << np->getName();

         // output all attributes
         Attributes::const_iterator aiter, astop;
         aiter = mAttributes.begin();
         astop = mAttributes.end();

         for(; aiter!=astop; ++aiter)
         {
            Attributes::value_type attr = *aiter;
            out << ' ' << attr.first << '=' << '\"' << attr.second << '\"';
         }
         // output closing brace
         out << "?>" << std::endl;
      }

      // output <!doctype> tag

      // left to do ...


      // call save() method of the first (and hopefully only) node in Document
      (*mNodeList.begin())->save(out, 0, doIndent, doNewline);
   }

   void Document::loadFile(const std::string& filename) throw(Error)
   {
      std::ifstream in;
      in.open(filename.c_str(), std::ios::in);

      if (! in.good())
      {
         throw CPPDOM_ERROR(xml_filename_invalid, "Filename passed to loadFile was invalid");
      }

      load(in, mContext);
      in.close();
   }

   void Document::saveFile(std::string filename)
   {
      std::fstream out;
      out.open(filename.c_str(), std::fstream::out);
      this->save(out);
      out.close();
   }
}
