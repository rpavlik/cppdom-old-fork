#include <iostream>
#include <assert.h>
#include <cppdom/predicates.h>

int main()
{
   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::Document doc( ctx );
   std::string filename = "game.xml";
   
   // load a xml document from a file
   try
   {
      doc.loadFile( filename );
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

   cppdom::NodePtr root = doc.getChild( "gameinput" );

      

   // get all nodes with noattrs attributes...
   cppdom::NodeList nl = root->getChildrenPred( cppdom::HasAttributeNamePredicate( "noattrs" ) );
   
   assert( nl.size() == 1 && "test failed" );
   
   // get all the nodes that set up Mouse devices
   nl = root->getChildrenPred( cppdom::HasAttributeValuePredicate( "device", "Mouse" ) );
   assert( nl.size() == 2 && "test failed" );
  
   std::cout << "Tests Passed" << std::endl;
}
