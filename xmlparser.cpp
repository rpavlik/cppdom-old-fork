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
/*! \file XMLParser.cpp

  member functions for the tokenizer and parser classes

*/

// needed includes
#include <cppdom/xmlparser.h>

// namespace declaration
namespace cppdom {


// XMLParser methods

XMLParser::XMLParser( std::istream &inputstream, XMLLocation &loc )
:instream(inputstream), tokenizer(inputstream,loc)
{
}

bool XMLParser::parseDocument( XMLDocument &doc, XMLContextPtr &ctxptr )
{
   // set root nodename
   doc.contextptr = ctxptr;
   XMLString rootstr("root");
   doc.nodenamehandle = ctxptr->insertTagname( rootstr );

   bool handle = ctxptr->handleEvents();

   // start parsing
   if (handle)
      ctxptr->getEventHandler().startDocument();

   parseHeader( doc, ctxptr );

   // parse the only one subnode
   XMLNodePtr new_subnode(new XMLNode(ctxptr));

   bool ret = parseNode( *new_subnode, ctxptr );

   // if successful, put node into nodelist
   if (ret)
   {
//      XMLNodePtr nodeptr( new XMLNode(subnode) );
      doc.addChild( new_subnode );
   }

   if (handle)
      ctxptr->getEventHandler().endDocument();

   return ret;
}

// parses the header, ie processing instructions and doctype tag
//! \todo parse <!doctype> tag
bool XMLParser::parseHeader( XMLDocument &doc, XMLContextPtr &ctxptr )
{
   while(1==1)
   {
      tokenizer++;
      XMLToken token1 = *tokenizer;
      if (token1 != '<')
         throw XMLError( xml_opentag_expected );

      // token after opening < is a literal?
      tokenizer++;
      XMLToken token2 = *tokenizer;
      if (!token2.isLiteral())
      {
         // generic string encountered: assume no pi and doctype tags
         tokenizer.putBack();
         tokenizer.putBack(token1);
         return false;
      }

      // now check for the literal
      switch(token2.getLiteral())
      {
         // comment or doctype tag
      case '!':
         {
            tokenizer++;
            XMLToken token3 = *tokenizer;

            if (!token3.isLiteral())
            {
               // now a doctype tag or a comment may follow
               if (token3.getGeneric().at(0) == '-' &&
                   token3.getGeneric().at(1) == '-')
                   parseComment( ctxptr );
               else
               {
                  XMLString doctypestr(token3.getGeneric());

                  std::transform(doctypestr.begin(),doctypestr.end(),doctypestr.begin(),toupper);

                  if (doctypestr == "DOCTYPE")
                  {
                     // \todo parse doctype tag

                     // read the complete tag till the closing >
                     while (*(tokenizer++) != '>');
                  }
                  else
                     throw XMLError( xml_unknown );
               }
            }
            else
               throw XMLError( xml_pi_doctype_expected );

            break;
         }
      case '?':
         {
            tokenizer++;
            XMLToken token3 = *tokenizer;

            if (token3.isLiteral())
               throw XMLError( xml_pi_doctype_expected );

            // parse processing instruction
            XMLNode pinode( ctxptr );

            XMLString tagname( token3.getGeneric() );
            pinode.nodenamehandle = ctxptr->insertTagname( tagname );

            parseAttributes( pinode.attributes );

            XMLNodePtr nodeptr( new XMLNode(pinode) );
            doc.procinstructions.push_back( nodeptr );

            if (ctxptr->handleEvents()) ctxptr->getEventHandler().processingInstruction(pinode);

            tokenizer++;
            if (*tokenizer != '?')
               throw XMLError( xml_pi_doctype_expected );

            tokenizer++;
            if (*tokenizer != '>')
               throw XMLError( xml_closetag_expected );
            break;
         }
      default:
         // unknown literal encountered
         throw XMLError( xml_pi_doctype_expected );

      } // end switch

   } // end while
}

// parses the contents of the current node
bool XMLParser::parseNode( XMLNode &node, XMLContextPtr &ctxptr )
{
   node.contextptr = ctxptr;
   bool handle = ctxptr->handleEvents();

   tokenizer++;
   XMLToken token1 = *tokenizer;

   if (token1.isEndOfStream())
      return false;

   XMLToken token2;

   // loop when we encounter a comment
   bool again;
   do
   {
      again = false;

      // check if we have cdata
      if (!token1.isLiteral())
      {
         XMLString cdataname("cdata");
         node.nodenamehandle = ctxptr->insertTagname( cdataname );

         // parse cdata section(s) and return
         node.nodetype = xml_nt_cdata;
         node.mCdata.empty();

         while(!token1.isLiteral())
         {
            node.mCdata += token1.getGeneric();
            tokenizer++;
            token1 = *tokenizer;
         }
         tokenizer.putBack();

         if (handle)
            ctxptr->getEventHandler().gotCdata( node.mCdata );

         return true;
      }

      // no cdata, try to continue parsing node content
      // Must be a start of a node (ie. < literal)
      if (token1 != '<')
         throw XMLError(xml_opentag_cdata_expected);

      // get node name
      tokenizer++;
      token2 = *tokenizer;
      if (token2.isLiteral())
      {
         // check the following literal
         switch(token2.getLiteral())
         {
            // closing '</...>' follows
         case '/':
            // return, we have a closing node with no more content
            tokenizer.putBack();
            tokenizer.putBack( token1 );
            return false;

            // comment follows
         case '!':
            this->parseComment( ctxptr );

            // get next token
            tokenizer++;
            token1 = *tokenizer;

            // parse again, until we encounter some useful data
            again = true;
            break;

         default:
            throw XMLError(xml_tagname_expected);
         }
      }
   } while (again);

   // insert tag name and set handle for it
   XMLString tagname( token2.getGeneric() );
   node.nodenamehandle = ctxptr->insertTagname( tagname );

   // notify event handler
   if (handle)
      ctxptr->getEventHandler().startNode( tagname );

   // parse attributes
   this->parseAttributes( node.attributes );

   if (handle)
      ctxptr->getEventHandler().parsedAttributes( node.attributes );

   // check for leaf
   tokenizer++;
   XMLToken token3 = *tokenizer;
   if (token3 == '/' )
   {
      // node has finished
      tokenizer++;
      XMLToken token4 = *tokenizer;
      if (token4 != '>' )
         throw XMLError(xml_closetag_expected);

      node.nodetype = xml_nt_leaf;

      // return, let the caller continue to parse
      return true;
   }

   // now a closing bracket must follow
   if (token3 != '>')
      throw XMLError(xml_closetag_expected);

   // loop to parse all subnodes
   while (1==1)
   {
      // create subnode
      XMLNodePtr new_subnode(new XMLNode(ctxptr));

      // try to parse possible sub nodes
      if (this->parseNode( *new_subnode, ctxptr ))
      {
         // if successful, put node into nodelist
//         XMLNodePtr nodeptr( new XMLNode(subnode) );
         node.addChild( new_subnode );
      }
      else
         break;
   }

   // parse end tag
   XMLToken token5 = *tokenizer++;
   tokenizer++;
   if (token5 != '<' && *tokenizer != '/')
      throw XMLError(xml_opentag_expected);

   tokenizer++;
   token1 = *tokenizer;
   if (token1.isLiteral())
      throw XMLError(xml_tagname_expected);

   // check if open and close tag names are identical
   if (token1.getGeneric() != token2.getGeneric() )
      throw XMLError(xml_tagname_close_mismatch);

   tokenizer++;
   if (*tokenizer != '>')
      throw XMLError(xml_opentag_expected);

   if (handle) ctxptr->getEventHandler().endNode( node );

   return true;
}

// parses tag attributes
bool XMLParser::parseAttributes( XMLAttributes &attr )
{
   while(1==1)
   {
      tokenizer++;
      XMLToken token1 = *tokenizer;

      if (token1.isLiteral())
      {
         tokenizer.putBack();
         return false;
      }

      // guru: get value name here
      XMLString name = token1.getGeneric();

      tokenizer++;
      if (*tokenizer != '=')
         throw XMLError(xml_attr_equal_expected);

      tokenizer++;
      XMLToken token2 = *tokenizer;

      if (token2.isLiteral())
         throw XMLError(xml_attr_value_expected);

      // remove "" from attribute value
      XMLString value( token2.getGeneric() );
      value.erase(0,1);
      value.erase(value.length()-1,1);

      // insert attribute into the map
      // guru: we got the name already
      XMLAttributes::value_type attrpair(name, value);
      attr.insert(attrpair);
   }
   return true;
}

void XMLParser::parseComment( XMLContextPtr &ctxptr )
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
