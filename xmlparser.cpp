// this xmlpp was branched from the original LGPL'd xmlpp version 0.6
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
/*! \file xmlparser.cpp

  member functions for the tokenizer and parser classes

*/

// needed includes
#include <xmlpp/xmlparser.h>

// namespace declaration
namespace xmlpp {


// xmlparser methods

xmlparser::xmlparser( std::istream &inputstream, xmllocation &loc )
:instream(inputstream), tokenizer(inputstream,loc)
{
}

bool xmlparser::parse_document( xmldocument &doc, xmlcontextptr &ctxptr )
{
   // set root nodename
   doc.contextptr = ctxptr;
   xmlstring rootstr("root");
   doc.nodenamehandle = ctxptr->insert_tagname( rootstr );

   bool handle = ctxptr->handle_events();

   // start parsing
   if (handle)
      ctxptr->get_eventhandler().start_document();

   parse_header( doc, ctxptr );

   // parse the only one subnode
   xmlnode subnode( ctxptr );

   bool ret = parse_node( subnode, ctxptr );

   // if successful, put node into nodelist
   if (ret)
   {
      xmlnodeptr nodeptr( new xmlnode(subnode) );
      doc.mNodelist.push_back( nodeptr );
   }

   if (handle)
      ctxptr->get_eventhandler().end_document();

   return ret;
}

// parses the header, ie processing instructions and doctype tag
//! \todo parse <!doctype> tag
bool xmlparser::parse_header( xmldocument &doc, xmlcontextptr &ctxptr )
{
   while(1==1)
   {
      tokenizer++;
      xmltoken token1 = *tokenizer;
      if (token1 != '<')
         throw xmlerror( xml_opentag_expected );

      // token after opening < is a literal?
      tokenizer++;
      xmltoken token2 = *tokenizer;
      if (!token2.is_literal())
      {
         // generic string encountered: assume no pi and doctype tags
         tokenizer.put_back();
         tokenizer.put_back(token1);
         return false;
      }

      // now check for the literal
      switch(token2.get_literal())
      {
         // comment or doctype tag
      case '!':
         {
            tokenizer++;
            xmltoken token3 = *tokenizer;

            if (!token3.is_literal())
            {
               // now a doctype tag or a comment may follow
               if (token3.get_generic().at(0) == '-' &&
                   token3.get_generic().at(1) == '-')
                   parse_comment(ctxptr);
               else
               {
                  xmlstring doctypestr(token3.get_generic());

                  std::transform(doctypestr.begin(),doctypestr.end(),doctypestr.begin(),toupper);

                  if (doctypestr == "DOCTYPE")
                  {
                     // \todo parse doctype tag

                     // read the complete tag till the closing >
                     while (*(tokenizer++) != '>');
                  }
                  else
                     throw xmlerror( xml_unknown );
               }
            }
            else
               throw xmlerror( xml_pi_doctype_expected );

            break;
         }
      case '?':
         {
            tokenizer++;
            xmltoken token3 = *tokenizer;

            if (token3.is_literal())
               throw xmlerror( xml_pi_doctype_expected );

            // parse processing instruction
            xmlnode pinode( ctxptr );

            xmlstring tagname( token3.get_generic() );
            pinode.nodenamehandle = ctxptr->insert_tagname( tagname );

            parse_attributes( pinode.attributes );

            xmlnodeptr nodeptr( new xmlnode(pinode) );
            doc.procinstructions.push_back( nodeptr );

            if (ctxptr->handle_events()) ctxptr->get_eventhandler().processing_instruction(pinode);

            tokenizer++;
            if (*tokenizer != '?')
               throw xmlerror( xml_pi_doctype_expected );

            tokenizer++;
            if (*tokenizer != '>')
               throw xmlerror( xml_closetag_expected );
            break;
         }
      default:
         // unknown literal encountered
         throw xmlerror( xml_pi_doctype_expected );

      } // end switch

   } // end while
}

// parses the contents of the current node
bool xmlparser::parse_node( xmlnode &node, xmlcontextptr &ctxptr )
{
   node.contextptr = ctxptr;
   bool handle = ctxptr->handle_events();

   tokenizer++;
   xmltoken token1 = *tokenizer;

   if (token1.is_endofstream())
      return false;

   xmltoken token2;

   // loop when we encounter a comment
   bool again;
   do
   {
      again = false;

      // check if we have cdata
      if (!token1.is_literal())
      {
         xmlstring cdataname("cdata");
         node.nodenamehandle = ctxptr->insert_tagname( cdataname );

         // parse cdata section(s) and return
         node.nodetype = xml_nt_cdata;
         node.mCdata.empty();

         while(!token1.is_literal())
         {
            node.mCdata += token1.get_generic();
            tokenizer++;
            token1 = *tokenizer;
         }
         tokenizer.put_back();

         if (handle)
            ctxptr->get_eventhandler().got_cdata(node.mCdata);

         return true;
      }

      // no cdata, try to continue parsing node content

      if (token1 != '<')
         throw xmlerror(xml_opentag_cdata_expected);

      // get node name
      tokenizer++;
      token2 = *tokenizer;
      if (token2.is_literal())
      {
         // check the following literal
         switch(token2.get_literal())
         {
            // closing '</...>' follows
         case '/':
            // return, we have a closing node with no more content
            tokenizer.put_back();
            tokenizer.put_back( token1 );
            return false;

            // comment follows
         case '!':
            parse_comment(ctxptr);

            // get next token
            tokenizer++;
            token1 = *tokenizer;

            // parse again, until we encounter some useful data
            again = true;
            break;

         default:
            throw xmlerror(xml_tagname_expected);
         }
      }
   } while (again);

   // insert tag name and set handle for it
   xmlstring tagname( token2.get_generic() );
   node.nodenamehandle = ctxptr->insert_tagname( tagname );

   // notify event handler
   if (handle)
      ctxptr->get_eventhandler().start_node(tagname);

   // parse attributes
   parse_attributes(node.attributes);

   if (handle)
      ctxptr->get_eventhandler().parsed_attributes(node.attributes);

   // check for leaf
   tokenizer++;
   xmltoken token3 = *tokenizer;
   if (token3 == '/' )
   {
      // node has finished
      tokenizer++;
      xmltoken token4 = *tokenizer;
      if (token4 != '>' )
         throw xmlerror(xml_closetag_expected);

      node.nodetype = xml_nt_leaf;

      // return, let the caller continue to parse
      return true;
   }

   // now a closing bracket must follow
   if (token3 != '>')
      throw xmlerror(xml_closetag_expected);

   // loop to parse all subnodes
   while (1==1)
   {
      // create subnode
      xmlnode subnode( ctxptr );

      // try to parse possible sub nodes
      if (parse_node( subnode, ctxptr ))
      {
         // if successful, put node into nodelist
         xmlnodeptr nodeptr( new xmlnode(subnode) );
         node.mNodelist.push_back( nodeptr );
      }
      else
         break;
   }

   // parse end tag
   xmltoken token5 = *tokenizer++;
   tokenizer++;
   if (token5 != '<' && *tokenizer != '/')
      throw xmlerror(xml_opentag_expected);

   tokenizer++;
   token1 = *tokenizer;
   if (token1.is_literal())
      throw xmlerror(xml_tagname_expected);

   // check if open and close tag names are identical
   if (token1.get_generic() != token2.get_generic() )
      throw xmlerror(xml_tagname_close_mismatch);

   tokenizer++;
   if (*tokenizer != '>')
      throw xmlerror(xml_opentag_expected);

   if (handle) ctxptr->get_eventhandler().end_node(node);

   return true;
}

// parses tag attributes
bool xmlparser::parse_attributes( xmlattributes &attr )
{
   while(1==1)
   {
      tokenizer++;
      xmltoken token1 = *tokenizer;

      if (token1.is_literal())
      {
         tokenizer.put_back();
         return false;
      }

      // guru: get value name here
      xmlstring name = token1.get_generic();

      tokenizer++;
      if (*tokenizer != '=')
         throw xmlerror(xml_attr_equal_expected);

      tokenizer++;
      xmltoken token2 = *tokenizer;

      if (token2.is_literal())
         throw xmlerror(xml_attr_value_expected);

      // remove "" from attribute value
      xmlstring value( token2.get_generic() );
      value.erase(0,1);
      value.erase(value.length()-1,1);

      // insert attribute into the map
      // guru: we got the name already
      xmlattributes::value_type attrpair(name, value);
      attr.insert(attrpair);
   }
   return true;
}

void xmlparser::parse_comment( xmlcontextptr &ctxptr )
{
   // get tokens until comment is over
   while (1==1)
   {
      tokenizer++;
      if (*tokenizer == "--")
      {
         tokenizer++;
         if (*tokenizer == '>')
            break;
      }
   }
}

// namespace end
};
