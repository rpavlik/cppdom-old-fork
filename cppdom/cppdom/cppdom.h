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

   // Visual Studio 7 has support for member templates.
#  if _MSC_VER < 1300
#     define CPPDOM_NO_MEMBER_TEMPLATES
#  endif
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
   XMLError( XMLErrorCode code ) { errorcode = code; }

   /** returns the error code */
   XMLErrorCode getError() const { return errorcode; }

   /** returns the string representation of the error code */
   void getStrError(XMLString &error) const;
   
   XMLString getString() const
   {
      XMLString err;
      this->getStrError( err );
      return err;
   }
   
   /** return additional error info */
   const XMLString getInfo() const { return "unknown error"; }

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
   { 
      reset(); 
   }

   /** returns current line */
   int getLine() const
   { 
      return line; 
   }

   /** returns current position in a line */
   int getPos() const
   { 
      return pos; 
   }

   /** advances a char */
   void step( int chars = 1 )
   { 
      pos += chars; 
   }
   
   /** indicates entering a new line */
   void newline()
   { 
      ++line; 
      pos = 1; 
   }

   /** reset location */
   void reset()
   { 
      line = pos = 1; 
   }

protected:
   int line, pos;
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
   XMLString getEntity( const XMLString &entname );

   /** returns the tagname by the tagname handle */
   XMLString getTagname( XMLTagNameHandle handle );

   /** inserts a tag name and returns a tag name handle to the string */
   XMLTagNameHandle insertTagname( const XMLString &tagname );

   /** returns the current location in the xml stream */
   XMLLocation &getLocation()
   { 
      return location; 
   }

   /** called once when the context instance starts up; overwrite to customize
   * @note: The base member should always be called, to set init to true
   */
   virtual void initContext()
   { 
      init = true; 
   }

   /** @name event handling methods */
   //@{
   /** sets the event handler; enables handling events */
   void setEventHandler( XMLEventHandlerPtr ehptr )
   {
      eventhandler = ehptr;
      handleevents = true;
   }

   /** returns the currently used eventhandler (per reference) */
   XMLEventHandler &getEventHandler()
   { 
      return *eventhandler.get(); 
   }

   /** returns if a valid event handler is set */
   bool handleEvents() const
   { 
      return handleevents; 
   }

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
enum XMLNodeType
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
   XMLAttribute() : mData( "" ) {}

   XMLAttribute( const XMLAttribute& r )
   {
      mData = r.mData;
   }

   XMLAttribute( const std::string& str_val )
   {
      mData = str_val;
   }

#ifndef CPPDOM_NO_MEMBER_TEMPLATES
   template<class T>
   XMLAttribute( const T& val )
   {
      setValue<T>( val );
   }
#endif // ! CPPDOM_NO_MEMBER_TEMPLATES

   XMLString getString() const
   {
      return mData; 
   }

#ifndef CPPDOM_NO_MEMBER_TEMPLATES
   /** Set mData to the string value of val
   * @note Requires a stream operation of type T
   */
   template<class T>
   void setValue( const T& val )
   {
      std::ostringstream oss;
      oss << val;
      mData = oss.str();
   }

   template<class T>
   T getValue() const
   {
      T t;
      std::istringstream iss( mData );
      iss >> t;
      return t;
   }
#endif // ! CPPDOM_NO_MEMBER_TEMPLATES

   /** Autoconversion to string (so old code should work) */
   operator std::string() const
   { 
      return mData; 
   }

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
* type - The type of the node. see XMLNodeType
* children - Child elements of the node
* cdata - the cdata content if of type cdata
*
*/
class CPPDOM_API XMLNode
{
   friend class XMLParser;
protected:
   /** Default Constructor */
   XMLNode() : mParent(NULL)
   { 
      nodetype = xml_nt_node;
   }

public:
   /** constructor, takes xml context pointer */
   explicit XMLNode( XMLContextPtr pctx ) : mParent(NULL)
   { 
      nodetype = xml_nt_node; 
      contextptr = pctx;
   }
   
   /** Destructor */
   ~XMLNode()
   {
      // Disown our rotten children.  Even Brandine!
      for ( XMLNodeList::iterator i = mNodelist.begin(); i != mNodelist.end(); ++i )
      {
         (*i)->mParent = NULL;
      }
   }
   
   /** copy constructor */
   XMLNode( const XMLNode &node );
   
   /** assign operator */
   XMLNode &operator =( const XMLNode &node );

   /** @name access to node info */
   //@{
   /** returns type of node */
   XMLNodeType getType() const
   { 
      return nodetype; 
   }

   /** Returns the local name of the node (the element name) */
   XMLString getName();
   
   /** returns attribute map of the node */
   XMLAttributes &getAttrMap()
   { 
      return attributes; 
   }

   /** Get the named attribute
   * @returns empty string ("") if not found, else the value
   * @post Object does not change.
   */
   XMLAttribute getAttribute( const XMLString& name ) const
   { 
      return attributes.get(name); 
   }

   /** Check if the node has a given attribute */
   bool hasAttribute( const XMLString& name ) const
   { 
      return attributes.has( name ); 
   }

