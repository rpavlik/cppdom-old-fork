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

#include <cppdom/SpiritParser.h>

namespace bs = boost::spirit;
using namespace boost::spirit;

namespace
{
/** Callback builder class to use for testing.
*/
class TestXmlBuilder
{
public:
   /** Call when parser is done. */
   void finish()
   {;}

   void startElement(char const* first, char const* last)
   {  std::cout << "StartElement: " << std::string(first,last) << std::endl; }

   void endElement(char const* first, char const* last)
   {  std::cout << "endElement: " << std::string(first,last) << std::endl; }
   void endElementQuick(char const* first, char const* last)
   {  std::cout << "endElement quick." << std::endl; }


   void startAttribute(char const* first, char const* last)
   {  std::cout << "startAttribute: " << std::string(first,last) << std::endl; }

   void attribValue(char const* first, char const* last)
   {  std::cout << "attribValue: " << std::string(first,last) << std::endl; }

   void elementText(char const* first, char const* last)
   {  std::cout << "elementText: " << std::string(first,last) << std::endl; }
};

}

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

void SpiritTest::testXmlParser()
{
   std::cout << "xml grammar" << std::endl;

   TestXmlBuilder test_builder;
   cppdom::spirit::XmlGrammar<TestXmlBuilder> xml_grammar(&test_builder);
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
            "<root><node1 attrib='value'>"
            "<node2 a1='val a1' a2='1.567' a3='false'>"
            "<node3><node4>"
            "Stuff<!-- More here -->And more<b>this</b>dfdf"
            "</node4>txt and txt </node3>  aa</node2></node1>"
            "</root>"
            ,xml_grammar);
   CPPUNIT_ASSERT(result.full);

   std::string xml_content1(
            "<root><node1 attrib='value'>"
            "<node2 a1='val a1' a2='1.567' a3='false'>"
            "<node3><node4>"
            "Stuff<!-- More here -->And &lt;more<b>this</b>dfdf"
            "</node4>txt and txt </node3>  aa</node2></node1>"
            "</root>"
            );

   cppdom::spirit::Parser spirit_parser;
   cppdom::ContextPtr ctx(new cppdom::Context());

   std::cout << "---------------------- Parsing document (char*) -----------" << std::endl;
   cppdom::DocumentPtr doc1(new cppdom::Document(ctx));
   spirit_parser.parseDocument(*doc1, xml_content1);
   std::cout << "---------------------- Document output (char*) -----------" << std::endl;
   doc1->save(std::cout);
   std::cout << std::endl;

   std::cout << "---------------------- Parsing document (istream) ---------" << std::endl;
   cppdom::DocumentPtr doc2(new cppdom::Document(ctx));
   std::istringstream xml_content1_istream(xml_content1);
   spirit_parser.parseDocument(*doc2, xml_content1_istream);
   std::cout << "---------------------- Document (istream) -----------" << std::endl;
   doc2->save(std::cout);
   std::cout << std::endl;

}


}  // namespace cppdomtest


