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

#define BOOST_SPIRIT_DEBUG 1

#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>

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

}
