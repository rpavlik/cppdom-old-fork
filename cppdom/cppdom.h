// this xmlpp was branched from the original LGPL'd xmlpp version 0.6
// this new branched xmlpp is under the same LGPL (of course) and
// is being maintained by:
//    kevin meinert <subatomic@users.sf.net>
//    allen bierbaum <allenb@users.sf.net>
//    ben scott <nonchocoboy@users.sf.net>
/////////////////////////////////////////////////////////////////////
/*
   xmlpp - an xml parser and validator written in C++
   copyright (c) 2000-2001 Michael Fink

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA  02111-1307  USA.

*/
/** @file xmlpp.hpp
*
*  the main declaration header
*/

// prevent multiple includes
#ifndef __xmlpp_hpp_
#define __xmlpp_hpp_

#ifdef _MSC_VER
   // disable 'identifier was truncated to 255 characters in debug information' warning
#  pragma warning(disable: 4786)
#endif

// needed includes
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <iosfwd>
#include <fstream>
#include <xmlpp/xmlconfig.h>
#include <xmlpp/shared_ptr.h>   // the boost::shared_ptr class

//! namespace of the xmlpp project
namespace xmlpp {


/** basic char type */
typedef char xml_char_type;
/** string class typedef */
typedef std::basic_string<xml_char_type> xmlstring;
/** string smart pointer */
typedef xmlpp_boost::shared_ptr<xmlstring> xmlstringptr;


//! xml parsing error codes enumeration
enum xmlerrorcode
{
   xml_unknown = 0,              /**< unspecified or unknown error */
   xml_instream_error,           /**< error in the infile stream */
   xml_opentag_expected,         /**< expected an open tag literal '<' */
   xml_opentag_cdata_expected,   /**< expected a '<' or cdata */
   xml_closetag_expected,        /**< expected a '>' closing tag literal */
   xml_pi_doctype_expected,      /**< expected a processing instruction or doctype tag */
   xml_tagname_expected,         /**< expected a tag name after '<' or '</' */
   xml_closetag_slash_expected,  /**< expected a '/' after closing tag literal '<' */
   xml_tagname_close_mismatch,   /**< tag name from start and end tag mismatch */
   xml_attr_equal_expected,      /**< expected '=' after attribute name */
   xml_attr_value_expected,      /**< expected value after an '=' in attribute */
   xml_save_invalid_nodetype,    /**< invalid nodetype encountered while saving */

// added by kevin for 0.7 compatibility...
   xml_filename_invalid,
   xml_file_access,

   xml_dummy                     /**< dummy error code */
};


// classes

/** xml error class
*  contains an xmlerrorcode and is thrown while parsing xml input
*/
class XMLPP_API xmlerror
{
public:
   /** constructor */
   xmlerror( xmlerrorcode code ){ errorcode = code; }
   /** returns the error code */
   xmlerrorcode get_error() const { return errorcode; }
   /** returns the string representation of the error code */
   void get_strerror(xmlstring &error) const;
   xmlstring get_string() const
   {
      xmlstring err;
      this->get_strerror( err );
      return err;
   }
   /** return additional error info */
   const xmlstring get_info() const { return "unknown error"; }

protected:
   xmlerrorcode errorcode;
};


/** xml stream position
* represents the position in the xml input stream; usable if load()
*    throws an error on parsing xml content
*/
class XMLPP_API xmllocation
{
public:
   /** Constructor */
   xmllocation()
   { reset(); }

   /** returns current line */
   int get_line() const
   { return line; }

   /** returns current position in a line */
   int get_pos() const
   { return pos; }

   /** advances a char */
   void step( int chars = 1 )
   { pos+=chars; }
   /** indicates entering a new line */
   void newline()
   { line++; pos=1; }

   /** reset location */
   void reset()
   { line=pos=1; }

protected:
   int line,pos;
};


// typedefs

/** handle to a tagname string in a tagname map */
typedef int xmltagnamehandle;
/** maps the tagname string to a handle */
typedef std::map<xmltagnamehandle,xmlstring> xmltagnamemap;
/** maps an entity to a string representation */
typedef std::map<xmlstring,xmlstring> xmlentitymap;
/** smart pointer for xmlcontext */
typedef xmlpp_boost::shared_ptr<class xmlcontext> xmlcontextptr;
/** smart pointer to the event handler */
typedef xmlpp_boost::shared_ptr<class xmleventhandler> xmleventhandlerptr;

typedef xmltagnamehandle XMLTagNameHandle;
/** maps the tagname string to a handle */
typedef xmltagnamemap XMLTagNameMap;
/** maps an entity to a string representation */
typedef xmlentitymap XMLEntityMap;
/** smart pointer for xmlcontext */
typedef xmlcontextptr XMLContextPtr;
/** smart pointer to the event handler */
typedef xmleventhandlerptr XMLEventHandlerPtr;


/** xml parsing context class.
* the class is the parsing context for the parsed xml document.
* the class has a tagname lookup table and an entity map
*/
class XMLPP_API xmlcontext
{
public:
   /** ctor */
   xmlcontext();
   /** dtor */
   virtual ~xmlcontext();

