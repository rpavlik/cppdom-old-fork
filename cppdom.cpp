// cppdom was branched from the original LGPL'd xmlpp version 0.6
// this new branched xmlpp is under the same LGPL (of course) and
// is being maintained by:
//    kevin meinert <subatomic@users.sf.net>
//    allen bierbaum <allenb@users.sf.net>
//    ben scott <nonchocoboy@users.sf.net>
/////////////////////////////////////////////////////////////////////
/*
   xmlpp - an xml parser and validator written in C++
   copyright (c) 2000-2001 Michael Fink

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA  02111-1307  USA.

*/
/*! \file cppdom.cpp

  contains the methods of the xmlpp classes

*/

// needed includes
#include <cppdom/cppdom.h>
#include <cppdom/xmlparser.h>

// namespace declaration
namespace cppdom
{

   // macro for keeping the errorcode switch short and easy
#define XMLERRORCODE(x,y)  case x: err = y; break;

   // XMLError methods
   void XMLError::getStrError(XMLString& error) const
   {
      const char *err;
      switch(errorcode)
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
   }


   // XMLContext methods

   XMLContext::XMLContext()
      : eventhandler(new XMLEventHandler)
   {
      init = false;
      nexthandle = 0;
      handleevents = false;
   }

   XMLContext::~XMLContext()
   {
   }

   XMLString XMLContext::getEntity(const XMLString& entname)
   {
      if (!init)
      {
         initContext();
      }

      XMLEntityMap::const_iterator iter = entities.find(entname);
      return (iter == entities.end() ? entname : iter->second);
   }

   XMLString XMLContext::getTagname(XMLTagNameHandle handle)
   {
      if (!init)
      {
         initContext();
      }
      XMLTagNameMap::const_iterator iter = tagnames.find(handle);

      XMLString empty("");
      return (iter == tagnames.end() ? empty : iter->second);
   }

   XMLTagNameHandle XMLContext::insertTagname(const XMLString& tagname)
   {
      if (!init)
      {
         initContext();
      }

      // bugfix: first search, if the tagname is already in the list
      // since usually there are not much different tagnames, the search
      // shouldn't slow down parsing too much
      XMLTagNameMap::const_iterator iter,stop;
      iter = tagnames.begin();
      stop = tagnames.end();

      for(; iter!=stop; ++iter)
      {
         if ((*iter).second == tagname)
         {
            return (*iter).first;
         }
      }

      // insert new tagname
      XMLTagNameMap::value_type keyvaluepair(nexthandle,tagname);
      tagnames.insert(keyvaluepair);

      return nexthandle++;
   }


   // XMLAttributes methods
   bool XMLAttributes::has(const XMLString& key) const
   {
      XMLAttributes::const_iterator iter;

      // try to find the key in the map
      iter = find(key);
      return (iter != end());
   }

   XMLString XMLAttributes::get(const XMLString& key) const
   {
      XMLAttributes::const_iterator iter;

      // try to find the key in the map
      iter = find(key);
      XMLString empty("");
      return ((iter == end()) ? empty : iter->second);
   }

   void XMLAttributes::set(const XMLString& key, const XMLString& value)
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


   // XMLNode methods

   XMLNode::XMLNode(const XMLNode& node)
   {
      nodenamehandle = node.nodenamehandle;
      contextptr = node.contextptr;
      nodetype = node.nodetype;
      attributes = node.attributes;
      mCdata = node.mCdata;
      mNodelist = node.mNodelist;
      mParent = node.mParent;
   };

   XMLNode& XMLNode::operator=(const XMLNode& node)
   {
      nodenamehandle = node.nodenamehandle;
      contextptr = node.contextptr;
      nodetype = node.nodetype;
      attributes = node.attributes;
      mCdata = node.mCdata;
      mNodelist = node.mNodelist;
      mParent = node.mParent;
      return *this;
   };

   XMLString XMLNode::getName()
   {
      return contextptr->getTagname(nodenamehandle);
   }

