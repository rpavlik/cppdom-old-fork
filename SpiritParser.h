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
/** \file SpiritParser.h

  definitions for the spirit parsing classes

*/
#ifndef CPPDOM_SPIRIT_PARSER_H
#define CPPDOM_SPIRIT_PARSER_H

#include <cppdom/cppdom.h>

//#define BOOST_SPIRIT_DEBUG 1
//#define BOOST_SPIRIT_DEBUG_PRINT_SOME 100
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/spirit/utility.hpp>
#include <boost/bind.hpp>

namespace bs = boost::spirit;
using namespace boost::spirit;

namespace cppdom
{

namespace spirit
{

/** Class for building an xml node tree from a spirit parser.
 */
class XmlBuilder
{
public:
   void startElement(char const* first, char const* last);
   void endElement(char const* first, char const* last);
   void startAttribute(char const* first, char const* last);
   void attribValue(char const* first, char const* last);
   void elementText(char const* first, char const* last);
};

/**
* XML grammar.
* Based on: http://www.w3.org/TR/2004/REC-xml-20040204/
*/
struct XmlGrammar : public grammar<XmlGrammar>
{
   XmlGrammar(XmlBuilder* builder)
      : mBuilder(builder)
   {;}

   XmlBuilder* mBuilder;

   /** Grammar definition. */
   template <typename ScannerT>
   struct definition
   {
      definition(XmlGrammar const& self)
      {
         document = prolog >> element >> *misc;       // Main document root
         ws = +space_p;                               // Whitespace, simplified from XML spec

         chset<> char_data_set(anychar_p - chset_p("<&"));

         //char_data_char = anychar_p - (chset_p('<') | chset_p('&'));
         char_data = *char_data_set;

         // Name specifications.  Pretty strict for now
         name_char = alnum_p | '_' | '-' | ':';
         name = (alpha_p | '_' | ':') >> *(alnum_p | '_' | '-' | ':');
         names = name >> *(blank_p >> name);
         nmtoken = +name_char;

         comment = confix_p("<!--", *anychar_p, "-->");  // Slightly less strict.

         pi = str_p("<?") >> name >> anychar_p >> str_p("?>");

         cdata_sect = str_p("<![CDATA[") >> cdata >> str_p("]]>");
         cdata = *anychar_p;

         misc = comment | pi | ws;
         prolog = !xmldecl >> *misc >> !(doctypedecl >> *misc);
         xmldecl = confix_p("<?xml", *anychar_p, "?>");        // Use confix since any_char cosumes everything

         doctypedecl = str_p("<!DOCTYPE")
                           >> *(anychar_p - chset_p("[>"))
                           >> !('[' >> *(anychar_p - ']') >> ']')
                           >> *space_p >> '>';

         // element = (start element) >> ( (more elts >> end) | (quick end))
         element = ch_p('<') >> name[boost::bind(&XmlBuilder::startElement,self.mBuilder,_1,_2)] >> *(ws >> attribute) >> *space_p
                   >> ( (ch_p('>') >> elem_content >> str_p("</")
                                   >> name[boost::bind(&XmlBuilder::endElement,self.mBuilder,_1,_2)] >> *space_p >> ch_p('>')) |
                        (str_p("/>"))[boost::bind(&XmlBuilder::endElement,self.mBuilder,_1,_2)] );

         // elementText: eliminate extra space by consuming initial whitespace & only call when char_data matches
         elem_content = *space_p >> !(char_data[boost::bind(&XmlBuilder::elementText,self.mBuilder,_1,_2)])
                          >> *( (element | comment | pi | cdata_sect) >> *space_p >> !(char_data[boost::bind(&XmlBuilder::elementText,self.mBuilder,_1,_2)]) );

         attribute = name[boost::bind(&XmlBuilder::startAttribute,self.mBuilder,_1,_2)] >> *space_p >> '=' >> *space_p >> attrib_value;
         attrib_value = ('"' >> (*(anychar_p-'"'))[boost::bind(&XmlBuilder::attribValue,self.mBuilder,_1,_2)] >> '"') |
                        ("'" >> (*(anychar_p-"'"))[boost::bind(&XmlBuilder::attribValue,self.mBuilder,_1,_2)] >> "'");

         BOOST_SPIRIT_DEBUG_RULE(attribute);
         BOOST_SPIRIT_DEBUG_RULE(attrib_value);
         BOOST_SPIRIT_DEBUG_RULE(cdata_sect);
         BOOST_SPIRIT_DEBUG_RULE(cdata);
         BOOST_SPIRIT_DEBUG_RULE(char_data);
         BOOST_SPIRIT_DEBUG_RULE(char_data_char);
         BOOST_SPIRIT_DEBUG_RULE(comment);
         BOOST_SPIRIT_DEBUG_RULE(document);
         BOOST_SPIRIT_DEBUG_RULE(doctypedecl);
         BOOST_SPIRIT_DEBUG_RULE(element);
         BOOST_SPIRIT_DEBUG_RULE(elem_content);
         BOOST_SPIRIT_DEBUG_RULE(misc);
         BOOST_SPIRIT_DEBUG_RULE(name);
         BOOST_SPIRIT_DEBUG_RULE(name_char);
         BOOST_SPIRIT_DEBUG_RULE(names);
         BOOST_SPIRIT_DEBUG_RULE(nmtoken);
         BOOST_SPIRIT_DEBUG_RULE(pi);
         BOOST_SPIRIT_DEBUG_RULE(prolog);
         BOOST_SPIRIT_DEBUG_RULE(ws);
         BOOST_SPIRIT_DEBUG_RULE(xmldecl);
      }

      rule<ScannerT> attribute,
                     attrib_value,
                     cdata_sect,
                     cdata,
                     char_data,
                     char_data_char,
                     comment,
                     document,
                     doctypedecl,
                     element,
                     elem_content,
                     misc,
                     name,
                     name_char,
                     names,
                     nmtoken,
                     pi,
                     prolog,
                     ws,
                     xmldecl
                     ;

      rule<ScannerT> const& start() const
      { return document; }
   };
};

}  // namespace spirit


} // namespace cppdom


#endif

