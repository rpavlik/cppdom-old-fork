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

// needed includes
#include "cppdom.h"
#include "xmlparser.h"

// namespace declaration
namespace cppdom
{
   // XMLError methods

   XMLError::XMLError(XMLErrorCode code)
      : mErrorCode(code)
   {}

   XMLErrorCode XMLError::getError() const
   {
      return mErrorCode;
   }

   void XMLError::getStrError(std::string& error) const
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
         XMLERRORCODE(xml_dummy,"dummy error code (this error should never been seen)");
      }
      error.assign(err);
#undef XMLERRORCODE
   }

   std::string XMLError::getString() const
   {
      std::string err;
      getStrError(err);
      return err;
   }

   std::string XMLError::getInfo() const
   {
      return "unknown error";
   }

   // XMLLocation methods

   XMLLocation::XMLLocation()
   {
      reset();
   }

   int XMLLocation::getLine() const
   {
      return mLine;
   }

   int XMLLocation::getPos() const
   {
      return mPos;
   }

   void XMLLocation::step(int chars)
   {
      mPos += chars;
   }

   void XMLLocation::newline()
   {
      ++mLine;
      mPos = 1;
   }

   void XMLLocation::reset()
   {
      mLine = mPos = 0;
   }

   // XMLContext methods

   XMLContext::XMLContext()
      : mEventHandler(new XMLEventHandler)
   {
      mInit = false;
      mNextHandle = 0;
      mHandleEvents = false;
   }

   XMLContext::~XMLContext()
   {
   }

   std::string XMLContext::getEntity(const std::string& entName)
   {
      if (!mInit)
      {
         initContext();
      }

      XMLEntityMap::const_iterator iter = mEntities.find(entName);
      return (iter == mEntities.end() ? entName : iter->second);
   }

   std::string XMLContext::getTagname(XMLTagNameHandle handle)
   {
      if (!mInit)
      {
         initContext();
      }
      XMLTagNameMap::const_iterator iter = mTagNames.find(handle);

      std::string empty("");
      return (iter == mTagNames.end() ? empty : iter->second);
   }

   XMLTagNameHandle XMLContext::insertTagname(const std::string& tagName)
   {
      if (!mInit)
      {
         initContext();
      }

      // bugfix: first search, if the tagname is already in the list
      // since usually there are not much different tagnames, the search
      // shouldn't slow down parsing too much
      XMLTagNameMap::const_iterator iter,stop;
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
      XMLTagNameMap::value_type keyvaluepair(mNextHandle, tagName);
      mTagNames.insert(keyvaluepair);

      return mNextHandle++;
   }

   XMLLocation& XMLContext::getLocation()
   {
      return mLocation;
   }

   void XMLContext::initContext()
   {
      mInit = true;
   }

   void XMLContext::setEventHandler(XMLEventHandlerPtr ehptr)
   {
      mEventHandler = ehptr;
      mHandleEvents = true;
   }

   XMLEventHandler& XMLContext::getEventHandler()
   {
      return *mEventHandler.get();
   }

   bool XMLContext::handleEvents() const
   {
      return mHandleEvents;
   }
   

   // XMLAttributes methods

   XMLAttribute::XMLAttribute()
      : mData("")
   {}

   XMLAttribute::XMLAttribute(const XMLAttribute& attr)
      : mData(attr.mData)
   {}

   XMLAttribute::XMLAttribute(const std::string& val)
      : mData(val)
   {}

   const std::string& XMLAttribute::getString() const
   {
      return mData;
   }

   XMLAttribute::operator std::string() const
   {
      return getString();
   }

   // XMLAttributes methods

   XMLAttributes::XMLAttributes()
   {}

   std::string XMLAttributes::get(const std::string& key) const
   {
      XMLAttributes::const_iterator iter;

      // try to find the key in the map
      iter = find(key);
      std::string empty("");
      return ((iter == end()) ? empty : iter->second);
   }

   void XMLAttributes::set(const std::string& key, const std::string& value)
   {
      XMLAttributes::iterator iter;

      // try to find the key in the map
      iter = find(key);
      if (iter != end())
      {
         (*iter).second = value;
      }
      else
      {
         // insert, because the key-value pair was not found
         XMLAttributes::value_type pa(key,value);
         insert(pa);
      }
   }

   bool XMLAttributes::has(const std::string& key) const
   {
      XMLAttributes::const_iterator iter;

      // try to find the key in the map
      iter = find(key);
      return (iter != end());
   }


   // XMLNode methods

   XMLNode::XMLNode()
      : mNodeType(xml_nt_node), mParent(0)
   {}

   XMLNode::XMLNode(XMLContextPtr ctx)
      : mContext(ctx), mNodeType(xml_nt_node), mParent(0)
   {}

   XMLNode::XMLNode(const XMLNode& node)
      : mNodeNameHandle(node.mNodeNameHandle)
      , mContext(node.mContext)
      , mNodeType(node.mNodeType)
      , mAttributes(node.mAttributes)
      , mCdata(node.mCdata)
      , mNodeList(node.mNodeList)
      , mParent(node.mParent)
   {}

   XMLNode::~XMLNode()
   {
      for (XMLNodeList::iterator i = mNodeList.begin(); i != mNodeList.end(); ++i)
      {
         (*i)->mParent = NULL;
      }
   }

   XMLNode& XMLNode::operator=(const XMLNode& node)
   {
      mNodeNameHandle = node.mNodeNameHandle;
      mContext = node.mContext;
      mNodeType = node.mNodeType;
      mAttributes = node.mAttributes;
      mCdata = node.mCdata;
      mNodeList = node.mNodeList;
      mParent = node.mParent;
      return *this;
   }

   XMLNodeType XMLNode::getType() const
   {
      return mNodeType;
   }

   std::string XMLNode::getName()
   {
      return mContext->getTagname(mNodeNameHandle);
   }

   XMLAttributes& XMLNode::getAttrMap()
   {
      return mAttributes;
   }

   XMLAttribute XMLNode::getAttribute(const std::string& name) const
   {
      return mAttributes.get(name);
   }

   bool XMLNode::hasAttribute(const std::string& name) const
   {
      return mAttributes.has(name);
   }

   const std::string& XMLNode::getCdata()
   {
      return mCdata;
   }

   void XMLNode::setType(XMLNodeType type)
   {
      mNodeType = type;
   }

   void XMLNode::setName(const std::string& name)
   {
      mNodeNameHandle = mContext->insertTagname(name);
   }

   void XMLNode::setCdata(const std::string& cdata)
   {
      mCdata = cdata;
   }

   void XMLNode::setAttribute(const std::string& attr, const XMLAttribute& value)
   {
      mAttributes.set(attr, value.getString());
   }

   void XMLNode::addChild(XMLNodePtr& node)
   {
      node->mParent = this;      // Tell the child who their daddy is
      mNodeList.push_back(node);
   }

   bool XMLNode::removeChild(XMLNodePtr& node)
   {
      std::cout << "cppdom::XMLNode::removeChild:  not implemented\n";
      return false;
   }

   bool XMLNode::removeChild(std::string& childName)
   {
      std::cout << "cppdom::XMLNode::removeChild:  not implemented\n";
      return false;
   }

   bool XMLNode::removeChildren(std::string& childName)
   {
      std::cout << "cppdom::XMLNode::removeChildren:  not implemented\n";
      return false;
   }

   XMLNodeList& XMLNode::getChildren()
   {
      return mNodeList;
   }

   /** \note currently no path-like childname can be passed, like in e.g. msxml */
   XMLNodePtr XMLNode::getChild(const std::string& name)
   {
      // possible speedup: first search if a handle to the childname is existing
      XMLNodeList::const_iterator iter;

      // search for first occurance of node
      for(iter = mNodeList.begin(); iter != mNodeList.end(); ++iter)
      {
         XMLNodePtr np = (*iter);
         if (np->getName() == name)
         {
            return np;
         }
      }

      // no valid child found
      return XMLNodePtr();
   }

   XMLNodeList XMLNode::getChildren(const std::string& name)
   {
      XMLNodeList result(0);
      XMLNodeList::const_iterator iter;

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

   XMLNodeList XMLNode::getChildren(const char* name)
   {
      return getChildren(std::string(name));
   }

   XMLNode* XMLNode::getParent() const
   {
      return mParent;
   }

   /** \exception throws cppdom::XMLError when a streaming or parsing error occur */
   void XMLNode::load(std::istream& in, XMLContextPtr& context)
   {
      XMLParser parser(in, context->getLocation());
      parser.parseNode(*this, context);
   }

   /** \exception throws cppdom::XMLError when a streaming or parsing error occur */
   void XMLNode::save(std::ostream& out, int indent)
   {
      // output indendation spaces
      for(int i=0; i<indent; ++i)
      {
         out << ' ';
      }

      // output cdata
      if (mNodeType == xml_nt_cdata)
      {
         out << mCdata.c_str() << std::endl;
         return;
      }

      // output tag name
      out << '<' << mContext->getTagname(mNodeNameHandle);

      // output all attributes
      XMLAttributes::const_iterator iter, stop;
      iter = mAttributes.begin();
      stop = mAttributes.end();

      for(; iter!=stop; ++iter)
      {
         XMLAttributes::value_type attr = *iter;
         out << ' ' << attr.first << '=' << '\"' << attr.second << '\"';
      }

      // depending on the nodetype, do output
      switch(mNodeType)
      {
      case xml_nt_node:
         {
            out << '>' << std::endl;

            // output all subnodes
            XMLNodeList::const_iterator iter,stop;
            iter = mNodeList.begin();
            stop = mNodeList.end();

            for(; iter!=stop; ++iter)
            {
               (*iter)->save(out, indent+1);
            }

            // output indendation spaces
            for(int i=0;i<indent;i++)
            {
               out << ' ';
            }

            // output closing tag
            out << '<' << '/'
               << mContext->getTagname(mNodeNameHandle) << '>' << std::endl;
         }
         break;
      case xml_nt_leaf:
         // a leaf has no subnodes
         out << '/' << '>' << std::endl;
         break;
      default:
         // unknown nodetype
         throw XMLError(xml_save_invalid_nodetype);
      }
   }

   XMLContextPtr XMLNode::getContext()
   {
      return mContext;
   }


   // XMLDocument methods

   XMLDocument::XMLDocument()
   {
      mNodeType = xml_nt_document;
   }

   XMLDocument::XMLDocument(XMLContextPtr context)
      : XMLNode(context)
   {
      mNodeType = xml_nt_document;
   }

   XMLNodeList& XMLDocument::getPiList()
   {
      return mProcInstructions;
   }

   XMLNodeList& XMLDocument::getDtdList()
   {
      return mDtdRules;
   }

   /** \exception throws cppdom::XMLError when a streaming or parsing error occur */
   void XMLDocument::load(std::istream& in, XMLContextPtr& context)
   {
      XMLParser parser(in, context->getLocation());
      parser.parseDocument(*this, context);
   }

   /** \todo implement: print <!doctype> tag;
   * \exception throws cppdom::XMLError when a streaming or parsing error occur
   */
   void XMLDocument::save(std::ostream& out)
   {
      // output all processing instructions
      XMLNodeList::const_iterator iter, stop;
      iter = mProcInstructions.begin();
      stop = mProcInstructions.end();

      for(; iter!=stop; ++iter)
      {
         XMLNodePtr np = *iter;

         // output pi tag
         out << "<?" << np->getName();

         // output all attributes
         XMLAttributes::const_iterator aiter, astop;
         aiter = mAttributes.begin();
         astop = mAttributes.end();

         for(; aiter!=astop; ++aiter)
         {
            XMLAttributes::value_type attr = *aiter;
            out << ' ' << attr.first << '=' << '\"' << attr.second << '\"';
         }
         // output closing brace
         out << "?>" << std::endl;
      }

      // output <!doctype> tag

      // left to do ...


      // call save() method of the first (and hopefully only) node in XMLDocument
      (*mNodeList.begin())->save(out, 0);
   }

   void XMLDocument::loadFile(const std::string& filename) throw(XMLError)
   {
      std::ifstream in;
      in.open(filename.c_str(), std::ios::in);

      if (! in.good())
      {
         throw XMLError(xml_filename_invalid);
      }

      load(in, mContext);
      in.close();
   }

   void XMLDocument::saveFile(const std::string& filename)
   {
      std::ofstream out;
      out.open(filename.c_str(), std::ios::in | std::ios::out);
      this->save(out);
      out.close();
   }
}