   void XMLNode::setName(const XMLString& nname)
   {
      nodenamehandle = contextptr->insertTagname(nname);
   }

   /** \note currently no path-like childname can be passed, like in e.g. msxml */
   XMLNodePtr XMLNode::getChild(const XMLString& name)
   {
      // possible speedup: first search if a handle to the childname is existing
      XMLNodeList::const_iterator iter;

      // search for first occurance of node
      for(iter = mNodelist.begin(); iter != mNodelist.end(); ++iter)
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

   /** \exception throws cppdom::XMLError when a streaming or parsing error occur */
   void XMLNode::load(std::istream& instream, XMLContextPtr& ctxptr )
   {
      XMLParser parser(instream, ctxptr->getLocation());
      parser.parseNode(*this, ctxptr);
   }

   /** \exception throws cppdom::XMLError when a streaming or parsing error occur */
   void XMLNode::save(std::ostream& outstream, int indent)
   {
      // output indendation spaces
      for(int i=0; i<indent; ++i)
      {
         outstream << ' ';
      }

      // output cdata
      if (nodetype == xml_nt_cdata)
      {
         outstream << mCdata.c_str() << std::endl;
         return;
      }

      // output tag name
      outstream << '<' << contextptr->getTagname(nodenamehandle).c_str();

      // output all attributes
      XMLAttributes::const_iterator iter, stop;
      iter = attributes.begin();
      stop = attributes.end();

      for(; iter!=stop; ++iter)
      {
         XMLAttributes::value_type attr = *iter;
         outstream << ' ' << attr.first.c_str() << '='
            << '\"' << attr.second.c_str() << '\"';
      }

      // depending on the nodetype, do output
      switch(nodetype)
      {
      case xml_nt_node:
         {
            outstream << '>' << std::endl;

            // output all subnodes
            XMLNodeList::const_iterator iter,stop;
            iter = mNodelist.begin();
            stop = mNodelist.end();

            for(;iter!=stop;iter++)
            {
               (*iter)->save(outstream, indent+1);
            }

            // output indendation spaces
            for(int i=0;i<indent;i++)
            {
               outstream << ' ';
            }

            // output closing tag
            outstream << '<' << '/'
               << contextptr->getTagname(nodenamehandle).c_str() << '>' << std::endl;
         }
         break;
      case xml_nt_leaf:
         // a leaf has no subnodes
         outstream << '/' << '>' << std::endl;
         break;
      default:
         // unknown nodetype
         throw XMLError(xml_save_invalid_nodetype);
      }
   }


   // XMLDocument methods

   /** \exception throws cppdom::XMLError when a streaming or parsing error occur */
   void XMLDocument::load(std::istream& instream, XMLContextPtr& ctxptr)
   {
      XMLParser parser(instream, ctxptr->getLocation());
      parser.parseDocument(*this, ctxptr);
   }

   /** \todo implement: print <!doctype> tag;
   * \exception throws cppdom::XMLError when a streaming or parsing error occur
   */
   void XMLDocument::save(std::ostream& outstream)
   {
      // output all processing instructions
      XMLNodeList::const_iterator iter,stop;
      iter = procinstructions.begin();
      stop = procinstructions.end();

      for(; iter!=stop; ++iter)
      {
         XMLNodePtr np = *iter;

         // output pi tag
         outstream << "<?" << np->getName().c_str();

         // output all attributes
         XMLAttributes::const_iterator aiter, astop;
         aiter = attributes.begin();
         astop = attributes.end();

         for(; aiter!=astop; ++aiter)
         {
            XMLAttributes::value_type attr = *aiter;
            outstream << ' ' << attr.first.c_str() << '='
               << '\"' << attr.second.c_str() << '\"';
         }
         // output closing brace
         outstream << "?>" << std::endl;
      }

      // output <!doctype> tag

      // left to do ...


      // call save() method of the first (and hopefully only) node in XMLDocument
      (*mNodelist.begin())->save(outstream, 0);
   }
}
