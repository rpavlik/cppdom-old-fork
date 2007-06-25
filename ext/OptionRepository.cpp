#include <cppdom/ext/OptionRepository.h>

#include <sstream>
#include <assert.h>

namespace cppdom
{

OptionRepository::OptionRepository(std::string rootTagName)
{
   mDocRoot = DocumentPtr(new Document(ContextPtr(new Context)));
   mOptionRoot = cppdom::NodePtr(new cppdom::Node(rootTagName, mDocRoot->getContext()));     // Create empty option node
   mOptionRootTag = rootTagName;
   mDocRoot->addChild(mOptionRoot);
}


std::string OptionRepository::getOptionString(std::string option)
{
   std::string option_path, attrib_name;
   splitOptionPath(option,option_path,attrib_name);

   NodePtr option_node(mOptionRoot);   // Default to getting it off the root
   if(!option_path.empty())            // If not just attribute off root
   {
      option_node = mOptionRoot->getChildPath(option_path);
   }

   if(option_node.get() == NULL)
   {  throw CPPDOM_ERROR(xml_invalid_argument, std::string("Could not find node for option:") + option); }

   return option_node->getAttribute(attrib_name).getString();
}

bool OptionRepository::hasOption(std::string option)
{
   std::string option_path, attrib_name;
   splitOptionPath(option,option_path,attrib_name);

   NodePtr option_node(mOptionRoot);   // Default to getting it off the root
   if(!option_path.empty())            // If not just attribute off root
   {
      option_node = mOptionRoot->getChildPath(option_path);
   }

   if(option_node.get() == NULL)
   {
      return false;
   }
   else
   {
      return option_node->hasAttribute(attrib_name);
   }
}

void OptionRepository::splitOptionPath(const std::string& option, std::string& option_path, std::string& attrib_name)
{
   const std::string::size_type attrib_start( option.find_last_of('/') );

   if((0 == attrib_start) || (option.length()-1 == attrib_start))
   {  throw CPPDOM_ERROR(xml_invalid_argument, std::string("Invalid option path:") + option); }

   // If "/" not found, then we are off the root
   if(attrib_start == std::string::npos)
   {
      option_path = "";
      attrib_name = option;
   }
   else
   {
      option_path = option.substr(0,attrib_start);
      attrib_name = option.substr(attrib_start+1,option.length()-attrib_start-1);
   }
}

/** Set the value of an option.
 * @param option     The option to set the value for.
 * @param value      The value to set option to.
 */
void OptionRepository::setOptionString(std::string option, std::string value)
{
   std::string option_path, attrib_name;
   splitOptionPath(option,option_path,attrib_name);

   std::vector<std::string> node_path;
   // If we have more then just an attrib off the root
   if(!option_path.empty())
   {
      cppdom::splitStr(option_path, "/", std::back_inserter(node_path));
   }

   NodePtr option_node = mOptionRoot;

   // For each link in path, find the node or create the node
   // If no links in path, then we just keep with the current path
   for(unsigned i=0; i<node_path.size(); ++i)
   {
      const std::string cur_branch(node_path[i]);
      if (option_node->hasChild(cur_branch))
      {
         option_node = option_node->getChild(cur_branch);
      }
      else
      {
         NodePtr new_node(new Node(cur_branch, option_node->getContext()));
         option_node->addChild(new_node);
         option_node = option_node->getChild(cur_branch);
      }
   }

   // ASSERT: Now we have a node with the full path we were looking for
   option_node->setAttribute(attrib_name, value);

   // ASSERT: Insertion completed correctly
   assert(getOptionString(option) == value);
}



void OptionRepository::loadOptionsFile(std::string filename)
{
   // Load the document and merge it
   // XXX: In future this should really merge the options
   mDocRoot->getChildren().clear(); // Clear all children
   mDocRoot->loadFile(filename);
   cppdom::NodePtr options_node = mDocRoot->getChild(mOptionRootTag);
   if(options_node.get() == NULL)
   {
      std::ostringstream oss;
      oss << "Option file does not have root options element: ["<< mOptionRootTag <<"] in file [" << filename << "]";
      throw CPPDOM_ERROR(xml_invalid_operation, oss.str());
   }

   mOptionRoot = options_node;
}

void OptionRepository::saveOptionsFile(std::string filename)
{
   mDocRoot->getPiList().clear();
   mDocRoot->saveFile(filename);
}


}  // namespace terra
