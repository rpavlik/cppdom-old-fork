#include <cppdom/predicates.h>

int main()
{
   cppdom::XMLContextPtr ctx( new cppdom::XMLContext );
   cppdom::XMLDocument doc( ctx );
   std::string filename = "hamlet.xml";
   
   // load a xml document from a file
   try
   {
      doc.load_file( filename );
   }
   catch (cppdom::xmlerror e)
   {
      std::cerr << "Error: " << e.get_string() << std::endl;
      if (e.get_info().size())
      {
         std::cerr << "File: " << e.get_info() << std::endl;
      }
      if (e.get_error() != cppdom::xml_filename_invalid &&
          e.get_error() != cppdom::xml_file_access)
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

   cppdom::XMLNodePtr root = doc.getChild( "gameinput" );

      

   // get all nodes with noattrs attributes...
   cppdom::XMLNodeList nl = root->getChildrenPred( cppdom::HasAttributeNamePredicate( "noattrs" ) );
   
   assert( nl.size() == 1 && "test failed" );
   
   // get all the nodes that set up Mouse devices
   nl = root->getChildrenPred( cppdom::HasAttributeValuePredicate( "device", "Mouse" ) );
   assert( nl.size() == 2 && "test failed" );
  
   std::cout << "Tests Passed" << std::endl;
}
