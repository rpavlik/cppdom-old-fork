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
#include <TestCases/NodeTest.h>
#include <TestCases/TestData.h>
#include <extensions/MetricRegistry.h>

#include <Suites.h>
#include <iostream>
#include <fstream>

#include <cppdom/cppdom.h>
#include <cppdom/predicates.h>
#include <testHelpers.h>

namespace cppdomtest
{
CPPUNIT_TEST_SUITE_REGISTRATION(NodeTest);

// Test equality test (isEqual())
// - Uses an xml file that is coded to specify several
//   tests to use for this
//
// The file has a set of nodes one for each test.
//   under that node is information about the test as
//   well as two nodes to use to run the test.
// Ex:
/*
<root>
   <test id="Basic not equal ignoring attrib" equal="0" ignore_attr="attrib" ignore_elem="blah">
   Test basic inequality. where we are ignoring the differing attribute.
      <node data="X" attrib="1"/>
      <node attrib="2" data="Y"/>
   </test>
</root>
*/
void NodeTest::testEqual()
{
   const std::string id_token("id");
   const std::string equal_token("equal");
   const std::string ignore_attr_token("ignore_attr");
   const std::string ignore_elem_token("ignore_elem");


   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::Document doc( ctx );
   std::string test_filename = "data/equal_test.xml";

   // load test test file
   try
   {
      doc.loadFile( test_filename );
   }
   catch (cppdom::Error e)
   {
      std::cerr << "Error: " << e.getString() << std::endl;
      if (e.getInfo().size())
      {
         std::cerr << "File: " << e.getInfo() << std::endl;
      }

      CPPUNIT_ASSERT(false && "Failed to load equal_test.xml file for equal test:");
      return;
   }

   cppdom::NodePtr root = doc.getChild( "root" );
   CPPUNIT_ASSERT(root.get() != NULL);

   // Get all the child tests
   cppdom::NodeList tests = root->getChildren( "test" );
   CPPUNIT_ASSERT(tests.size() > 4);                       // Just make sure we got some

   // For each test
   // - Make sure right type
   // - Get the attributes of the test
   // - Load the two child nodes
   // - Run the test and print results
   for(cppdom::NodeList::iterator cur_elt = tests.begin(); cur_elt != tests.end(); ++cur_elt)
   {
      cppdom::NodePtr child1, child2;
      cppdom::NodePtr cur_test = *cur_elt;
      CPPUNIT_ASSERT(cur_test->getName() == std::string("test"));
      std::string test_id = cur_test->getAttribute(id_token);
      std::string eq_value = cur_test->getAttribute(equal_token);
      bool should_be_equal = (eq_value == std::string("1"));

      bool has_ignore_attrib = cur_test->hasAttribute(ignore_attr_token);
      std::string attrib_ignore = cur_test->getAttribute(ignore_attr_token);
      bool has_elem_ignore = cur_test->hasAttribute(ignore_elem_token);
      std::string elem_ignore = cur_test->getAttribute(ignore_elem_token);

      // Put together the list of attributes and elements to ignore in comparison
      std::vector<std::string> attrib_list;
      std::vector<std::string> element_list;
      if(has_ignore_attrib)
      { attrib_list.push_back(attrib_ignore); }
      if(has_elem_ignore)
      { element_list.push_back(elem_ignore); }

      // Load children
      // - Get the first two non-cdata nodes
      cppdom::NodeList nl = cur_test->getChildren();
      CPPUNIT_ASSERT(nl.size() >= 2);
      unsigned cur_child=0;

      while((nl[cur_child]->getType() != Node::xml_nt_node) &&
            (nl[cur_child]->getType() != Node::xml_nt_leaf))
      {  cur_child++; }
      child1 = nl[cur_child];
      cur_child++;                  // Goto next child

      while((nl[cur_child]->getType() != Node::xml_nt_node) &&
            (nl[cur_child]->getType() != Node::xml_nt_leaf))
      {  cur_child++; }
      child2 = nl[cur_child];

      // Run test
      bool is_equal = child1->isEqual(child2, attrib_list, element_list);
      bool passed = (is_equal == should_be_equal);

      if(!passed)
      {
         std::cout << " --- equal test failed: " << test_id << std::endl
                   << "should be equal: " << should_be_equal << std::endl
                   << "has ignore: " << has_ignore_attrib << std::endl
                   << "ignore: " << attrib_ignore << std::endl
                   << "-------- child1: ---------" << std::endl;
         testHelpers::dump_node(*child1);
         std::cout << "\n---------- child2: -------" << std::endl;
         testHelpers::dump_node(*child2);
         std::cout << "--------------------------" << std::endl;

         CPPUNIT_ASSERT(false && "Failed equal test");
      }
   }

}

}
