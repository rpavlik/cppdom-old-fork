/*
   xmlpp - an xml parser and validator written in C++
   (c) 2000 Michael Fink
   (c) 2001 Karl Pitrich
      
   $Id$
*/

/*! \file xmlparser.h

  definitions for the parsing classes

*/

#ifndef __xmlparser_h__
#define __xmlparser_h__

#include <cstddef>            // for std::size_t
#include <memory>             // for std::auto_ptr
#include <algorithm>          // for std::swap
#include <functional>         // for std::less
#include "xmlpp.h"
#include "xmltokenizer.h"
#include "xmlhelpers.h"


namespace xmlpp {

//! xml parser implementation class
class xmlparser {
public:
   //! ctor
   xmlparser( std::istream &inputstream, xmllocation &loc );
   //! parses the node as the document root
   bool parse_document( XMLDocument &doc, XMLContextPtr &ctxptr );
   //! parses a node, without processing instructions
   bool parse_node( XMLNode &node, XMLContextPtr &ctxptr );

protected:
   //! parses xml header, such as processing instructions, doctype etc.
   bool parse_header( XMLDocument &doc, XMLContextPtr &ctxptr );
   //! parses an xml tag attribute list
   bool parse_attributes( XMLAttributes &attr );
   //! parses a <!-- --> comment 
   void parse_comment( XMLContextPtr &ctxptr );
   //! input stream
   std::istream &instream;
   //! stream iterator
   xmlstream_iterator tokenizer;
};

}; // namespace end

#endif
/* vi: set ts=3: */