   /** returns cdata string
    * @note: This only returns data for nodes that are leaf nodes of type "cdata".
    *        Calling this on a node that has cdata as a child does nothing
    */
   const XMLString &getCdata()
   { 
      return mCdata; 
   }
   //@}

   /** @name node data manipulation */
   //@{
   /** sets new nodetype */
   void setType( XMLNodeType ntype )
   { 
      nodetype = ntype; 
   }

   /** set the node name */
   void setName( const XMLString& nname);

   /** sets new cdata */
   void setCdata( const XMLString &ncdata )
   { 
      mCdata = ncdata; 
   }

   /** sets new attribute value
   * @post Element.attr is set to value.  If it didn't exist before, now it does.
   */
   void setAttribute( const XMLString &attr, const XMLAttribute& value )
   {
      attributes.set( attr, value.getString() );
   }

   void addChild( XMLNodePtr& node )
   {
      node->mParent = this;      // Tell the child who their daddy is
      mNodelist.push_back( node );
   }

   bool removeChild( XMLNodePtr& node )
   {
      std::cout << "cppdom::XMLNode::removeChildren:  not implemented\n";
      return false;
   }

   bool removeChild( XMLString& childName )
   {
      std::cout << "cppdom::XMLNode::removeChildren:  not implemented\n";
      return false;
   }

   bool removeChildren(XMLString& childName)
   {
      std::cout << "cppdom::XMLNode::removeChildren:  not implemented\n";
      return false;
   }

   //@}

   /** @name navigation through the nodes */
   //@{

   /** returns a list of the nodes children */
   XMLNodeList& getChildren()
   { 
      return mNodelist; 
   }

   /** Returns the first child of the given local name */
   XMLNodePtr getChild( const XMLString& name );

   /** Returns a list of all children (one level deep) with local name of childName
   * \note currently no path-like childname can be passed, like in e.g. msxml
   * If has standard compose() functions, could impl this by calling getChildren(pred)
   */
   XMLNodeList getChildren( const XMLString& name )
   {
      XMLNodeList ret_nlist(0);
      XMLNodeList::const_iterator iter;

      // search for all occurances of nodename and insert them into the new list
      for(iter = mNodelist.begin(); iter != mNodelist.end(); ++iter)
      {
         if ((*iter)->getName() == name)
         { 
            ret_nlist.push_back(*iter); 
         }
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
         if (pred( *iter ))
         { 
            ret_nlist.push_back( *iter ); 
         }
      }
      return ret_nlist;
   }

   /**
    * Returns our parent.  The returned value could be NULL.
    *
    * @return Our parent, which may be NULL if we have no parent.
    */
   XMLNode* getParent () const
   {
      return mParent;
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
   { 
      return contextptr;
   }

protected:
   XMLTagNameHandle  nodenamehandle;   /**< handle to the real tag name */
   XMLContextPtr     contextptr;       /**< smart pointer to the context class */
   XMLNodeType       nodetype;         /**< The type of the node */
   XMLAttributes     attributes;       /**< Attributes of the element */
   XMLString         mCdata;           /**< Character data (if there is any) */
   XMLNodeList       mNodelist;        /**< stl list with subnodes */
   XMLNode*          mParent;          /**< Our parent */
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
      contextptr = pctx;
   }

   /** returns a list of processing instruction nodes */
   XMLNodeList &getPiList()
   { 
      return procinstructions; 
   }

   /** returns a list of document type definition rules to check the xml file */
   XMLNodeList &getDtdList()
   { 
      return dtdrules; 
   }

   /** loads xml Document (node) from input stream */
   void load( std::istream &instream, XMLContextPtr &ctxptr );

   /** saves node to xml output stream */
   void save( std::ostream &outstream );

   /**
    * \exception throws cppdom::XMLError when the file name is invalid.
    */
   void loadFile( const std::string& st ) throw(XMLError)
   {
      std::ifstream file_istream;
      file_istream.open( st.c_str(), std::ios::in );

      if ( ! file_istream.good() )
      {
         throw XMLError(xml_filename_invalid);
      }

      this->load( file_istream, contextptr );
      file_istream.close();
   }

   void saveFile( const std::string& st )
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

typedef cppdom_boost::shared_ptr<class XMLDocument> XMLDocumentPtr;


/** Interface for xml parsing event handler */
class CPPDOM_API XMLEventHandler
{
public:
   /** ctor */
   XMLEventHandler(){}

   /** virtual dtor */
   virtual ~XMLEventHandler(){}

   /** called when parsing of an xml document starts */
   virtual void startDocument(){}

   /** called when ended parsing a document */
   virtual void endDocument(){}

   /** called when parsing a processing instruction */
   virtual void processingInstruction( XMLNode &pinode ){}

   /** called when start parsing a node */
   virtual void startNode( const XMLString &nodename ){}
   /** called when an attribute list was parsed */
   virtual void parsedAttributes( XMLAttributes &attr ){}
   /** called when parsing of a node was finished */
   virtual void endNode( XMLNode &node ){}

   /** called when a cdata section ended */
   virtual void gotCdata( const XMLString &cdata ){}
};


// namespace end
};

#endif
