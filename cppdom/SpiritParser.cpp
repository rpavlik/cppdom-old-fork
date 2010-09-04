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
/** \file SpiritParser.cpp
  definitions for the spirit parsing classes
*/
#include <cppdom/SpiritParser.h>

namespace bs = boost::spirit;
using namespace boost::spirit;

namespace cppdom
{

namespace spirit
{
// Constructe an XmlBuilder
/*
XmlBuilder::XmlBuilder(cppdom::DocumentPtr doc, cppdom::ContextPtr context)
{
   reinit(doc,context);
}
*/


void Parser::parseDocument(cppdom::Document& doc, std::string& content)
{
   mCharBuilder.reinit(&doc, doc.getContext());
   XmlGrammar<XmlBuilder<const char*> > xml_grammar(&mCharBuilder);
   parse_info<char const*> result;

   result = bs::parse(content.c_str(), xml_grammar);
   if(!result.full)
   {
      throw CPPDOM_ERROR(xml_invalid_operation, "Invalid format of XML.");
   }
}

void Parser::parseDocument(cppdom::Document& doc, std::istream& instream)
{
    typedef char char_t;
    typedef bs::multi_pass<std::istream_iterator<char_t> > iterator_t;

    //typedef skip_parser_iteration_policy<space_parser> iter_policy_t;
    //typedef scanner_policies<iter_policy_t> scanner_policies_t;
    //typedef scanner<iterator_t, scanner_policies_t> scanner_t;

    //typedef rule<scanner_t> rule_t;

    //iter_policy_t iter_policy(space_p);
    //scanner_policies_t policies(iter_policy);

    typedef  XmlBuilder<iterator_t> builder_t;

    builder_t local_builder;
    local_builder.reinit(&doc, doc.getContext());

    XmlGrammar<builder_t> xml_grammar(&local_builder);
    bs::parse_info<iterator_t> result;

    iterator_t first( bs::make_multi_pass( std::istream_iterator<char_t>(instream)));
    iterator_t last( bs::make_multi_pass( std::istream_iterator<char_t>()));

    result = bs::parse(first, last, xml_grammar, bs::space_p);
    if(!result.full)
    {
       throw CPPDOM_ERROR(xml_invalid_operation, "Invalid format of XML.");
    }

    //std::string test(first,last);
    //parseDocument(doc,test);
}



}  // namespace spirit
} // namespace cppdom

