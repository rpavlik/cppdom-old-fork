// Parse test application
// Based upon: parsetest.cpp from xmlpp

// needed includes
#include <fstream>
#include <iostream>
//#include <ctime>
#include <cppdom/cppdom.h>

// namespace includes
using namespace cppdom;
using namespace std;


// dumps the node
void dump_node( Node &node, int level = 0 )
{
   std::string name = node.getName();
   NodeType type = node.getType();
   std::string c_data;

   for(int i=0;i<level;i++) cout << " ";

   char c = ' ';
   switch(type)
   {
   case xml_nt_node:
      c = '+';
      break;
   case xml_nt_leaf:
      c = '-';
      break;
   case xml_nt_document:
      c = '\\';
      break;
   case xml_nt_cdata:
      c = '#';
      c_data = node.getCdata();
      break;
   }

   if(type == xml_nt_cdata)
      cout << c << name.c_str() << "[" << c_data << "]" << endl;
   else
      cout << c << name.c_str() << endl;

   Attributes attr = node.getAttrMap();

   // guru: added output of attributes
   for (Attributes::iterator j = attr.begin(); j!=attr.end(); j++)
   {
      for (int i=0; i<level; i++)
         cout << " ";
      cout << "   ";
      cout << j->first << ": " << j->second << endl;
   }

   NodeList& nlist = node.getChildren();

   NodeList::const_iterator iter, stop;
   iter = nlist.begin();
   stop = nlist.end();

   while (iter != stop)
   {
      NodePtr node = *iter;

      dump_node ( *node, level+1 );

      ++iter;
   }
};

void process_xml( std::string filename )
{
   cout << "processing [" << filename << "] ..." << endl;

   ContextPtr context( new Context );
   Document node( context );
   ifstream istr( filename.c_str() );

   // Verify that file opened
   if(!istr)
   {
      std::cerr << "Bad file: " << filename << std::endl;
      return;
   }

   try
   {
//      clock_t tstart = ::clock();

      node.load( istr, context );

//      clock_t tstop = ::clock();
//      cout << " needed " <<
//         (tstop-tstart)/static_cast<float>(CLOCKS_PER_SEC)
//         << " seconds." << endl;

      dump_node( node );

      ofstream ostr( "parsetest.xml" );
      node.save( ostr );
      ostr.close();

   }
   catch (Error e)
   {
      Location where( context->getLocation() );
      std::string errmsg;
      e.getStrError(errmsg);

      // print out where the error occured
      cout << filename << ":" << where.getLine() << " ";
      cout << "at position " << where.getPos();
      cout << ": error: " << errmsg.c_str();
      cout << endl;

      // print out line where the error occured
      ifstream errfile( filename.c_str() );
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
      cout << err_line << std::flush;
      cout << err_line.c_str() << std::endl;
      cout << linebuffer << std::endl;
      for(int j=2;j<pos;j++)
         std::cout << " ";
      cout << '^' << endl;
   }
}

int main(int argc, char* argv[])
{
   for(int i=1;i<argc;i++)
   {
      process_xml( std::string(argv[i]) );
   }

   return 0;
}
