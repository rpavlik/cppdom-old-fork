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
/** \file XMLParser.hpp

  definitions for the parsing classes

*/

// prevent multiple includes
#ifndef CPPDOM_XML_PARSER_H
#define CPPDOM_XML_PARSER_H

// needed includes
#include <cppdom/cppdom.h>
#include <cppdom/xmltokenizer.h>

// namespace declaration
namespace cppdom
{

   /** xml parser implementation class */
   class XMLParser
   {
   public:
      /** ctor */
      XMLParser(std::istream& inputstream, XMLLocation& loc);

      /** parses the node as the document root */
      bool parseDocument(XMLDocument& doc, XMLContextPtr& ctxptr);

      /** parses a node, without processing instructions */
      bool parseNode(XMLNode& node, XMLContextPtr& ctxptr);

   protected:
      /** parses xml header, such as processing instructions, doctype etc. */
      bool parseHeader(XMLDocument& doc, XMLContextPtr& ctxptr);

      /** parses an xml tag attribute list */
      bool parseAttributes(XMLAttributes& attr);

      /** parses a <!-- --> comment */
      void parseComment(XMLContextPtr& ctxptr);

   protected:
      /** input stream */
      std::istream& instream;

      /** stream iterator */
      xmlstream_iterator tokenizer;
   };
}

#endif
