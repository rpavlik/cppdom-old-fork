/*
   xmlpp - an xml parser and validator written in C++
   copyright (c) 2000-2001 Michael Fink
   modified by Karl Pitrich
      
   $Id$
*/

/*! \file xmlpp.cpp

  contains the methods of the xmlpp classes

*/

#include <cstdio>
#include <cstdarg>
#include "xmlpp.h"
#include "xmlparser.h"
#include <fstream>

//debug
#include <iostream>

namespace xmlpp {

// internal use for saving
static bool xml_last_was_cdata;

		  
// XMLContext methods

XMLContext::XMLContext() {
	init = false;
	nexthandle = 0;
}

XMLContext::~XMLContext() { }

string XMLContext::get_entity(string &entname) {
	if (!init) init_context();
	xmlentitymap::const_iterator iter = entities.find( entname );
	return ( iter == entities.end() ? entname : iter->second );
}

string XMLContext::get_tagname(xmltagnamehandle handle) {
	if (!init) init_context();
	xmltagnamemap::const_iterator iter = tagnames.find( handle );
	string empty("");
	return ( iter == tagnames.end() ? empty : iter->second );
}

xmltagnamehandle XMLContext::insert_tagname( string &tagname ) {
	if (!init) init_context();
	xmltagnamemap::const_iterator iter;
	iter=tagnames.begin();
	for(;iter!=tagnames.end();iter++) {
		if ((*iter).second == tagname ) return (*iter).first;
	}	

	xmltagnamemap::value_type kp(nexthandle,tagname);
	tagnames.insert(kp);

	return nexthandle++;
}


// XMLAttributes methods

string XMLAttributes::get(string key) {
	XMLAttributes::const_iterator it;

	// try to find the key in the map
	it=find(key);
	string empty("");
	return it==end() ? empty : it->second;
}

void XMLAttributes::set(string key, string value) {
	XMLAttributes::iterator it;

	// try to find the key in the map
	it=find(key);
	if (it != end()) {
		(*it).second = value;
	} else {
		// insert, because the key-value pair was not found
		XMLAttributes::value_type kp(key,value);
		insert(kp);
	}
}

void XMLAttributes::remove(const string &key) {
	XMLAttributes::iterator it;

	it=find(key);
	if (it != end()) erase(key);
}


// XMLNode methods

XMLNode::XMLNode(XMLContextPtr pctx) {
	nodetype=xml_nt_node;
	contextptr=pctx;
}

XMLNode::XMLNode(XMLContextPtr pctx, string nname){
	nodetype=xml_nt_node;
	contextptr=pctx;
	nodenamehandle = contextptr->insert_tagname(nname);
}

XMLNode::XMLNode(const XMLNode &node) {
   nodenamehandle = node.nodenamehandle;
   contextptr = node.contextptr;
   nodetype = node.nodetype;
   attributes = node.attributes;
   cdata = node.cdata;
   nodelist = node.nodelist;
};

XMLNode::~XMLNode() {
	// delete childs
	XMLNodeListIterator it;
	for(it=nodelist.begin(); it!=nodelist.end(); it++) {
		delete (*it);
	}
}

XMLNode &XMLNode::operator =(const XMLNode &n) {
   contextptr = n.contextptr;
   nodelist = n.nodelist;
	nodenamehandle = n.nodenamehandle;
	nodetype = n.nodetype;
	attributes = n.attributes;
   cdata = n.cdata;
   return *this;
};


XMLNode &XMLNode::operator =(const XMLNodePtr n) {
	contextptr = n->contextptr;
	name(n->name());
	type(n->type());
	replace_attr(n->get_attrmap());	
	nodelist=n->nodelist;
	data(n->data());
   return *this;
};


string XMLNode::name(string _name) {
	if(_name.size()) {
		nodenamehandle = contextptr->insert_tagname(_name);	
	} 
	return contextptr->get_tagname(nodenamehandle);
}


xmlnodetype XMLNode::type(xmlnodetype ntype) {
	if(ntype!=xml_nt_dummy) {
		nodetype=ntype;
	}	  
	return nodetype;
}

const string XMLNode::data(string d) {
	if(!d.empty()) {
		cdata=d;
	}
	return cdata;
}


/*! \exception throws xmlpp::xmlerror if requested children was not found */
XMLNodePtr XMLNode::firstchild(const string &n) {
   XMLNodeListConstIterator it;

	// search for child	
   for(it=nodelist.begin(); it!=nodelist.end(); it++) {
      if ((*it)->name() == n) return *it;
   }

   // no valid child found
	throw xmlerror(xml_name_not_found, n);
	return XMLNodePtr(NULL);
}
/*!
	\code
		// usage example:
		const XMLNodeList & XMLNode::children(const string &n) const 
		XMLNodeList L;
		XMLNodeList::iterator It;
		try {
			L=some_node.children("person");
		} catch (xmlerror e) {
			// child "person" could not be found
			cerr << "Error: " << e.get_string() << " (" << e.get_info() << ")" << endl;
			exit(1);
		}
	\endcode	
	\exception throws xmlpp::xmlerror if the requested child was not found
*/

const XMLNodeList & XMLNode::children(const string &n) const {
	if(n.empty()) {
		if(nodelist.empty()) throw xmlerror(xml_childlist_empty);  
		return nodelist;
	}
   
	static XMLNodeList ret;
	ret.clear();
	XMLNodeListConstIterator it;
	
	for(it=nodelist.begin(); it!=nodelist.end(); it++) {	
		if((*it)->name()==n) ret.push_back(*it);
	}
	
	if(ret.empty()) throw xmlerror(xml_name_not_found, n);
   return ret;
}
//! \attention Do not delete the instance n in your program
void XMLNode::add_child(XMLNodePtr n, bool front) {
	if(front) nodelist.push_front(n);
	else nodelist.push_back(n);
}

void XMLNode::add_child(XMLNode &n, bool front) {
	XMLNode *tmp=new XMLNode(n);
	if(front) nodelist.push_front(tmp);
	else nodelist.push_back(tmp);
}

XMLNodePtr XMLNode::add_child(XMLContextPtr ctxp, string n, bool front) {
	XMLNode *tmp=new XMLNode(ctxp, n);
	if(front) nodelist.push_front(tmp);
	else nodelist.push_back(tmp);
	return tmp;
}

void XMLNode::remove_child(XMLNodePtr n) {
	if(n) nodelist.remove(n);
}

//! \exception throws xmlpp::xmlerror when a streaming or parsing error occur
void XMLNode::load( std::istream &instream, XMLContextPtr &ctxptr ) {
   xmlparser parser(instream,ctxptr->get_location());
   parser.parse_node( *this, ctxptr );
}

//! \exception throws xmlpp::xmlerror when a streaming or parsing error occur
//! \todo fix cdata output, esp. because of indentation
void XMLNode::save(std::ostream &outstream, int indent) {
	
	
	// output cdata
	if (nodetype == xml_nt_cdata) {
		outstream << cdata.c_str(); // << std::endl;
		xml_last_was_cdata=true;
		return;
	} else {
		// output indendation spaces
		//if(xml_last_was_cdata) {
		//	xml_last_was_cdata=false;
		//} else {
	  	for(int i=0;i<indent;i++) outstream << ' ';
		//}
	}

	// output tag name
	outstream << '<' << contextptr->get_tagname(nodenamehandle).c_str();
	
	// output all attributes
	XMLAttributes::const_iterator it;
	for(it=attributes.begin(); it!=attributes.end();it++) {
		XMLAttributes::value_type attr = *it;
		outstream << ' ' << attr.first.c_str() << '='
					 << '\"' << attr.second.c_str() << '\"';
	}
	
	// safety for empty nodes	
	if(nodetype==xml_nt_node && nodelist.empty()) {
		nodetype=xml_nt_leaf;
	} 
	
	// depending on the nodetype, do output
	switch(nodetype) {
		case xml_nt_node:
			{
				XMLNodeListIterator it;
				bool makecr=false;
				
				outstream << '>';
			
				// output std::endl only if we dont have cdata childs	
				if((*nodelist.begin())->type()!=xml_nt_cdata) makecr=true;
				if(makecr) outstream << std::endl;
				
				// output all subnodes
				for(it=nodelist.begin(); it!=nodelist.end(); it++) {
					(*it)->save(outstream,indent+1);
				}
         
				// output indendation spaces
				if(xml_last_was_cdata) {
					xml_last_was_cdata=false;
				} else {
					for(int i=0;i<indent;i++) outstream << ' ';
				}
	
				// output closing tag
				outstream << '<' << '/'
							 << contextptr->get_tagname(nodenamehandle).c_str() 
							 << '>' << std::endl;
			}
			break;
		case xml_nt_leaf:
			// a leaf has no subnodes
			outstream << '/' << '>' << std::endl;
			break;
		default:
			// unknown nodetype
			throw xmlerror( xml_save_invalid_nodetype );
	}
}


// XMLDocument methods

XMLDocument::XMLDocument() {
	nodetype = xml_nt_document;
}

XMLDocument::XMLDocument(XMLContextPtr pctx) {
	nodetype = xml_nt_document; contextptr=pctx;
}

XMLDocument::~XMLDocument() {
	XMLNodeListIterator it;

	for(it=procinstructions.begin(); it!=procinstructions.end(); it++) {
		delete (*it);
	}

	for(it=dtdrules.begin(); it!=dtdrules.end(); it++) {
		delete (*it);
	}
}

string XMLDocument::filename(string name) {
	if(name.size()) {
		filename_=name;
	}
	return filename_;
}

/*! \exception throws xmlpp::xmlerror on filename or file access error */
void XMLDocument::save_file(string f) {
	string outfile;
	
	if(f.size()<1) {
		if(filename_.size()<1) {
			// we do not have a valid filename to use
			throw xmlerror(xml_filename_invalid);
		} else {
			outfile=filename_;
		}
 	} else {
		outfile=f;
	}
			
	ofstream outstr(outfile.c_str());
	if (outstr.is_open()) {
		save(outstr);
		filename_=outfile;
	} else {
		// file access error	  
		throw xmlerror(xml_file_access, outfile);	
	}
}

/*! \exception throws xmlpp::xmlerror on filename or file access error */
void XMLDocument::load_file(string f) {
	string infile;

	if(f.size()<1) {
		if(filename_.size()<1) {
			// we do not have a valid filename to use
			throw xmlerror(xml_filename_invalid);
		} else {
			infile=filename_;
		}
	} else {
		infile=f;
	}
			
	ifstream istr(infile.c_str());
	if(istr.is_open()) {	
		load(istr, contextptr);
		filename_=infile;
	} else {
		// file access error	  
		throw xmlerror(xml_file_access, infile);	
	}
}

/*! \exception throws xmlpp::xmlerror when a streaming or parsing error occur */
void XMLDocument::load( std::istream &instream, XMLContextPtr &ctxptr ) {
   xmlparser parser(instream,ctxptr->get_location());
   parser.parse_document( *this, ctxptr );
}

void XMLDocument::add_pi(XMLNodePtr n) {
	procinstructions.push_back(n);
}

void XMLDocument::add_pi(XMLNode &n) {
	XMLNode *tmp=new XMLNode(n);
	procinstructions.push_back(tmp);
}


/*! \exception throws xmlpp::xmlerror when a streaming or parsing error occur */
void XMLDocument::save( std::ostream &outstream ) {
	// output all processing instructions
	XMLNodeListConstIterator it;

   for(it=procinstructions.begin(); it!=procinstructions.end(); it++) {
      XMLNodePtr np=*it;
      
		// output pi tag
		outstream << "<?" << np->name().c_str();

		// output all attributes
		XMLAttributes pi_attr=(*it)->get_attrmap();
		XMLAttributes::const_iterator aIt;
		for(aIt=pi_attr.begin(); aIt!=pi_attr.end(); aIt++) {
			XMLAttributes::value_type attr = *aIt;
			outstream << ' ' << attr.first.c_str() << '='
						 << '\"' << attr.second.c_str() << '\"';
		}
		
		// output closing brace      
		outstream << "?>" << std::endl;
	}

	// output <!doctype> tag
	//for(it=dtdrules.begin(); it!=dtdrules.end(); it++) {
	//	XMLNodePtr np=*it;
	//	cerr << "debug doctype: " << np->name().c_str() << endl;	
		//outstream << "<!" << np->name().c_str();
		// output closing brace
		//outstream << ">" << std::endl;	
	//}
			  
	// call save() method of the root node in XMLDocument
	(*nodelist.begin())->save(outstream,0);   
}



// macro for keeping the errorcode switch short and easy
#define XMLERRORCODE(x,y)  case x: err = y; break;

// xmlerror methods
const string xmlerror::get_string() const {
   const char *err;
   switch(errorcode) {
      XMLERRORCODE(xml_unknown,"unspecified or unknown error");
      XMLERRORCODE(xml_instream_error,"error in the infile stream");
      XMLERRORCODE(xml_opentag_expected,"expected an open tag literal '<'");
      XMLERRORCODE(xml_opentag_cdata_expected,"expected a '<' or cdata");
      XMLERRORCODE(xml_closetag_expected,"expected an '>' closing tag literal");
      XMLERRORCODE(xml_pi_doctype_expected,"expected a processing instruction or doctype tag");
      XMLERRORCODE(xml_tagname_expected,"expected a tag name after '<' or '</'");
      XMLERRORCODE(xml_closetag_slash_expected,"expected a '/' after closing tag literal '<'");
      XMLERRORCODE(xml_tagname_close_mismatch,"tag name from start and end tag mismatch");
      XMLERRORCODE(xml_attr_equal_expected,"expected '=' after attribute name");
      XMLERRORCODE(xml_attr_value_expected,"expected value after 'a' in attribute");
      XMLERRORCODE(xml_save_invalid_nodetype,"invalid nodetype encountered while saving");
      XMLERRORCODE(xml_name_not_found,"name not found");
      XMLERRORCODE(xml_childlist_empty,"node has no childs");
		XMLERRORCODE(xml_dummy,"dummy error code (this error should never been seen)");
      XMLERRORCODE(xml_filename_invalid,"no valid filename provided");
      XMLERRORCODE(xml_file_access,"file could not be accessed");
   }
	return(err);
}

void xmlerror::show_error(XMLContextPtr c) {
	xmllocation where(c->get_location());
	cerr << "Error at line " << where.get_line();
	cerr << ", position " << where.get_pos() << ": ";
	cerr << get_string() << endl;
}

void xmlerror::show_line(XMLContextPtr c, string filename) {
	xmllocation where(c->get_location());
	ifstream errfile(filename.c_str());
	
	int linenr = where.get_line();
	char linebuffer[1024];
	
	for(int i=0;i<linenr&&!errfile.eof();i++)
		errfile.getline(linebuffer,1024);
	
	int pos = where.get_pos();
	if (pos>=80) pos%=80;
	
	string line(linebuffer + (where.get_pos()-pos));
	
	if (line.length()>=79)
		line.erase(79);
	cout << line.c_str() << endl;
	for(int j=2;j<pos;j++)
		cout << ' ';
	cout << '^' << endl;
} 


};// namespace end

/* vi: set ts=3: */
