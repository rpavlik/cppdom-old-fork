#include <xmlpp/predicates.h>

int main()
{
   xmlpp::XMLContextPtr ctx( new xmlpp::XMLContext );
   xmlpp::XMLDocument doc( ctx );
   std::string filename = "hamlet.xml";
   
   // load a xml document from a file
   try
   {
      doc.load_file( filename );
   }
   catch (xmlpp::xmlerror e)
   {
      std::cerr << "Error: " << e.get_string() << std::endl;
      if (e.get_info().size())
      {
         std::cerr << "File: " << e.get_info() << std::endl;
      }
      if (e.get_error() != xmlpp::xml_filename_invalid &&
          e.get_error() != xmlpp::xml_file_access)
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

   xmlpp::XMLNodePtr root = doc.getChild( "gameinput" );

      

   // get all nodes with noattrs attributes...
   xmlpp::XMLNodeList nl = root->getChildrenPred( xmlpp::AttributeType( "noattrs" ) );
   
   assert( nl.size() == 1 && "test failed" );
   
   // get all the nodes that set up Mouse devices
   nl = root->getChildrenPred( xmlpp::AttributeValue( "device", "Mouse" ) );
   assert( nl.size() == 2 && "test failed" );
  
   std::cout << "Tests Passed" << std::endl;
}
