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
#include <stdexcept>

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>

#include <extensions/MetricRegistry.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <cppunit/ui/text/TestRunner.h>

#include <cppdom/version.h>

#include <Suites.h>

std::string getHostname(void);

int main(int argc, char** argv)
{
   // Commandline parameter is the test path to use
   std::string test_path = (argc > 1) ? std::string(argv[1]) : "noninteractive";

   // Print out what version of GMTL we're testing.
   std::cout << "\nTesting CppDOM Version: " << cppdom::getVersion() << std::endl << std::endl;

   // -------- CONFIGURE METRIC REGISTRY ------- //
   CppUnit::MetricRegistry* metric_reg = CppUnit::MetricRegistry::instance();

   std::string metric_prefix = getHostname() + "/";
#ifdef _DEBUG
   metric_prefix += "Debug/";
#endif
#ifdef _OPT
   metric_prefix += "Opt/";
#endif

   std::cout << "Setting up metrics for host: " << getHostname() << std::endl;
   std::cout << "                     prefix: " << metric_prefix << std::endl;

   metric_reg->setPrefix(metric_prefix);
   metric_reg->setFilename("data/cppdom_metrics.txt");
   metric_reg->setMetric("Main/MetricTest", 1221.75f);

   //------------------------------------
   //  Test suites
   //------------------------------------
   CppUnit::TestFactoryRegistry& global_registry = CppUnit::TestFactoryRegistry::getRegistry();

   // noninteractive
   CppUnit::TestSuite* noninteractive_suite = new CppUnit::TestSuite("noninteractive");
   noninteractive_suite->addTest(global_registry.makeTest());

   // metric
   CppUnit::TestSuite* metric_suite = new CppUnit::TestSuite(cppdomtest::Suites::metric());
   metric_suite->addTest(CppUnit::TestFactoryRegistry::getRegistry(cppdomtest::Suites::metric()).makeTest());

   //------------------------------------
   // Test Runner
   //------------------------------------
   CppUnit::TextUi::TestRunner runner;

   // Make it use a compiler outputter
   CppUnit::Outputter* run_outputter =
      CppUnit::CompilerOutputter::defaultOutputter(&runner.result(), std::cout);
   runner.setOutputter(run_outputter);

   // Add a listener that prints the test names as the tests progress
   CppUnit::TestResult& result_event_manager = runner.eventManager();
   CppUnit::BriefTestProgressListener progress;
   result_event_manager.addListener(&progress);

   runner.addTest(noninteractive_suite);
   runner.addTest(metric_suite);

   //------------------------------------
   // Run Tests
   //------------------------------------
   bool success(false);

   try
   {
      std::cout << "Running " << test_path << std::endl;
      success = runner.run(test_path);
   }
   catch (std::invalid_argument& e)
   {
      // Test path was not resolved
      std::cerr   << std::endl
                  << "ERROR: " << e.what()
                  << std::endl;
      success = false;
   }

   return (success ? 0 : 1);
}

#ifdef WIN32

std::string getHostname(void)
{
   return "WindowBox";
}

#else

#include <sys/utsname.h>

std::string getHostname(void)
{
   struct utsname buffer;

   if ( uname(&buffer) == 0 )
   {
      char* temp;
      temp = strchr(buffer.nodename, '.');

      // If the node name contains the full host, dots and all, truncate it
      // at the first dot.
      if ( temp != NULL )
      {
         *temp = '\0';
      }

      return std::string(buffer.nodename);
   }
   else
   {
      return std::string("<hostname-lookup failed>");
   }
}
#endif
