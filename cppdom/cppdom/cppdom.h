/*
   xmlpp - an xml parser and validator written in C++
   (c) 2000 Michael Fink
   (c) 2001 Karl Pitrich
      
   $Id$
*/

/*! \file xmlpp.hpp

  the main declaration header

*/

#ifndef __xmlpp_h__
#define __xmlpp_h__

#include <string>
#include <list>
#include <map>
#include <iosfwd>
#include "xmlcommon.h"
#include "xmlhelpers.h"

//! namespace of the xmlpp project
namespace xmlpp {

//! pointer for XMLContext
typedef class XMLContext* XMLContextPtr;


//! xml error class
/*! contains an xmlerrorcode and is thrown while parsing xml input */
class XMLPP_API xmlerror {
public:
   //! constructor
   xmlerror( xmlerrorcode code, string info="" ) { errorcode = code; errorinfo=info; }
   //! returns the error code
   xmlerrorcode get_error() const { return errorcode; }
   //! returns the string representation of the error code
   const string get_string() const;
   //! return additional error info
	const string get_info() const { return errorinfo; }
	//! display a parse error in human readable form on stderr
	void show_error(XMLContextPtr c);
	//! show line in xmlfile where error occured on stderr
	void show_line(XMLContextPtr c, string filename);
protected:
   xmlerrorcode errorcode;
	string errorinfo;	
};


//! xml parsing context class
/*! the class is the parsing context for the parsed xml document.
    the class has a tagname lookup table and an entity map */
class XMLPP_API XMLContext {
public:
   //! ctor
   XMLContext();
   //! dtor
   virtual ~XMLContext();
   //! returns the entity representation for the named entity
   string get_entity( string &entname );
   //! returns the tagname by the tagname handle
   string get_tagname( xmltagnamehandle handle );
   //! inserts a tag name and returns a tag name handle to the string
   xmltagnamehandle insert_tagname( string &tagname );
   //! returns the current location in the xml stream
   xmllocation &get_location() { return location; }

   //! called once when the context instance starts up; overwrite to customize
   /*! \note the base member should always be called to set init to true; */
   virtual void init_context() { init = true; }

protected:
   //! indicates if init_context() was already called
   bool init;
   //! next available tagname handle
   int nexthandle;
   //! the tagname map matches xmltagnamehandles to the real xmlstring's
   xmltagnamemap tagnames;
   //! the entity map contains entity codes and their string representations
   xmlentitymap entities;
   //! location of the xml input stream
   xmllocation location;
};


//! pointer to node
typedef class XMLNode* XMLNodePtr;
//! list of node smart pointer
typedef std::list<XMLNodePtr> XMLNodeList;
//! iterator for XMLNodeList
typedef XMLNodeList::iterator XMLNodeListIterator;
//! const iterator for XMLNodeList
typedef XMLNodeList::const_iterator XMLNodeListConstIterator;


//! xml tag attribute map
/*! contains all attributes and values a tag has, represented in a map */
class XMLPP_API XMLAttributes: public std::map<string, string> {
friend class xmlparser;

public:
   //! ctor
   XMLAttributes(){}
   //! returns attribute value by name
   string get(string key);
   //! sets new attribute value
   void set(string key, string value);
	//! removes attribute by name
	void remove(const string &key);
};


//! xml node
class XMLPP_API XMLNode {
friend class xmlparser;

public:
   //! ctor, takes xml context pointer
   explicit XMLNode(XMLContextPtr pctx);
   //! ctor, takes xml context pointer and the node name as string
   explicit XMLNode(XMLContextPtr pctx, string nname);
   //! dtor
   ~XMLNode();
   //! copy constructor
   XMLNode(const XMLNode &node);
   //! assign operator with reference
   XMLNode & operator =(const XMLNode &node);
   //! assign operator with pointer
   XMLNode & operator =(const XMLNodePtr n);
	
	//! returns or sets the node name
	string XMLNode::name(string _name="");
  	//! returns or sets the type of the node	
	xmlnodetype type(xmlnodetype ntype=xml_nt_dummy);
	//! ireturns or sets cdata string
   const string data(string d="");

   //! returns attribute map of the node
   XMLAttributes &get_attrmap() { return attributes; }
   //! returns attribute value for given attribute
   string get_attr(string attr) { return attributes.get(attr); }
	//! sets new attribute value
   void add_attr(string attr, string value){ attributes.set(attr,value); } 
	//! removes attribute 'attr'
	void remove_attr(string attr) {attributes.remove(attr); }
	//! replaces current attributes by 'attrmap'
	void replace_attr(XMLAttributes &attrmap) { attributes=attrmap; }
		
	//! add a child node from a pointer to a node
	//! \param front If set to true, the node will be added at the top of the list,
	//!        otherwise at the end. 
	void add_child(XMLNodePtr n, bool front=false);
	//! add a child node from a reference to a local XMLNode
	//! \param front If set to true, the node will be added at the top of the list,
	//!        otherwise at the end. 
	void add_child(XMLNode &n, bool front=false);
	//! add a new node named 'n' as child and return a ptr to it
	//! \param front If set to true, the node will be added at the top of the list,
	//!        otherwise at the end. 
	XMLNodePtr add_child(XMLContextPtr ctxp, string n, bool front=false);
	//! removes the child pointed to by 'n' 
	void remove_child(XMLNodePtr n);
   //! select some nodes and put it into a separate nodelist
	const XMLNodeList & children(const string & = string()) const;
	//! returns the first child with the given name
   XMLNodePtr firstchild(const string &n);

   //! loads xml node from input stream
   void load(std::istream &instream, XMLContextPtr &ctxptr);
   //! saves node to xml output stream
   void save(std::ostream &outstream, int indent=0);
	
protected:
   //! ctor
   XMLNode() { nodetype = xml_nt_node; }
   //! handle to the real tag name
   xmltagnamehandle nodenamehandle;
   //! smart pointer to the context class
   XMLContextPtr contextptr;
   //! nodetype
   xmlnodetype nodetype;
   //! attributes of the tag
   XMLAttributes attributes;
   //! char data
   string cdata;
   //! stl list with subnodes
   XMLNodeList nodelist;
};


//! xml document
class XMLPP_API XMLDocument: public XMLNode {
friend class xmlparser;

public:
   //! ctor
   XMLDocument();
   //! ctor, takes xml context pointer
   explicit XMLDocument(XMLContextPtr pctx);
	//! dtor
	~XMLDocument();

	
   //! returns a list of processing instruction nodes
   XMLNodeList &get_pi_list(){ return procinstructions; }
   //! returns a list of document type definition rules to check the xml file
   XMLNodeList &get_dtd_list(){ return dtdrules; }
	//! add a process instruction node
	void add_pi(XMLNodePtr n);
	//! add a process instruction node from a reference to a local XMLNode
	void add_pi(XMLNode &n);

   //! loads xml node from input stream
   void load( std::istream &instream, XMLContextPtr &ctxptr );
   //! saves node to xml output stream
   void save( std::ostream &outstream );
	
	//! load xml node from file, sets filename for this XMLDocument	
	void load_file(string filename="");
	//! saves node to file, sets filename for this XMLDocument
	void save_file(string filename="");
	//! get or set filename
	string filename(string name="");
	
protected:
   //! node list of parsed processing instructions
   XMLNodeList procinstructions;
   //! node list of document type definition rules
   XMLNodeList dtdrules;
	//! filename
	string filename_;
};


// namespace end
};

#endif
/* vi: set ts=3: */