   /** returns the entity representation for the named entity */
   xmlstring get_entity( const xmlstring &entname );

   /** returns the tagname by the tagname handle */
   xmlstring get_tagname( xmltagnamehandle handle );

   /** inserts a tag name and returns a tag name handle to the string */
   xmltagnamehandle insert_tagname( const xmlstring &tagname );

   /** returns the current location in the xml stream */
   xmllocation &get_location()
   { return location; }

   /** called once when the context instance starts up; overwrite to customize
   * @note: The base member should always be called, to set init to true
   */
   virtual void init_context()
   { init = true; }

   /** @name event handling methods */
   //@{
   /** sets the event handler; enables handling events */
   void set_eventhandler(xmleventhandlerptr ehptr)
   {
      eventhandler=ehptr;
      handleevents=true;
   }

   /** returns the currently used eventhandler (per reference) */
   xmleventhandler &get_eventhandler()
   { return *eventhandler.get(); }

   /** returns if a valid event handler is set */
   bool handle_events() const
   { return handleevents; }

protected:

   bool init;        /**< indicates if init_context() was already called */
   int nexthandle;   /**< next available tagname handle */
   xmltagnamemap tagnames;    /**< matches xmltagnamehandles to the real xmlstring's */
   xmlentitymap entities;     /**< Contains entity codes and their string representations */
   xmllocation location;      /**< location of the xml input stream */
   bool handleevents;         /**< indicates if the event handler is used */
   xmleventhandlerptr eventhandler;    /**< current parsing event handler */
};

typedef xmlcontext XMLContext;

/** node type enumeration */
enum xmlnodetype
{
   xml_nt_node,      /**< normal node, can contain subnodes */
   xml_nt_leaf,      /**< a leaf node, which contains no further nodes, eg. <img/> */
   xml_nt_document,  /**< document root node */
   xml_nt_cdata      /**< cdata node, which only contains char data */
};


// typedefs
/** smart pointer to node */
typedef xmlpp_boost::shared_ptr<class xmlnode> xmlnodeptr;
/** list of node smart pointer */
typedef std::list<xmlnodeptr> xmlnodelist;
typedef xmlnodelist XMLNodeList;
typedef XMLNodeList::iterator XMLNodeListIterator;
typedef xmlnodeptr XMLNodePtr;



/** XML attribute class.
* Just wraps a string
*/
class XMLPP_API xmlattribute
{
public:
   xmlattribute()
    : mData("")
   {;}

   xmlattribute(const xmlattribute& r)
   {
      mData = r.mData;
   }

   xmlattribute(const std::string& str_val)
   {
      mData = str_val;
   }

   template<class T>
   xmlattribute(const T& val)
   {
      setValue<T>(val);
   }

   xmlstring getString() const
   { return mData; }

   /** Set mData to the string value of val
   * @note Requires a stream operation of type T
   */
   template<class T>
   void setValue(const T& val)
   {
      std::ostringstream oss;
      oss << val;
      mData = oss.str();
   }

   template<class T>
   T getValue() const
   {
      T t;
      std::istringstream iss(mData);
      iss >> t;
      return t;
   }

   /** Autoconversion to string (so old code should work) */
   operator std::string() const
   { return mData; }

protected:
   xmlstring mData;
};


//! xml tag attribute map
/*! contains all attributes and values a tag has, represented in a map */
class XMLPP_API xmlattributes: public std::map<xmlstring, xmlstring>
{
   friend class xmlparser;
public:
   //! ctor
   xmlattributes()
   {}

   /** Get the named attribute
   * @returns empty string "" if not found, else the value
   */
   xmlstring get( const xmlstring &key ) const;

   /** Sets new attribute value
   * If not found, then just insert the new attribute
   */
   void set(const xmlstring &key, const xmlstring &value);

   /** Check if the attribute map has the given attribute
   * @return false if not found
   */
   bool has(const xmlstring& key) const;

};

typedef xmlattributes XMLAttributes;

/** xml node */
class XMLPP_API xmlnode
{
   friend class xmlparser;
protected:
   /** Default Constructor */
   xmlnode()
   { nodetype = xml_nt_node; }
public:
   /** ctor, takes xml context pointer */
   explicit xmlnode( xmlcontextptr pctx )
   { nodetype=xml_nt_node; contextptr=pctx; }
   /** Destructor */
   ~xmlnode(){}
   /** copy constructor */
   xmlnode( const xmlnode &node );
   /** assign operator */
   xmlnode &operator =( const xmlnode &node );

   /** @name access to node info */
   //@{
   /** returns type of node */
   xmlnodetype getType() const
   { return nodetype; }

   

   /** Returns the local name of the node (the element name) */
   xmlstring getName();
   /** returns attribute map of the node */
   xmlattributes &get_attrmap()
   { return attributes; }

