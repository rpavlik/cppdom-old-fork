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
   cppdom::Document doc;
   doc = loadDocNoCatch(cppdomtest::game_xml_filename);
   doc = loadDocNoCatch(cppdomtest::nodetest_xml_filename);
   doc = loadDocNoCatch(cppdomtest::simple_nodes_xml_filename);
   doc = loadDocNoCatch(cppdomtest::rime_xml_filename);
   doc = loadDocNoCatch(cppdomtest::hamlet_xml_filename);
}


cppdom::Document ParseTest::loadDocNoCatch(std::string filename)
{
   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::Document doc( ctx );
   doc.loadFile( filename );
   return doc;
}

// ----- METRIC TEST ---- //

void ParseMetricTest::timeLoadTestDocs()
{
   cppdom::Document doc;
   std::vector<std::string> filenames;
   //filenames.push_back(cppdomtest::game_xml_filename);
   //filenames.push_back(cppdomtest::nodetest_xml_filename);
   filenames.push_back(cppdomtest::rime_xml_filename);
   filenames.push_back(cppdomtest::hamlet_xml_filename);
   
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

   }
}


cppdom::Document ParseMetricTest::loadDocNoCatch(std::string filename)
{
   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::Document doc( ctx );
   doc.loadFile( filename );
   return doc;
}

}
