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
/** \file XMLParser.hpp

  definitions for the parsing classes

*/

// prevent multiple includes
#ifndef CPPDOM_XML_PARSER_H
#define CPPDOM_XML_PARSER_H

// needed includes
#include "cppdom.h"
#include "xmltokenizer.h"

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
      bool parseDocument(XMLDocument& doc, XMLContextPtr& context);

      /** parses a node, without processing instructions */
      bool parseNode(XMLNode& node, XMLContextPtr& context);

   protected:
      /** parses xml header, such as processing instructions, doctype etc. */
      bool parseHeader(XMLDocument& doc, XMLContextPtr& context);

      /** parses an xml tag attribute list */
      bool parseAttributes(XMLAttributes& attr);

      /** parses a <!-- --> comment */
      void parseComment(XMLContextPtr& context);

   protected:
      /** input stream */
      std::istream& mInput;

      /** stream iterator */
      xmlstream_iterator mTokenizer;
   };
}

#endif
