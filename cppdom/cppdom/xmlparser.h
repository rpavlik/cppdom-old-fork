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
/*! \file xmlparser.hpp

  definitions for the parsing classes

*/

// prevent multiple includes
#ifndef __xmlparser_hpp_
#define __xmlparser_hpp_

// needed includes
#include <cppdom/cppdom.h>
#include <cppdom/xmltokenizer.h>

// namespace declaration
namespace cppdom {


//! xml parser implementation class
class xmlparser
{
public:
   //! ctor
   xmlparser( std::istream &inputstream, xmllocation &loc );

   //! parses the node as the document root
   bool parse_document( xmldocument &doc, xmlcontextptr &ctxptr );
   
   //! parses a node, without processing instructions
   bool parse_node( xmlnode &node, xmlcontextptr &ctxptr );

protected:
   //! parses xml header, such as processing instructions, doctype etc.
   bool parse_header( xmldocument &doc, xmlcontextptr &ctxptr );

   //! parses an xml tag attribute list
   bool parse_attributes( xmlattributes &attr );

   //! parses a <!-- --> comment 
   void parse_comment( xmlcontextptr &ctxptr );

protected:
   //! input stream
   std::istream &instream;
   //! stream iterator
   xmlstream_iterator tokenizer;
};

// namespace end
};

#endif
