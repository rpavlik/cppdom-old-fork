// cppdom was branched from the original LGPL'd xmlpp version 0.6
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
/** @file cppdom.h
*
*  the main declaration header
*/

// prevent multiple includes
#ifndef CPPDOM_API_INCLUDED
#define CPPDOM_API_INCLUDED

#ifdef _MSC_VER
   // disable 'identifier was truncated to 255 characters in debug information' warning
#  pragma warning(disable: 4786)
#  define XMLPP_NO_MEMBER_TEMPLATES
#endif

// needed includes
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <iosfwd>
#include <fstream>
#include <iostream>
#include <cppdom/config.h>
#include <cppdom/shared_ptr.h>   // the boost::shared_ptr class

//! namespace of the cppdom project
namespace cppdom {


/** basic char type */
typedef char xml_char_type;
/** string class typedef */
typedef std::basic_string<xml_char_type> XMLString;
/** string smart pointer */
typedef cppdom_boost::shared_ptr<XMLString> XMLStringPtr;


//! xml parsing error codes enumeration
enum XMLErrorCode
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
*  contains an XMLErrorCode and is thrown while parsing xml input
*/
class CPPDOM_API XMLError
{
public:
   /** constructor */
   XMLError( XMLErrorCode code ){ errorcode = code; }
   /** returns the error code */
   XMLErrorCode get_error() const { return errorcode; }
   /** returns the string representation of the error code */
   void get_strerror(XMLString &error) const;
   XMLString get_string() const
   {
      XMLString err;
      this->get_strerror( err );
      return err;
   }
   /** return additional error info */
   const XMLString get_info() const { return "unknown error"; }

protected:
   XMLErrorCode errorcode;
};


/** xml stream position
* represents the position in the xml input stream; usable if load()
*    throws an error on parsing xml content
*/
class CPPDOM_API XMLLocation
{
public:
   /** Constructor */
   XMLLocation()
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
typedef int XMLTagNameHandle;
/** maps the tagname string to a handle */
typedef std::map<XMLTagNameHandle,XMLString> XMLTagNameMap;
/** maps an entity to a string representation */
typedef std::map<XMLString,XMLString> XMLEntityMap;
/** smart pointer for XMLContext */
typedef cppdom_boost::shared_ptr<class XMLContext> XMLContextPtr;
/** smart pointer to the event handler */
typedef cppdom_boost::shared_ptr<class XMLEventHandler> XMLEventHandlerPtr;

/** xml parsing context class.
* the class is the parsing context for the parsed xml document.
* the class has a tagname lookup table and an entity map
*/
class CPPDOM_API XMLContext
{
public:
   /** ctor */
   XMLContext();
   /** dtor */
   virtual ~XMLContext();

   /** returns the entity representation for the named entity */
   XMLString get_entity( const XMLString &entname );

   /** returns the tagname by the tagname handle */
   XMLString get_tagname( XMLTagNameHandle handle );

   /** inserts a tag name and returns a tag name handle to the string */
   XMLTagNameHandle insert_tagname( const XMLString &tagname );

   /** returns the current location in the xml stream */
   XMLLocation &get_location()
   { return location; }

   /** called once when the context instance starts up; overwrite to customize
   * @note: The base member should always be called, to set init to true
   */
   virtual void init_context()
   { init = true; }

   /** @name event handling methods */
   //@{
   /** sets the event handler; enables handling events */
   void set_eventhandler(XMLEventHandlerPtr ehptr)
   {
      eventhandler=ehptr;
      handleevents=true;
   }

   /** returns the currently used eventhandler (per reference) */
   XMLEventHandler &get_eventhandler()
   { return *eventhandler.get(); }

   /** returns if a valid event handler is set */
   bool handle_events() const
   { return handleevents; }

protected:

   bool           init;                /**< indicates if init_context() was already called */
   int            nexthandle;          /**< next available tagname handle */
   XMLTagNameMap  tagnames;            /**< matches XMLTagNameHandles to the real XMLString's */
   XMLEntityMap   entities;            /**< Contains entity codes and their string representations */
   XMLLocation    location;            /**< location of the xml input stream */
   bool           handleevents;        /**< indicates if the event handler is used */
   XMLEventHandlerPtr eventhandler;    /**< current parsing event handler */
};

/** node type enumeration */
enum XMLNodetype
{
   xml_nt_node,      /**< normal node, can contain subnodes */
   xml_nt_leaf,      /**< a leaf node, which contains no further nodes, eg. <img/> */
   xml_nt_document,  /**< document root node */
   xml_nt_cdata      /**< cdata node, which only contains char data */
};


// typedefs
/** smart pointer to node */
typedef cppdom_boost::shared_ptr<class XMLNode> XMLNodePtr;
/** list of node smart pointer */
typedef std::list<XMLNodePtr> XMLNodeList;
typedef XMLNodeList::iterator XMLNodeListIterator;



/** XML attribute class.
* Just wraps a string (this is really just and attribute VALUE)
*/
class CPPDOM_API XMLAttribute
{
public:
   XMLAttribute()
    : mData("")
   {;}

