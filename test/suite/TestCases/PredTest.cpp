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
#include <TestCases/PredTest.h>
#include <TestCases/TestData.h>
#include <Suites.h>
#include <iostream>
#include <fstream>

#include <cppdom/cppdom.h>
#include <cppdom/predicates.h>

namespace cppdomtest
{
CPPUNIT_TEST_SUITE_REGISTRATION(PredTest);



void PredTest::testHasAttributeNamePredicate()
{
   cppdom::Document doc = loadGameDoc();
   cppdom::NodePtr root = doc.getChild( "gameinput" );

   // get all nodes with noattrs attributes...
   cppdom::NodeList nl = root->getChildrenPred( cppdom::HasAttributeNamePredicate( "noattrs" ) );
   CPPUNIT_ASSERT( nl.size() == 1 && "test failed" );    
}


void PredTest::testHasAttributeValuePredicate()
{
   cppdom::Document doc = loadGameDoc();
   cppdom::NodePtr root = doc.getChild( "gameinput" );

   // get all the nodes that set up Mouse devices
   cppdom::NodeList nl = root->getChildrenPred( cppdom::HasAttributeValuePredicate( "device", "Mouse" ) );
   CPPUNIT_ASSERT( nl.size() == 2 && "test failed" );
}


cppdom::Document PredTest::loadGameDoc()
{
   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::Document doc( ctx );
   std::string filename(cppdomtest::game_xml_filename);
   
   try
   {
      doc.loadFile( filename );
   }
   catch (cppdom::Error e)
  {
     cppdom::Location where( ctx->getLocation() );
     std::string errmsg = e.getStrError();

     // print out where the error occured
     std::cout << filename << ":" << where.getLine() << " "
               << "at position " << where.getPos()
               << ": error: " << errmsg.c_str()
               << std::endl;

     // print out line where the error occured
     std::ifstream errfile( filename.c_str() );
     if(!errfile)
     {
        std::cerr << "Can't open file [" << filename << "] to output error" << std::endl;
     }

     int linenr = where.getLine();
     char linebuffer[1024];
     for(int i=0; i<linenr && !errfile.eof(); i++)
        errfile.getline( linebuffer,1024 );

     int pos = where.getPos();
     if (pos>=80)
        pos %= 80;

     std::string err_line( linebuffer + (where.getPos()-pos) );
     if (err_line.length()>=79)
        err_line.erase(79);
     std::cout << err_line << std::flush
               << err_line.c_str() << std::endl
               << linebuffer << std::endl;
     for(int j=2;j<pos;j++)
        std::cout << " ";
     std::cout << '^' << std::endl;
  }

   return doc;
}

}
