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
#include <TestCases/OptionRepositoryTest.h>
#include <TestCases/TestData.h>
#include <extensions/MetricRegistry.h>

#include <Suites.h>
#include <iostream>
#include <fstream>

#include <cppdom/cppdom.h>
#include <cppdom/predicates.h>
#include <cppdom/ext/OptionRepository.h>
#include <testHelpers.h>

namespace cppdomtest
{
CPPUNIT_TEST_SUITE_REGISTRATION(OptionRepositoryTest);

void OptionRepositoryTest::testOptionAccess()
{
   //cppdom::Document doc(cppdom::ContextPtr(new cppdom::Context));
   //doc.loadFileChecked(cppdomtest::optionstest_xml_filename);

   cppdom::OptionRepository opts;
   opts.loadOptionsFile(cppdomtest::optionstest_xml_filename);    // Load the options

   // Get options by each value type
   int int_val = opts.getValue<int>("option_1/intval");
   CPPUNIT_ASSERT(1 == int_val);
   float float_val = opts.getValue<float>("option_1/floatval");
   CPPUNIT_ASSERT(12.21f == float_val);
   std::string str_val = opts.getValue<std::string>("option_1/stringval");
   CPPUNIT_ASSERT(std::string("value") == str_val);

   // Test a nested option
   str_val = opts.getOptionString("opt1/opt1_1/opt1_1_1/value");
   CPPUNIT_ASSERT(std::string("test") == str_val);

   // Test testing for option existence
   bool has_opt = opts.hasOption("option_1/intval");
   CPPUNIT_ASSERT(has_opt);
   has_opt = opts.hasOption("option_1/not_there_attrib");
   CPPUNIT_ASSERT(!has_opt);
   has_opt = opts.hasOption("option_1/not_elt/intval");
   CPPUNIT_ASSERT(!has_opt);
   has_opt = opts.hasOption("not/there/attrib");
   CPPUNIT_ASSERT(!has_opt);
   has_opt = opts.hasOption("opt1/opt1_1/opt1_1_1/value");
   CPPUNIT_ASSERT(has_opt);

   // Test getting an attribute off the root
   int_val = opts.getValue<int>("root_attrib");
   CPPUNIT_ASSERT(21 == int_val);

   // Test getting string option with spaces in it
   str_val = opts.getValue<std::string>("option_2/stringval");
   CPPUNIT_ASSERT(std::string("string value") == str_val);

   // Test getting nodes manually
   cppdom::NodePtr filelist_node = opts.optionsNode()->getChild("filelist");
   cppdom::NodeList file_nodes = filelist_node->getChildren("file");
   CPPUNIT_ASSERT(file_nodes.size() == 3);


}

void OptionRepositoryTest::testOptionSetting()
{
   cppdom::OptionRepository opts;
   opts.loadOptionsFile(cppdomtest::optionstest_xml_filename);    // Load the options

   // Test getting and setting a known value
   int int_val = opts.getValue<int>("option_1/intval");
   CPPUNIT_ASSERT(1 == int_val);

   opts.setValue("option_1/intval", 21);
   int_val = opts.getValue<int>("option_1/intval");
   CPPUNIT_ASSERT(21 == int_val);

   // Test setting a value that is not set yet
   CPPUNIT_ASSERT(opts.hasOption("test_set_option/intval") == false);
   opts.setValue("test_set_option/intval", 12);
   CPPUNIT_ASSERT(opts.hasOption("test_set_option/intval") == true);
   int_val = opts.getValue<int>("test_set_option/intval");
   CPPUNIT_ASSERT(12 == int_val);

   // Test setting an option that is attrib off the root
   CPPUNIT_ASSERT(opts.hasOption("test_set_root") == false);
   opts.setValue("test_set_root", "test string");
   CPPUNIT_ASSERT(opts.hasOption("test_set_root") == true);
   std::string str_val = opts.getValue<std::string>("test_set_root");
   CPPUNIT_ASSERT(std::string("test string") == str_val);
}


}
