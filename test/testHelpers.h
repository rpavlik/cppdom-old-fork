#ifndef _CPPDOM_TEST_HELPERS_
#define _CPPDOM_TEST_HELPERS_

#include <iostream>
#include <cppdom/cppdom.h>

using namespace cppdom;

namespace testHelpers
{

   // dumps the node
void dump_node( Node &node, int level = 0 )
{
   std::string name = node.getName();
   NodeType type = node.getType();
   std::string c_data;

   for(int i=0;i<level;i++) std::cout << " ";

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
      std::cout << c << name.c_str() << "[" << c_data << "]" << std::endl;
   else
      std::cout << c << name.c_str() << std::endl;

   Attributes attr = node.getAttrMap();

   // guru: added output of attributes
   for (Attributes::iterator j = attr.begin(); j!=attr.end(); j++)
   {
      for (int i=0; i<level; i++)
         std::cout << " ";
      std::cout << "   ";
      std::cout << j->first << ": " << j->second << std::endl;
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
}


}

#endif