   XMLAttribute(const XMLAttribute& r)
   {
      mData = r.mData;
   }

   XMLAttribute(const std::string& str_val)
   {
      mData = str_val;
   }

#ifndef XMLPP_NO_MEMBER_TEMPLATES
   template<class T>
   XMLAttribute(const T& val)
   {
      setValue<T>(val);
   }
#endif // ! XMLPP_NO_MEMBER_TEMPLATES

   XMLString getString() const
   { return mData; }

#ifndef XMLPP_NO_MEMBER_TEMPLATES
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
#endif // ! XMLPP_NO_MEMBER_TEMPLATES

   /** Autoconversion to string (so old code should work) */
   operator std::string() const
   { return mData; }

protected:
   XMLString mData;
};


//! xml tag attribute map
/*! contains all attributes and values a tag has, represented in a map */
class CPPDOM_API XMLAttributes: public std::map<XMLString, XMLString>
{
   friend class XMLParser;
public:
   //! ctor
   XMLAttributes()
   {}

   /** Get the named attribute
   * @returns empty string "" if not found, else the value
   */
   XMLString get( const XMLString &key ) const;

   /** Sets new attribute value
   * If not found, then just insert the new attribute
   */
   void set(const XMLString &key, const XMLString &value);

   /** Check if the attribute map has the given attribute
   * @return false if not found
   */
   bool has(const XMLString& key) const;

};




/** xml node
* A node has the following properties
* name - The element name of the node
* type - The type of the node. see XMLNodetype
* children - Child elements of the node
* cdata - the cdata content if of type cdata
*
*/
class CPPDOM_API XMLNode
{
   friend class XMLParser;
protected:
   /** Default Constructor */
   XMLNode()
   { nodetype = xml_nt_node; }
public:
   /** ctor, takes xml context pointer */
   explicit XMLNode( XMLContextPtr pctx )
   { nodetype=xml_nt_node; contextptr=pctx; }
   /** Destructor */
   ~XMLNode(){}
   /** copy constructor */
   XMLNode( const XMLNode &node );
   /** assign operator */
   XMLNode &operator =( const XMLNode &node );

   /** @name access to node info */
   //@{
   /** returns type of node */
   XMLNodetype getType() const
   { return nodetype; }

   /** Returns the local name of the node (the element name) */
   XMLString getName();
   /** returns attribute map of the node */
   XMLAttributes &get_attrmap()
   { return attributes; }

   /** Get the named attribute
   * @returns empty string ("") if not found, else the value
   * @post Object does not change.
   */
   XMLAttribute getAttribute( const XMLString& name ) const
   { return attributes.get(name); }

   /** Check if the node has a given attribute */
   bool hasAttribute( const XMLString& name ) const
   { return attributes.has(name); }

   /** returns cdata string
   * @note: This only returns data for nodes that are leaf nodes of type "cdata".
   *        Calling this on a node that has cdata as a child does nothing
   */
   const XMLString &get_cdata()
   { return mCdata; }
   //@}

   /** @name node data manipulation */
   //@{
   /** sets new nodetype */
   void set_type( XMLNodetype ntype )
   { nodetype=ntype; }

   /** set the node name */
   void set_name( const XMLString &nname )
   { setName(nname); }

   void setName( const XMLString& nname);

   /** sets new cdata */
   void set_cdata( const XMLString &ncdata )
   { mCdata=ncdata; }

   /** sets new attribute value
   * @post Element.attr is set to value.  If it didn't exist before, now it does.
   */
   void setAttribute( const XMLString &attr, const XMLAttribute& value )
   {
      attributes.set(attr,value.getString());
   }

   void addChild(XMLNodePtr& node)
   {
      mNodelist.push_back(node);
   }

   bool removeChild(XMLNodePtr& node)
   {
      std::cout << "not implemented\n";
      return false;
   }

   bool removeChild(XMLString& childName)
   {
      std::cout << "not implemented\n";
      return false;
   }

   bool removeChildren(XMLString& childName)
   {
      std::cout << "not implemented\n";
      return false;
   }

   //@}

   /** @name navigation through the nodes */
   //@{

   /** returns a list of the nodes children */
   XMLNodeList& getChildren()
   { return mNodelist; }

