#include <iostream>
#include <string>
#include <cppdom/cppdom.h>

bool configureInput( const std::string& filename )
{
   cppdom::XMLContextPtr ctx( new cppdom::XMLContext );
   cppdom::XMLDocument doc( ctx );

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
      return false;
   }

   std::cerr << "succesfully loaded " << filename << std::endl;

   cppdom::XMLNodeList nl = doc.getChild( "gameinput" )->getChildren();
   cppdom::XMLNodeListIterator it = nl.begin();
   while (it != nl.end())
   {
      std::cerr << "in name: " << (*it)->getName() << std::endl;
      try
      {
         cppdom::XMLAttributes& attr = (*it)->get_attrmap();
         std::cout << "attr: " << attr.get( "action" ) << "\n" << std::flush;
         std::cout << "attr: " << attr.get( "device" ) << "\n" << std::flush;
         std::cout << "attr: " << attr.get( "input" ) << "\n" << std::flush;
      }
      catch (cppdom::xmlerror e)
      {
         std::cerr << "Error: " << e.get_string() << std::endl;
         it++;
         continue;
      }
      it++;
   }

   return true;
}

/** Main function */
int main()
{
   // Just call single function to load and process input file
   configureInput( "hamlet.xml" );
   return 1;
}
