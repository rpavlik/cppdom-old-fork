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
#include <TestCases/ParseTest.h>
#include <TestCases/TestData.h>
#include <extensions/MetricRegistry.h>

#include <Suites.h>
#include <iostream>
#include <fstream>

#include <cppdom/cppdom.h>

namespace cppdomtest
{
CPPUNIT_TEST_SUITE_REGISTRATION(ParseTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ParseMetricTest, cppdomtest::Suites::metric());

void ParseTest::loadTestDocs()
{
   cppdom::DocumentPtr doc;
   doc = loadDocNoCatch(cppdomtest::game_xml_filename);
   doc = loadDocNoCatch(cppdomtest::nodetest_xml_filename);
   doc = loadDocNoCatch(cppdomtest::simple_nodes_xml_filename);
   doc = loadDocNoCatch(cppdomtest::rime_xml_filename);
   doc = loadDocNoCatch(cppdomtest::hamlet_xml_filename);
//   doc = loadDocNoCatch(cppdomtest::xml_spec_filename);
}


cppdom::DocumentPtr ParseTest::loadDocNoCatch(std::string filename)
{
   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::DocumentPtr doc( new cppdom::Document(ctx) );
   doc->loadFile( filename );
   return doc;
}

// ----- METRIC TEST ---- //

void ParseMetricTest::timeLoadTestDocs()
{
   cppdom::DocumentPtr doc;
   std::vector<std::string> filenames;
   //filenames.push_back(cppdomtest::game_xml_filename);
   //filenames.push_back(cppdomtest::nodetest_xml_filename);
   filenames.push_back(cppdomtest::rime_xml_filename);
   filenames.push_back(cppdomtest::hamlet_xml_filename);
//   filenames.push_back(cppdomtest::xml_spec_filename);

   const unsigned iters(10);

   for(unsigned fi=0;fi<filenames.size();++fi)
   {
      std::string filename = filenames[fi];
      std::string metric_name = std::string("ParseMetricTest/timeLoadTestDocs/") + filename;

      CPPUNIT_METRIC_START_TIMING();

      for(unsigned loop=0; loop<iters; ++loop)
      {
         doc = loadDocNoCatch(filename);
      }

      CPPUNIT_METRIC_STOP_TIMING();
      CPPUNIT_ASSERT_METRIC_TIMING_LE(metric_name, iters, 0.03f, 0.05f);

      doc = cppdom::DocumentPtr(NULL);    // Clear memory
   }
}

/** Time the saving and loading of radom docs of various sizes.
 * This code creates several random documens of sizes ranging from small to large.
 * For each size, we create the file, write it out, then time the loading of the file.
 */
void ParseMetricTest::timeLoadSaveSmallRandomDoc()
{
   std::string metric_name_root("ParseMetricTest/timeLoadSaveRandomDocs/");
   // small -7
      const std::string filename("ParseMetricTestDoc_Small.xml");
      cppdom::DocumentPtr doc_root = createDocument(7);
      doc_root->saveFile(filename);
      doc_root = cppdom::DocumentPtr(NULL);        // Clear memory

      const unsigned iters(20);
      CPPUNIT_METRIC_START_TIMING();

      for(unsigned loop=0; loop<iters; ++loop)
      {  doc_root = loadDocNoCatch(filename);  }

      CPPUNIT_METRIC_STOP_TIMING();
      CPPUNIT_ASSERT_METRIC_TIMING_LE(metric_name_root+"small", iters, 0.03f, 0.05f);
}

void ParseMetricTest::timeLoadSaveMediumRandomDoc()
{
   std::string metric_name_root("ParseMetricTest/timeLoadSaveRandomDocs/");
  // med - 8
   const std::string filename("ParseMetricTestDoc_Medium.xml");
   cppdom::DocumentPtr doc_root = createDocument(8);
   doc_root->saveFile(filename);
   doc_root = cppdom::DocumentPtr(NULL);        // Clear memory

   const unsigned iters(8);
   CPPUNIT_METRIC_START_TIMING();

   for(unsigned loop=0; loop<iters; ++loop)
   {  doc_root = loadDocNoCatch(filename);  }

   CPPUNIT_METRIC_STOP_TIMING();
   CPPUNIT_ASSERT_METRIC_TIMING_LE(metric_name_root+"medium", iters, 0.03f, 0.05f);
}

void ParseMetricTest::timeLoadSaveLargeRandomDoc()
{
   std::string metric_name_root("ParseMetricTest/timeLoadSaveRandomDocs/");
   // large - 10
   const std::string filename("ParseMetricTestDoc_Large.xml");
   cppdom::DocumentPtr doc_root = createDocument(10);
   doc_root->saveFile(filename);
   doc_root = cppdom::DocumentPtr(NULL);        // Clear memory

   const unsigned iters(5);
   CPPUNIT_METRIC_START_TIMING();

   for(unsigned loop=0; loop<iters; ++loop)
   {  doc_root = loadDocNoCatch(filename);  }

   CPPUNIT_METRIC_STOP_TIMING();
   CPPUNIT_ASSERT_METRIC_TIMING_LE(metric_name_root+"large", iters, 0.03f, 0.05f);
}

void ParseMetricTest::timeLoadSaveHugeRandomDoc()
{
   std::string metric_name_root("ParseMetricTest/timeLoadSaveRandomDocs/");
   // huge - 12
   const std::string filename("ParseMetricTestDoc_Huge.xml");
   cppdom::DocumentPtr doc_root = createDocument(11);
   doc_root->saveFile(filename);
   doc_root = cppdom::DocumentPtr(NULL);        // Clear memory

   const unsigned iters(3);
   CPPUNIT_METRIC_START_TIMING();

   for(unsigned loop=0; loop<iters; ++loop)
   {  doc_root = loadDocNoCatch(filename);  }

   CPPUNIT_METRIC_STOP_TIMING();
   CPPUNIT_ASSERT_METRIC_TIMING_LE(metric_name_root+"huge", iters, 0.03f, 0.05f);
}


cppdom::DocumentPtr ParseMetricTest::loadDocNoCatch(std::string filename)
{
   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::DocumentPtr doc( new cppdom::Document(ctx) );
   doc->loadFile( filename );
   return doc;
}

// Create a psuedo random document
cppdom::DocumentPtr ParseMetricTest::createDocument(unsigned depth, unsigned seed)
{
   srand48(seed);    // Seed randomness

   // Fill up the elt and attrib names
   mAvailElementNames.clear();
   mAvailElementNames.push_back("escher_elt");
   mAvailElementNames.push_back("person");
   mAvailElementNames.push_back("place");
   mAvailElementNames.push_back("thing");
   mAvailElementNames.push_back("idea");
   mAvailElementNames.push_back("extreme_programming_tests");
   mAvailElementNames.push_back("small_node");
   mAvailElementNames.push_back("ThisAndThat");
   mAvailElementNames.push_back("MoreStuff");
   mAvailElementNames.push_back("one_element");
   mAvailElementNames.push_back("two_element");
   mAvailElementNames.push_back("three_element");
   mAvailElementNames.push_back("four_element");
   mAvailElementNames.push_back("five_element");
   mAvailAttribNames.clear();
   mAvailAttribNames.push_back("name");
   mAvailAttribNames.push_back("size");
   mAvailAttribNames.push_back("color");
   mAvailAttribNames.push_back("one_attrib");
   mAvailAttribNames.push_back("two_attrib");
   mAvailAttribNames.push_back("three_attrib");
   mAvailAttribNames.push_back("four_attrib");
   mAvailAttribNames.push_back("five_attrib");
   mAvailAttribNames.push_back("six_attrib");
   mAvailAttribNames.push_back("seven_attrib");
   mAvailAttribNames.push_back("eight_attrib");
   mAvailAttribNames.push_back("nine_attrib");
   mAvailAttribValues.clear();
   mAvailAttribValues.push_back("yes");
   mAvailAttribValues.push_back("no");
   mAvailAttribValues.push_back("1");
   mAvailAttribValues.push_back("0");
   mAvailAttribValues.push_back("56789.45698");
   mAvailAttribValues.push_back("0.00000007");
   mAvailAttribValues.push_back("This is a test");
   mAvailAttribValues.push_back("my value is good");

   mLocalContext = cppdom::ContextPtr( new cppdom::Context());
   cppdom::DocumentPtr doc_node(new cppdom::Document("doc_base",mLocalContext));

   cppdom::NodePtr child_node = createBranch(depth);
   doc_node->addChild(child_node);
   return doc_node;
}


// Create a random branch of given depth
cppdom::NodePtr ParseMetricTest::createBranch(unsigned depth)
{
   std::string node_name = mAvailElementNames[getRandomNum(mAvailElementNames.size())];
   cppdom::NodePtr cur_node(new cppdom::Node(node_name,mLocalContext));

   const float max_attribs(7.0f);
   double rand_attrib_d(drand48());    // Calc X^4
   unsigned num_attribs(unsigned( rand_attrib_d*rand_attrib_d*rand_attrib_d*rand_attrib_d * max_attribs));

   for(unsigned a=0;a<num_attribs;++a)
   {
      std::string attrib_name = mAvailAttribNames[getRandomNum(mAvailAttribNames.size())];
      cur_node->setAttribute(attrib_name, mAvailAttribValues[getRandomNum(mAvailAttribValues.size())]);
   }

   if(depth > 0)
   {
      unsigned num_children(getRandomNum(depth+1));

      for(unsigned c=0;c<num_children;++c)
      {
         cppdom::NodePtr child_node = createBranch(depth-1);
         cur_node->addChild(child_node);
      }
   }

   return cur_node;
}



}
