#include <assert.h>
#include <cppdom/cppdom.h>
#include <testHelpers.h>


/**
* Test application to test creating a document from scratch
*/
int main()
{
   // Create the basic context and document to work on
   // - All nodes need to have an associated context
   //   we will just define one here at the beginning and use it
   //   the entire way through
   cppdom::ContextPtr ctx( new cppdom::Context );
   cppdom::Document doc("Document", ctx );

   // What it should look like
   // - Root
   //   - Element1: attrib1:1, attrib2:two
   //        cdata: This is element1
   //   - Element2:
   //      - Element3: attrib1:attrib1
   //        cdata: We are element 3
   //        cdata: We are still element 3
   cppdom::NodePtr root(new cppdom::Node("root", ctx));
   cppdom::NodePtr element1(new cppdom::Node("Element1", ctx));
   cppdom::NodePtr element2(new cppdom::Node("Element2", ctx));
   cppdom::NodePtr element3(new cppdom::Node("Element3", ctx));
   cppdom::NodePtr element1_cdata(new cppdom::Node("Element1-cdata", ctx));
   cppdom::NodePtr element3_cdata2(new cppdom::Node("Element3-cdata2", ctx));

   // Document can only have one element as child (to be valid xml)
   // Set this to the root element
   doc.addChild(root);

   // Now add element 1
   // - Also set it's attributes
   root->addChild(element1);
   element1->setAttribute("attrib1", 1);
   element1->setAttribute("attrib2", "two");
   std::string escaped_attrib_text = "<this>&<that>\"\'<done>";
   element1->setAttribute("attrib3", escaped_attrib_text);
   element1->addChild(element1_cdata);

   // Cdata must have it's type set
   // then set the actual contents of the cdata
   element1_cdata->setType(cppdom::xml_nt_cdata);
   std::string escaped_elt1_cdata("This is 'element1'<here>&\"there\"");
   element1_cdata->setCdata(escaped_elt1_cdata);

   // Add a couple of nested nodes and set the attributes
   root->addChild(element2);
   element2->addChild(element3);
   element3->setAttribute("attrib1", "attrib1");

   // Set Cdata a couple of different ways (this is a test isn't it :)
   element3->setCdata("We are element 3 <<clear me>>");
   element3_cdata2->setType(cppdom::xml_nt_cdata);
   element3_cdata2->setCdata("We are still element 3");
   element3->addChild(element3_cdata2);
   element3->setCdata("We are element 3");

   // Get the cdata contents and make sure they match up
   std::string cdata_text;
   cdata_text = element1->getCdata();
   std::cout << escaped_elt1_cdata << cdata_text << std::endl;
   cdata_text = element1->getFullCdata();
   std::cout << escaped_elt1_cdata << cdata_text << std::endl;
   cdata_text = element1_cdata->getCdata();
   std::cout << escaped_elt1_cdata << cdata_text << std::endl;

   cdata_text = element3->getCdata();
   std::cout << "We are element 3: " << cdata_text << std::endl;
   cdata_text = element3->getFullCdata();
   std::cout << "We are element 3,We are still element 3: " << cdata_text << std::endl;

   // Dump the tree to the screen
   testHelpers::dump_node(doc);

   // Write the document out to a file
   std::cout << "------- Indented document ------\n";
   doc.save(std::cout);
   std::cout << "------- No indent, no newline document ------\n";
   doc.save(std::cout, false, false);
   std::cout << std::endl;

   std::string filename("maketree.xml");
   doc.saveFile(filename);

   // Now load it to test some things
   cppdom::Document loaded_doc(ctx);
   loaded_doc.loadFile(filename);

   cppdom::NodePtr r_element1 = loaded_doc.getChild("root")->getChild("Element1");
   
   std::string r_elt1_cdata = r_element1->getCdata();
   while(r_elt1_cdata[r_elt1_cdata.size()-1] == ' ')     // Remove trailing whitespace
   { r_elt1_cdata.erase(r_elt1_cdata.size()-1); }

   /*
   r_elt1_cdata.erase(std::find_if(r_elt1_cdata.rbegin(), r_elt1_cdata.rend(),
                    std::not1(std::ptr_fun(std::isspace))).base(),
                    r_elt1_cdata.end());
                    */

   assert(r_elt1_cdata == escaped_elt1_cdata);

   std::string r_attrib = r_element1->getAttribute("attrib3");
   assert(r_attrib == escaped_attrib_text);

   std::cout << "---------------------------------------\n"
             << "Tests passed." << std::endl;

   return 0;
}