   /** Returns the first child of the given local name */
   XMLNodePtr getChild( const XMLString& name );

   /** Returns a list of all children (one level deep) with local name of childName
   * \note currently no path-like childname can be passed, like in e.g. msxml
   * If has standard compose() functions, could impl this by calling getChildren(pred)
   */
   XMLNodeList getChildren(const XMLString& name)
   {
      XMLNodeList ret_nlist(0);
      XMLNodeList::const_iterator iter;

      // search for all occurances of nodename and insert them into the new list
      for(iter = mNodelist.begin(); iter != mNodelist.end(); ++iter)
      {
         if ((*iter)->getName() == name)
         { ret_nlist.push_back(*iter); }
      }

      return ret_nlist;
   }

   /** Returns list of all children. 
   * @see getChildren
   * @note Needed because of predicate template matching char*
   */
   XMLNodeList getChildren(const char* name)
   {
      return getChildren( XMLString(name) );
   }

   /** Return a list of children that pass the given STL predicate */
   template<class Predicate>
   XMLNodeList getChildrenPred(Predicate pred)
   {
      XMLNodeList ret_nlist(0);
      XMLNodeList::const_iterator iter;

      // search for all occurances of nodename and insert them into the new list
      for(iter = mNodelist.begin(); iter != mNodelist.end(); ++iter)
      {
         if(pred(*iter))
         { ret_nlist.push_back(*iter); }
      }
      return ret_nlist;
   }
   
   //@}

   /** @name load/save functions */
   //@{
   /** loads xml node from input stream */
   void load( std::istream &instream, XMLContextPtr &ctxptr );
   /** saves node to xml output stream */
   void save( std::ostream &outstream, int indent=0 );
   //@}

   XMLContextPtr getContext()
   { return contextptr; }

protected:

   XMLTagNameHandle  nodenamehandle;   /**< handle to the real tag name */
   XMLContextPtr     contextptr;       /**< smart pointer to the context class */
   XMLNodetype       nodetype;         /**< The type of the node */
   XMLAttributes     attributes;       /**< Attributes of the element */
   XMLString         mCdata;           /**< Character data (if there is any) */
   XMLNodeList       mNodelist;         /**< stl list with subnodes */
};


/** xml document */
class CPPDOM_API XMLDocument: public XMLNode
{
   friend class XMLParser;
public:
   /** constructor */
   XMLDocument(){ nodetype = xml_nt_document; }

   /** constructor taking xml context pointer */
   explicit XMLDocument( XMLContextPtr pctx )
   {
      nodetype = xml_nt_document;
      contextptr=pctx;
   }

   /** returns a list of processing instruction nodes */
   XMLNodeList &get_pi_list()
   { return procinstructions; }

   /** returns a list of document type definition rules to check the xml file */
   XMLNodeList &get_dtd_list()
   { return dtdrules; }

   /** loads xml Document (node) from input stream */
   void load( std::istream &instream, XMLContextPtr &ctxptr );

   /** saves node to xml output stream */
   void save( std::ostream &outstream );

   void load_file( const std::string& st )
   {
      std::ifstream file_istream;
      file_istream.open( st.c_str(), std::ios::in );
      this->load( file_istream, contextptr );
      file_istream.close();
   }

   void save_file( const std::string& st )
   {
      std::fstream file_stream;
      file_stream.open( st.c_str(), std::ios::in | std::ios::out );
      this->save( file_stream);
      file_stream.close();
   }


protected:
   /** node list of parsed processing instructions */
   XMLNodeList procinstructions;
   /** node list of document type definition rules */
   XMLNodeList dtdrules;
};

typedef cppdom_boost::shared_ptr<class XMLDocument> XMLDocumentptr;


/** Interface for xml parsing event handler */
class CPPDOM_API XMLEventHandler
{
public:
   /** ctor */
   XMLEventHandler(){}

   /** virtual dtor */
   virtual ~XMLEventHandler(){}

   /** called when parsing of an xml document starts */
   virtual void start_document(){}

   /** called when ended parsing a document */
   virtual void end_document(){}

   /** called when parsing a processing instruction */
   virtual void processing_instruction( XMLNode &pinode ){}

   /** called when start parsing a node */
   virtual void start_node( const XMLString &nodename ){}
   /** called when an attribute list was parsed */
   virtual void parsed_attributes( XMLAttributes &attr ){}
   /** called when parsing of a node was finished */
   virtual void end_node( XMLNode &node ){}

   /** called when a cdata section ended */
   virtual void got_cdata( const XMLString &cdata ){}
};


// namespace end
};

#endif
