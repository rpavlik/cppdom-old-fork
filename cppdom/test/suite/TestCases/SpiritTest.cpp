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
#include <TestCases/SpiritTest.h>
#include <Suites.h>
#include <iostream>

#include <cppdom/cppdom.h>

//#define BOOST_SPIRIT_DEBUG 1
//#define BOOST_SPIRIT_DEBUG_PRINT_SOME 100

#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/spirit/utility.hpp>

namespace bs = boost::spirit;
using namespace boost::spirit;

namespace cppdomtest
{
CPPUNIT_TEST_SUITE_REGISTRATION(SpiritTest);

struct AbsGrammar : public grammar<AbsGrammar>
{
   template <typename ScannerT>
   struct definition
   {
      definition(AbsGrammar const& self)
      {
         as = ch_p('a') >> !as;
         bs = +ch_p('b');
         expr = *(as >> bs);         
         BOOST_SPIRIT_DEBUG_RULE(as);
         BOOST_SPIRIT_DEBUG_RULE(bs);
         BOOST_SPIRIT_DEBUG_RULE(expr);
      }
      
      rule<ScannerT> as, bs, expr;
      
      rule<ScannerT> const& start() const 
      { return expr; }
   };   
};

   
void SpiritTest::testBasics()
{
   // Match a list of numbers
   bool full;
   
   std::cout << "Num list:" << std::endl;
   
   full = bs::parse("1.0, 3.0, 4.0",
           (
              real_p >> *(',' >> real_p)
           ),
           space_p).full;
           
   CPPUNIT_ASSERT(full);
   
   std::cout << "abs (no rules):" << std::endl;
   
      full = bs::parse("aabbbabbaaaabb",
           (
              *(+ch_p('a') >> +ch_p('b'))
           )).full;
           
   CPPUNIT_ASSERT(full);

   std::cout << "abs (rules):" << std::endl;
      
   rule<> as, bs, expr;
   as = +ch_p('a');
   bs = +ch_p('b');
   expr = *(as >> bs);
   
   BOOST_SPIRIT_DEBUG_RULE(as);
   BOOST_SPIRIT_DEBUG_RULE(bs);
   BOOST_SPIRIT_DEBUG_RULE(expr);
   
   full = bs::parse("abaabbabbb", expr).full;
      
   CPPUNIT_ASSERT(full);
   
   std::cout << "abs (grammar):" << std::endl;
   
   AbsGrammar abs_grammar;
   BOOST_SPIRIT_DEBUG_GRAMMAR(abs_grammar);
   full = bs::parse("abaabbabbb", abs_grammar).full;
   
   CPPUNIT_ASSERT(full);
}

void printElt(char const* first, char const* last)
{
   std::string str(first,last);
   std::cout << "Elt: [" << str << "]" << std::endl;
}

void printEltExit(char const* first, char const* last)
{
   std::cout << "elt exit: " << std::string(first,last) << std::endl;
}

/**
* XML grammar.
* Based on: http://www.w3.org/TR/2004/REC-xml-20040204/
*/
struct XmlGrammar : public grammar<XmlGrammar>
{
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
         
         element = ch_p('<') >> name[&printElt] >> *(ws >> attribute) >> *space_p 
                   >> ( (ch_p('>') >> elem_content >> str_p("</") >> name[&printEltExit] >> *space_p >> ch_p('>')) |
                        (str_p("/>"))[&printEltExit] );
                   
         elem_content = !char_data >> *( (element | comment | pi | cdata_sect) >> !char_data);
         
         attribute = name >> *space_p >> '=' >> *space_p >> attrib_value;
         attrib_value = ('"' >> *(anychar_p-'"') >> '"') |
                        ("'" >> *(anychar_p-"'") >> "'");

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

void SpiritTest::testXmlParser()
{
   std::cout << "xml grammar" << std::endl;
   
   XmlGrammar xml_grammar;
   BOOST_SPIRIT_DEBUG_GRAMMAR(xml_grammar);

   parse_info<char const*> result;

   std::cout << "-------------------------" << std::endl;
   result = bs::parse(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
            "<!DOCTYPE greeting [<!ELEMENT greeting (#PCDATA)>]>"
            "<greeting>Hello, world!</greeting>"
            "<!-- declarations for <head> & <body> -->",
            xml_grammar);
   CPPUNIT_ASSERT(result.full);

   std::cout << "-------------------------" << std::endl;
   result = bs::parse(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
            "<root><node param=\"arg\" p2 = \"2\" >node text</node></root>"
            ,xml_grammar);
   CPPUNIT_ASSERT(result.full);

   std::cout << "-------------------------" << std::endl;
   result = bs::parse(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
            "<root>"
            "<node><node><node>"
            "<single_node arg=\"alone\"/>"
            "</node></node></node>"
            "</root>"
            ,xml_grammar);
   CPPUNIT_ASSERT(result.full);
   
   std::cout << "-------------------------" << std::endl;
   result = bs::parse(
            "<root><node1><node2><node3><node4>"
            "Stuff<!-- More here -->And more<b>this</b>dfdf"
            "</node4>txt and txt </node3>  aa</node2></node1>"
            "</root>"
            ,xml_grammar);
   CPPUNIT_ASSERT(result.full);
   
}

}


