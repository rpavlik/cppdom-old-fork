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
/** \file xmlparser.cpp

  member functions for the mTokenizer and parser classes

*/

// needed includes
#include "xmlparser.h"

// namespace declaration
namespace cppdom
{
   // Parser methods
   Parser::Parser(std::istream& in, Location& loc)
      : mInput(in), mTokenizer(in, loc)
   {}

   bool Parser::parseDocument(Document& doc, ContextPtr& context)
   {
      // set root nodename
      doc.mContext = context;
      std::string rootstr("root");
      doc.mNodeNameHandle = context->insertTagname(rootstr);
#ifdef CPPDOM_DEBUG
      doc.mNodeName_debug = rootstr;
#endif

      bool handle = context->handleEvents();

      // start parsing
      if (handle)
      {
         context->getEventHandler().startDocument();
      }

      parseHeader(doc, context);

      // parse the only one subnode
      NodePtr new_subnode(new Node(context));

      bool ret = parseNode(*new_subnode, context);

      // if successful, put node into nodelist
      if (ret)
      {
         doc.addChild(new_subnode);
      }

      if (handle)
      {
         context->getEventHandler().endDocument();
      }

      return ret;
   }

   // parses the header, ie processing instructions and doctype tag
   /// \todo parse <!doctype> tag
   bool Parser::parseHeader(Document& doc, ContextPtr& context)
   {
      while(true)
      {
         ++mTokenizer;
         Token token1 = *mTokenizer;
         if (token1 != '<')
         {
            throw CPPDOM_ERROR(xml_opentag_expected, "");
         }

         // token after opening < is a literal?
         mTokenizer++;
         Token token2 = *mTokenizer;
         if (!token2.isLiteral())
         {
            // generic string encountered: assume no pi and doctype tags
            mTokenizer.putBack();
            mTokenizer.putBack(token1);
            return false;
         }

         // now check for the literal
         switch(token2.getLiteral())
         {
            // comment or doctype tag
         case '!':
            {
               ++mTokenizer;
               Token token3 = *mTokenizer;

               if (!token3.isLiteral())
               {
                  // now a doctype tag or a comment may follow
                  if (token3.getGeneric().at(0) == '-' &&
                      token3.getGeneric().at(1) == '-')
                  {
                      parseComment(context);
                  }
                  else
                  {
                     std::string doctypestr(token3.getGeneric());

                     std::transform(doctypestr.begin(), doctypestr.end(), doctypestr.begin(), toupper);

                     if (doctypestr == "DOCTYPE")
                     {
                        // \todo parse doctype tag

                        // read the complete tag till the closing >
                        while (*(mTokenizer++) != '>');
                     }
                     else
                     {
                        throw CPPDOM_ERROR(xml_unknown, "");
                     }
                  }
               }
               else
               {
                  throw CPPDOM_ERROR(xml_pi_doctype_expected, "");
               }

               break;
            }
         case '?':
            {
               ++mTokenizer;
               Token token3 = *mTokenizer;

               if (token3.isLiteral())
               {
                  throw CPPDOM_ERROR(xml_pi_doctype_expected, "");
               }

               // parse processing instruction
               Node pinode(context);

               std::string tagname(token3.getGeneric());
               pinode.mNodeNameHandle = context->insertTagname(tagname);
#ifdef CPPDOM_DEBUG
               pinode.mNodeName_debug = tagname;
#endif


               parseAttributes(pinode.getAttrMap());

               NodePtr nodeptr(new Node(pinode));
               doc.mProcInstructions.push_back(nodeptr);

               if (context->handleEvents())
               {
                  context->getEventHandler().processingInstruction(pinode);
               }

               ++mTokenizer;
               if (*mTokenizer != '?')
               {
                  throw CPPDOM_ERROR(xml_pi_doctype_expected, "");
               }

               ++mTokenizer;
               if (*mTokenizer != '>')
               {
                  throw CPPDOM_ERROR(xml_closetag_expected, "");
               }
               break;
            }
         default:
            // unknown literal encountered
            throw CPPDOM_ERROR(xml_pi_doctype_expected, "");

         } // end switch

      } // end while
   }

