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
#ifndef CPPDOM_TEST_PARSE_TEST_H
#define CPPDOM_TEST_PARSE_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppdom/cppdom.h>
#include <stdlib.h>

namespace cppdomtest
{

class ParseTest : public CppUnit::TestFixture
{

CPPUNIT_TEST_SUITE(ParseTest);
CPPUNIT_TEST(loadTestDocs);
CPPUNIT_TEST_SUITE_END();

public:

   /** Just load up a bunch of test documents with no errors. */
   void loadTestDocs();

public:
   // Load the named file without catching exceptions
   cppdom::DocumentPtr loadDocNoCatch(std::string filename);
};


class ParseMetricTest : public CppUnit::TestFixture
{

CPPUNIT_TEST_SUITE(ParseMetricTest);
CPPUNIT_TEST(timeLoadTestDocs);
CPPUNIT_TEST(timeLoadSaveSmallRandomDoc);
CPPUNIT_TEST(timeLoadSaveMediumRandomDoc);
CPPUNIT_TEST(timeLoadSaveLargeRandomDoc);
//CPPUNIT_TEST(timeLoadSaveHugeRandomDoc);
CPPUNIT_TEST_SUITE_END();

public:

   /** Time loading of a bunch of test documents. */
   void timeLoadTestDocs();

   /** Time the saving and loading of a large document. */
   void timeLoadSaveSmallRandomDoc();
   void timeLoadSaveMediumRandomDoc();
   void timeLoadSaveLargeRandomDoc();
   void timeLoadSaveHugeRandomDoc();

public:
   // Load the named file without catching exceptions
   cppdom::DocumentPtr loadDocNoCatch(std::string filename);

   // Create a psuedo random document
   cppdom::DocumentPtr createDocument(unsigned depth, unsigned seed=21);

   // Create a random branch of given depth
   cppdom::NodePtr createBranch(unsigned depth);

   unsigned getRandomNum(unsigned max)
   {  return unsigned(float(max)*drand48()); }


protected:
   std::vector<std::string>   mAvailElementNames;     // List of elt names to choose from
   std::vector<std::string>   mAvailAttribNames;      // Available attrib names to use
   std::vector<std::string>   mAvailAttribValues;      // Available attrib names to use
   cppdom::ContextPtr         mLocalContext;          // The context to use
};

}

#endif