   /** Get the named attribute
   * @returns empty string ("") if not found, else the value
   * @post Object does not change.
   */
   xmlattribute getAttribute( const xmlstring& name ) const
   { return attributes.get(name); }

   /** Check if the node has a given attribute */
   bool hasAttribute( const xmlstring& name ) const
   { return attributes.has(name); }

   /** returns cdata string
   * @note: This only returns data for nodes that are leaf nodes of type "cdata".
   *        Calling this on a node that has cdata as a child does nothing
   */
   const xmlstring &get_cdata()
   { return mCdata; }
   //@}

   /** @name node data manipulation */
   //@{
   /** sets new nodetype */
   void set_type( xmlnodetype ntype )
   { nodetype=ntype; }

   /** set the node name */
   void set_name( const xmlstring &nname )
   { setName(nname); }

   void setName( const xmlstring& nname);

   /** sets new cdata */
   void set_cdata( const xmlstring &ncdata )
   { mCdata=ncdata; }

   /** sets new attribute value
   * @post Element.attr is set to value
   */
   void setAttribute( const xmlstring &attr, const xmlattribute& value )
   {
      attributes.set(attr,value.getString());
   }

   void addChild(xmlnodeptr& node)
   {
      mNodelist.push_back(node);
   }

   bool removeChild(xmlnodeptr& node)
   {
      std::cout << "not implemented\n";
      return false;
   }

   bool removeChild(xmlstring& childName)
   {
      std::cout << "not implemented\n";
      return false;
   }

   bool removeChildren(xmlstring& childName)
   {
      std::cout << "not implemented\n";
      return false;
   }

   //@}

   /** @name navigation through the nodes */
   //@{
   
   /** returns a list of the nodes children */
   xmlnodelist& getChildren()
   { return mNodelist; }

   /** Returns the first child of the given local name */
   xmlnodeptr getChild( const xmlstring& name );

   /** Returns a list of all children (one level deep) with local name of childName */
   xmlnodelist getChildren(const xmlstring& name);

   //@}

   /** @name load/save functions */
   //@{
   /** loads xml node from input stream */
   void load( std::istream &instream, xmlcontextptr &ctxptr );
   /** saves node to xml output stream */
   void save( std::ostream &outstream, int indent=0 );
   //@}

   xmlcontextptr getContext()
   { return contextptr; }

protected:

   xmltagnamehandle  nodenamehandle;   /**< handle to the real tag name */
   xmlcontextptr     contextptr;       /**< smart pointer to the context class */
   xmlnodetype       nodetype;         /**< The type of the node */
   xmlattributes     attributes;       /**< Attributes of the element */
   xmlstring         mCdata;           /**< Character data (if there is any) */
   xmlnodelist       mNodelist;         /**< stl list with subnodes */
};


/** xml document */
class XMLPP_API xmldocument: public xmlnode
{
   friend class xmlparser;
public:
   /** constructor */
   xmldocument(){ nodetype = xml_nt_document; }

   /** constructor taking xml context pointer */
   explicit xmldocument( xmlcontextptr pctx )
   {
      nodetype = xml_nt_document;
      contextptr=pctx;
   }

   /** returns a list of processing instruction nodes */
   xmlnodelist &get_pi_list()
   { return procinstructions; }

   /** returns a list of document type definition rules to check the xml file */
   xmlnodelist &get_dtd_list()
   { return dtdrules; }

   /** loads xml Document (node) from input stream */
   void load( std::istream &instream, xmlcontextptr &ctxptr );

   /** saves node to xml output stream */
   void save( std::ostream &outstream );

   void load_file( const std::string& st )
   {
      std::ifstream file_istream;
      file_istream.open( st.c_str(), std::ios::in );
      this->load( file_istream, contextptr );
      file_istream.close();
   }

protected:
   /** node list of parsed processing instructions */
   xmlnodelist procinstructions;
   /** node list of document type definition rules */
   xmlnodelist dtdrules;
};

typedef xmldocument XMLDocument;
typedef xmlpp_boost::shared_ptr<class xmldocument> xmldocumentptr;
typedef xmldocumentptr XMLDocumentPtr;


/** Interface for xml parsing event handler */
class XMLPP_API xmleventhandler
{
public:
   /** ctor */
   xmleventhandler(){}

   /** virtual dtor */
   virtual ~xmleventhandler(){}

   /** called when parsing of an xml document starts */
   virtual void start_document(){}

   /** called when ended parsing a document */
   virtual void end_document(){}

   /** called when parsing a processing instruction */
   virtual void processing_instruction( xmlnode &pinode ){}

   /** called when start parsing a node */
   virtual void start_node( const xmlstring &nodename ){}
   /** called when an attribute list was parsed */
   virtual void parsed_attributes( xmlattributes &attr ){}
   /** called when parsing of a node was finished */
   virtual void end_node( xmlnode &node ){}

   /** called when a cdata section ended */
   virtual void got_cdata( const xmlstring &cdata ){}
};


// namespace end
};

#endif
