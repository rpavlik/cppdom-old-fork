#include <cppdom/ext/OptionRepository.h>

#include <sstream>

namespace cppdom
{

OptionRepository::OptionRepository(std::string rootTagName)
{
   cppdom::ContextPtr context(new cppdom::Context);
   mOptionRoot = cppdom::NodePtr(new cppdom::Node(rootTagName,context));     // Create empty option node
   mOptionRootTag = rootTagName;
}


std::string OptionRepository::getOptionString(std::string option)
{
   std::string option_path;
   std::string attrib_name;

   std::string::size_type attrib_start = option.find_last_of('/');
   if(attrib_start == std::string::npos)
   {  throw CPPDOM_ERROR(xml_invalid_argument, std::string("Could not find '/' in option:") + option); }
   if((0 == attrib_start) || (option.length()-1 == attrib_start))
   {  throw CPPDOM_ERROR(xml_invalid_argument, std::string("Invalid option path:") + option); }

   option_path = option.substr(0,attrib_start);
   attrib_name = option.substr(attrib_start+1,option.length()-attrib_start-1);

   NodePtr option_node = mOptionRoot->getChildPath(option_path);
   if(option_node.get() == NULL)
   {  throw CPPDOM_ERROR(xml_invalid_argument, std::string("Could not find node for option:") + option); }

   return option_node->getAttribute(attrib_name).getString();
}

/** Set the value of an option.
 * @param option     The option to set the value for.
 * @param value      The value to set option to.
 */
void OptionRepository::setOptionString(std::string option, std::string value)
{
   throw CPPDOM_ERROR(xml_invalid_argument, std::string("setOptionString not implemented yet."));
}



void OptionRepository::loadOptionsFile(std::string filename)
{
   // Load the document and merge it
   cppdom::Document doc(cppdom::ContextPtr(new cppdom::Context));
   doc.loadFile(filename);
   cppdom::NodePtr options_node = doc.getChild(mOptionRootTag);
   if(options_node.get() == NULL)
   {
      std::ostringstream oss;
      oss << "Option file does not have root options element: ["<< mOptionRootTag <<"] in file [" << filename << "]";
      throw CPPDOM_ERROR(xml_invalid_operation, oss.str());
   }

   mOptionRoot = options_node;
}

}  // namespace terra
