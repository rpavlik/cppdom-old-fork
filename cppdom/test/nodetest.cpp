#include <iostream>
#include <string>
#include <xmlpp/xmlpp.h>

bool configureInput( const std::string& filename )
{
   xmlpp::XMLContextPtr ctx( new xmlpp::XMLContext );
   xmlpp::XMLDocument doc( ctx );

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
      return false;
   }

   std::cerr << "succesfully loaded " << filename << std::endl;

   xmlpp::XMLNodeList nl = doc.firstchild( "gameinput" )->children();
   xmlpp::XMLNodeListIterator it = nl.begin();
   while (it != nl.end())
   {
      std::cerr << "in name: " << (*it)->name() << std::endl;
      try
      {
         xmlpp::XMLAttributes& attr = (*it)->get_attrmap();
         std::cout << "attr: " << attr.get( "action" ) << "\n" << std::flush;
         std::cout << "attr: " << attr.get( "device" ) << "\n" << std::flush;
         std::cout << "attr: " << attr.get( "input" ) << "\n" << std::flush;
      }
      catch (xmlpp::xmlerror e)
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
