//
// Test for isEqual method of Node
//
// Uses external equal_test.xml file to
// specify and run the tests
//
#include <iostream>
#include <assert.h>
#include <cppdom/predicates.h>
#include <testHelpers.h>

const std::string id_token("id");
const std::string equal_token("equal");
const std::string ignore_attr_token("ignore_attr");
const std::string ignore_elem_token("ignore_elem");

int main()
{
   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::Document doc( ctx );
   std::string test_filename = "equal_test.xml";

   // load test test file
   try
   {
      std::cout << "Loading test file: " << test_filename << std::endl;
      doc.loadFile( test_filename );
   }
   catch (cppdom::Error e)
   {
      std::cerr << "Error: " << e.getString() << std::endl;
      if (e.getInfo().size())
      {
         std::cerr << "File: " << e.getInfo() << std::endl;
      }
      if (e.getError() != cppdom::xml_filename_invalid &&
          e.getError() != cppdom::xml_file_access)
      {
/*
         e.show_error( ctx );
         e.show_line( ctx, filename );
*/
         std::cout << "Error: (need to impl the show functions)" << std::endl;
      }
      assert( false && "test failed" );
      return false;
   }

   cppdom::NodePtr root = doc.getChild( "root" );
   assert(root.get() != NULL);

   // Get all the child tests
   cppdom::NodeList tests = root->getChildren( "test" );
   assert(tests.size() > 4);                             // Just make sure we got some

   std::cout << "\nRunning tests:\n";
   bool has_test_failures(false);

   // For each test
   // - Make sure right type
   // - Get the attributes of the test
   // - Load the two child nodes
   // - Run the test and print results
   for(cppdom::NodeList::iterator cur_elt = tests.begin(); cur_elt != tests.end(); ++cur_elt)
   {
      cppdom::NodePtr child1, child2;
      cppdom::NodePtr cur_test = *cur_elt;
      assert(cur_test->getName() == std::string("test"));
      std::string test_id = cur_test->getAttribute(id_token);
      std::string eq_value = cur_test->getAttribute(equal_token);
      bool should_be_equal = (eq_value == std::string("1"));

      bool has_ignore_attrib = cur_test->hasAttribute(ignore_attr_token);
      std::string attrib_ignore = cur_test->getAttribute(ignore_attr_token);
      bool has_elem_ignore = cur_test->hasAttribute(ignore_elem_token);
      std::string elem_ignore = cur_test->getAttribute(ignore_elem_token);

      std::vector<std::string> attrib_list;
      std::vector<std::string> element_list;
      if(has_ignore_attrib)
      { attrib_list.push_back(attrib_ignore); }
      if(has_elem_ignore)
      { element_list.push_back(elem_ignore); }

      // Load children
      // - Get the first two non-cdata nodes
      cppdom::NodeList nl = cur_test->getChildren();
      assert(nl.size() >= 2);
      unsigned cur_child=0;

      while((nl[cur_child]->getType() != cppdom::xml_nt_node) &&
            (nl[cur_child]->getType() != cppdom::xml_nt_leaf))
      {  cur_child++; }
      child1 = nl[cur_child];
      cur_child++;                  // Goto next child

      while((nl[cur_child]->getType() != cppdom::xml_nt_node) &&
            (nl[cur_child]->getType() != cppdom::xml_nt_leaf))
      {  cur_child++; }
      child2 = nl[cur_child];

      // Run test
      bool is_equal = child1->isEqual(child2, attrib_list, element_list);
      bool passed = (is_equal == should_be_equal);

      std::cout << "   " << test_id << ": ";
      if(passed)
         std::cout << "passed.\n";
      else
      {
         has_test_failures = true;
         std::cout << "FAILED:\n";
         std::cout << "should be equal: " << should_be_equal << std::endl
                   << "has ignore: " << has_ignore_attrib << std::endl
                   << "ignore: " << attrib_ignore << std::endl
                   << "-------- child1: ---------" << std::endl;
         testHelpers::dump_node(*child1);
         std::cout << "\n---------- child2: -------" << std::endl;
         testHelpers::dump_node(*child2);
         std::cout << std::endl;
      }
   }

   if(has_test_failures)
      std::cout << "\n\nHAS FAILURES!!!!\n\nFix them and try again.\n";
   else
      std::cout << "\n\nAll tests pass.\n";

   return 1;
}
