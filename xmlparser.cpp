/*
   xmlpp - an xml parser and validator written in C++
   (c) 2000 Michael Fink
   (c) 2001 Karl Pitrich
      
   $Id$
*/

/*! \file xmlparser.cpp

  member functions for the tokenizer and parser classes

*/

#include "xmlparser.h"

namespace xmlpp {

xmlparser::xmlparser( std::istream &inputstream, xmllocation &loc )
:instream(inputstream), tokenizer(inputstream,loc)
{ }

bool xmlparser::parse_document(XMLDocument &doc, XMLContextPtr &ctxptr) {
   // set root nodename
   doc.contextptr = ctxptr;
   string rootstr("root");
   doc.nodenamehandle = ctxptr->insert_tagname(rootstr);

   parse_header(doc, ctxptr);

   // parse the only one subnode
	XMLNodePtr subnode=new XMLNode(ctxptr);
	
   bool ret = parse_node(*subnode, ctxptr);

   // if successful, put node into nodelist
   if (ret) doc.add_child(subnode);

   return ret;
}

// parses the header, ie processing instructions and doctype tag
//! \todo parse <!doctype> tag
bool xmlparser::parse_header(XMLDocument &doc, XMLContextPtr &ctxptr) {
	while(1) {
		tokenizer++;
		xmltoken token1 = *tokenizer;
		if (token1 != '<')
			throw xmlerror( xml_opentag_expected );

		// token after opening < is a literal?
		tokenizer++;
		xmltoken token2 = *tokenizer;
		if (!token2.is_literal()) {
			// generic string encountered: assume no pi and doctype tags
			tokenizer.put_back();
			tokenizer.put_back(token1);
			return false;
		}

		// now check for the literal
		switch(token2.get_literal()) {
			// comment or doctype tag
	      case '!':
			{
				tokenizer++;
				xmltoken token3 = *tokenizer;

				if (!token3.is_literal()) {
					// now a doctype tag or a comment may follow
					if (token3.get_generic().at(0) == '-' &&
						 token3.get_generic().at(1) == '-') {
						parse_comment(ctxptr);
					} else {
						string doctypestr(token3.get_generic());
						std::transform(doctypestr.begin(),
											doctypestr.end(),
											doctypestr.begin(),toupper);

						if (doctypestr == "DOCTYPE") {
							// \todo parse doctype tag
							// read the complete tag till the closing >
							while (*(tokenizer++) != '>');
							
						} else {
							throw xmlerror( xml_unknown );
						}
					}
				} else {
					throw xmlerror( xml_pi_doctype_expected );
				}
				break;
			}
		case '?':
		{
			tokenizer++;
			xmltoken token3 = *tokenizer;

			if (token3.is_literal())
				throw xmlerror( xml_pi_doctype_expected );

			// parse processing instruction
			XMLNodePtr pinode=new XMLNode(ctxptr);
			pinode->type(xml_nt_dummy);
			string tagname( token3.get_generic() );
			pinode->name(tagname);
			parse_attributes(pinode->attributes);
			doc.add_pi(pinode);	
				
			tokenizer++;
			if (*tokenizer != '?')
				throw xmlerror( xml_pi_doctype_expected );

			tokenizer++;
				if (*tokenizer != '>')
					throw xmlerror( xml_closetag_expected );
			break;
		}
		default:
			// unknown literal encountered
			throw xmlerror( xml_pi_doctype_expected );
		} // end switch
	} // end while
}

// parses the contents of the current node
bool xmlparser::parse_node( XMLNode &node, XMLContextPtr &ctxptr ) {
	node.contextptr = ctxptr;
	tokenizer++;
	xmltoken token1 = *tokenizer;
	xmltoken token2;

	if (token1.is_endofstream())
		return false;

	// loop when we encounter a comment
	bool again;
	do {
		again = false;

		// check if we have cdata
		if (!token1.is_literal()) {
			string cdataname("cdata");
			node.nodenamehandle = ctxptr->insert_tagname( cdataname );

			// parse cdata section(s) and return
			node.nodetype = xml_nt_cdata;
			node.cdata.empty();
			while(!token1.is_literal()) {
				node.cdata += token1.get_generic();
				tokenizer++;
				token1 = *tokenizer;
			}
			// strip parsed whitespace after cdata
			/*
			int remove=0;
			string::reverse_iterator sIt=node.cdata.rbegin();
			while(sIt!=node.cdata.rend()) {
	  		  	if((*sIt)!=' ') break;
				else remove++;
				sIt++;
			}
			if(remove>0)
				node.cdata.erase(node.cdata.size()-remove,remove);
			*/
			tokenizer.put_back();
			return true;
		}

		// no cdata, try to continue parsing node content

		if (token1 != '<')
			throw xmlerror(xml_opentag_cdata_expected);

		// get node name
		tokenizer++;
		token2 = *tokenizer;
		if (token2.is_literal()) {
			// check the following literal
			switch(token2.get_literal()) {
				// closing '</...>' follows
				case '/':
					// return, we have a closing node with no more content
					tokenizer.put_back();
					tokenizer.put_back( token1 );
					return false;
					
				// comment follows
				case '!':
					parse_comment(ctxptr);
					// get next token
					tokenizer++;
					token1 = *tokenizer;
					// parse again, until we encounter some useful data
					again = true;
				break;
				
				default:
					throw xmlerror(xml_tagname_expected);
			}
		}
	} while (again);

	// insert tag name and set handle for it
	string tagname(token2.get_generic());
	node.nodenamehandle = ctxptr->insert_tagname(tagname);

	// parse attributes
	parse_attributes(node.attributes);

	// check for leaf
	tokenizer++;
	xmltoken token3 = *tokenizer;
	if (token3 == '/' ) {
		// node has finished
		tokenizer++;
		xmltoken token4 = *tokenizer;
		if (token4 != '>' )
			throw xmlerror(xml_closetag_expected);
		node.nodetype = xml_nt_leaf;
		// return, let the caller continue to parse
		return true;
	}

	// now a closing bracket must follow
	if (token3 != '>')
		throw xmlerror(xml_closetag_expected);
	
	// loop to parse all subnodes
	while (1) {
		// create subnode
		XMLNodePtr subnode=new XMLNode(ctxptr);
		// try to parse possible sub nodes
		if (parse_node(*subnode, ctxptr)) {
			// if successful, put node into nodelist
			node.add_child(subnode);
		} else {
			break;
		}
	}

	// parse end tag
	xmltoken token5 = *tokenizer++;
	tokenizer++;
	if (token5 != '<' && *tokenizer != '/')
		throw xmlerror(xml_opentag_expected);

	tokenizer++;
	token1 = *tokenizer;
	if (token1.is_literal())
		throw xmlerror(xml_tagname_expected);

	// check if open and close tag names are identical
	if (token1.get_generic() != token2.get_generic() )
		throw xmlerror(xml_tagname_close_mismatch);

	tokenizer++;
	if (*tokenizer != '>')
		throw xmlerror(xml_opentag_expected);

	return true;
}

// parses tag attributes
bool xmlparser::parse_attributes( XMLAttributes &attr ) {
   while(1) {
      tokenizer++;
      xmltoken token1 = *tokenizer;

      if (token1.is_literal()) {
         tokenizer.put_back();
         return false;
      }

      // get value name here
      string name = token1.get_generic();

      tokenizer++;
      if (*tokenizer != '=')
         throw xmlerror(xml_attr_equal_expected);

      tokenizer++;
      xmltoken token2 = *tokenizer;

      if (token2.is_literal())
         throw xmlerror(xml_attr_value_expected);

      // remove "" from attribute value
      string value( token2.get_generic() );
      value.erase(0,1);
      value.erase(value.length()-1,1);
		
      // insert attribute into the map
      XMLAttributes::value_type attrpair(name, value);
      attr.insert(attrpair);
   }
   return true;
}

void xmlparser::parse_comment( XMLContextPtr &ctxptr ) {

/*		  
	string comment("");
	// safety for different method calls
	if(*tokenizer=='!') tokenizer++;
	if(*tokenizer=="--") tokenizer++;

	//tokenizer.set_cdata_mode(true);
	while(1) {
		if(*tokenizer=="--") {
			tokenizer++;
			if(*tokenizer=='>') {
				cerr << "TEST: " << comment << endl;
				break;
			} else {
				throw xmlerror(xml_closetag_expected);
				break;
			}
		}			  
		xmltoken t1 = *tokenizer;
		string part(t1.get_generic());	
		comment+=part;
		tokenizer++;
	}
	//tokenizer.set_cdata_mode(false);
*/	
// original code	
	while (1) {
      if (*tokenizer == "--") {
         tokenizer++;
			if (*tokenizer == '>') {
				break;
			} else {
				throw xmlerror(xml_closetag_expected);
				break;
			}
		}
      tokenizer++;
   }
	
}

}; // namespace end
/* vi: set ts=3: */