   // parses the contents of the current node
   bool Parser::parseNode(Node& node, ContextPtr& context)
   {
      node.mContext = context;
      bool handle = context->handleEvents();

      ++mTokenizer;
      Token token1 = *mTokenizer;

      if (token1.isEndOfStream())
      {
         return false;
      }

      Token token2;

      // loop when we encounter a comment
      bool again;
      do
      {
         again = false;

         // check if we have cdata
         if (!token1.isLiteral())
         {
            std::string cdataname("cdata");
            node.mNodeNameHandle = context->insertTagname(cdataname);
#ifdef CPPDOM_DEBUG
            node.mNodeName_debug = cdataname;
#endif

            // parse cdata section(s) and return
            node.mNodeType = xml_nt_cdata;
            node.mCdata.empty();

            while(!token1.isLiteral())
            {
               node.mCdata += token1.getGeneric();
               ++mTokenizer;
               token1 = *mTokenizer;
            }
            mTokenizer.putBack();

            if (handle)
            {
               context->getEventHandler().gotCdata( node.mCdata );
            }

            return true;
         }

         // no cdata, try to continue parsing node content
         // Must be a start of a node (ie. < literal)
         if (token1 != '<')
         {
            throw CPPDOM_ERROR(xml_opentag_cdata_expected, "");
         }

         // get node name
         ++mTokenizer;
         token2 = *mTokenizer;
         if (token2.isLiteral())
         {
            // check the following literal
            switch(token2.getLiteral())
            {
               // closing '</...>' follows
            case '/':
               // return, we have a closing node with no more content
               mTokenizer.putBack();
               mTokenizer.putBack(token1);
               return false;

               // comment follows
            case '!':
               this->parseComment(context);

               // get next token
               ++mTokenizer;
               token1 = *mTokenizer;

               // parse again, until we encounter some useful data
               again = true;
               break;

            default:
               throw CPPDOM_ERROR(xml_tagname_expected, "");
            }
         }
      } while (again);

      // insert tag name and set handle for it
      std::string tagname(token2.getGeneric());
      node.mNodeNameHandle = context->insertTagname(tagname);
#ifdef CPPDOM_DEBUG
      node.mNodeName_debug = tagname;
#endif

      // notify event handler
      if (handle)
      {
         context->getEventHandler().startNode(tagname);
      }

      // parse attributes
      this->parseAttributes(node.getAttrMap());

      if (handle)
      {
         context->getEventHandler().parsedAttributes(node.getAttrMap());
      }

      // check for leaf
      ++mTokenizer;
      Token token3 = *mTokenizer;
      if (token3 == '/' )
      {
         // node has finished
         ++mTokenizer;
         Token token4 = *mTokenizer;
         if (token4 != '>' )
         {
            throw CPPDOM_ERROR(xml_closetag_expected, "");
         }

         node.mNodeType = xml_nt_leaf;

         // return, let the caller continue to parse
         return true;
      }

      // now a closing bracket must follow
      if (token3 != '>')
      {
         throw CPPDOM_ERROR(xml_closetag_expected, "");
      }

      // loop to parse all subnodes
      while (true)
      {
         // create subnode
         NodePtr new_subnode(new Node(context));

         // try to parse possible sub nodes
         if (this->parseNode(*new_subnode, context))
         {
            // if successful, put node into nodelist
   //         NodePtr nodeptr( new Node(subnode) );
            node.addChild(new_subnode);
         }
         else
         {
            break;
         }
      }

      // parse end tag
      Token token5 = *mTokenizer++;
      ++mTokenizer;
      if (token5 != '<' && *mTokenizer != '/')
      {
         throw CPPDOM_ERROR(xml_opentag_expected, "");
      }

      ++mTokenizer;
      token1 = *mTokenizer;
      if (token1.isLiteral())
      {
         throw CPPDOM_ERROR(xml_tagname_expected, "");
      }

      // check if open and close tag names are identical
      if (token1.getGeneric() != token2.getGeneric())
      {
         throw CPPDOM_ERROR(xml_tagname_close_mismatch, "");
      }

      ++mTokenizer;
      if (*mTokenizer != '>')
      {
         throw CPPDOM_ERROR(xml_opentag_expected, "");
      }

      if (handle)
      {
         context->getEventHandler().endNode(node);
      }

      return true;
   }

   // parses tag attributes
   bool Parser::parseAttributes(Attributes& attr)
   {
      while(true)
      {
         ++mTokenizer;
         Token token1 = *mTokenizer;

         if (token1.isLiteral())
         {
            mTokenizer.putBack();
            return false;
         }

         // guru: get value name here
         std::string name = token1.getGeneric();

         ++mTokenizer;
         if (*mTokenizer != '=')
         {
            throw CPPDOM_ERROR(xml_attr_equal_expected, "");
         }

         ++mTokenizer;
         Token token2 = *mTokenizer;

         if (token2.isLiteral())
         {
            throw CPPDOM_ERROR(xml_attr_value_expected, "");
         }

         // remove "" from attribute value
         std::string value(token2.getGeneric());
         value.erase(0, 1);
         value.erase(value.length()-1, 1);

         // insert attribute into the map
         // guru: we got the name already
         Attributes::value_type attrpair(name, value);
         attr.insert(attrpair);
      }
      return true;
   }

   void Parser::parseComment(ContextPtr& context)
   {
      cppdom::ignore_unused_variable_warning(context);

      // get tokens until comment is over
      while (true)
      {
         ++mTokenizer;
         if (*mTokenizer == "--")
         {
            ++mTokenizer;
            if (*mTokenizer == '>')
            {
               break;
            }
         }
      }
   }
}
