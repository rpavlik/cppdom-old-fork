/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil c-basic-offset: 3 -*- */
// vim:cindent:ts=3:sw=3:et:tw=80:sta:
/*************************************************************** cppdom-cpr beg
 *
 * cppdom was forked from the original xmlpp version 0.6 under the LGPL. This
 * new, branched xmlpp is under the same LGPL (of course) and is being
 * maintained by:
 *      Kevin Meinert   <subatomic@users.sourceforge.net>
 *      Allen Bierbaum  <allenb@users.sourceforge.net>
 *      Ben Scott       <nonchocoboy@users.sourceforge.net>
 *
 * -----------------------------------------------------------------
 *
 * xmlpp - an xml parser and validator written in C++
 * copyright (c) 2000-2001 Michael Fink
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ************************************************************ cppdom-cpr-end */
/**
 * @file cppdom.h
 *
 * The main declaration header for all of cppdom.
 */

// prevent multiple includes
#ifndef CPPDOM_CPPDOM_H
#define CPPDOM_CPPDOM_H

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
#include <vector>
#include <iostream>


// ---- TYPE DEFS for CPPDOM --- //
/** handle to a tagname string in a tagname map */
namespace cppdom
{
   typedef int TagNameHandle;
}

#define CPPDOM_USE_HASH_MAP 1

// Use fastest map available
#if defined(CPPDOM_USE_HASH_MAP) && defined(__GNUC__) && (__GNUC__ >= 3)

#include <ext/hash_map>
#include <map>

namespace std
{ using namespace __gnu_cxx; }

namespace cppdom
{
   struct HashString
   {
      bool operator()(std::string const& str) const
      { return __gnu_cxx::hash<char const *>()(str.c_str()); }
   };

   typedef std::hash_map<TagNameHandle,std::string>   TagNameMap_t;
   typedef std::hash_map<std::string, TagNameHandle, HashString>  NameToTagMap_t;
}

#else

#include <map>
namespace cppdom
{
   typedef std::map<TagNameHandle,std::string>   TagNameMap_t;
   typedef std::map<std::string, TagNameHandle>  NameToTagMap_t;
}
#endif

#include "config.h"
#include "shared_ptr.h"   // the boost::shared_ptr class



/** Macro for constructing a CPPDOM error */
//std::string(__FILE__) + std::string(__LINE__)
#define CPPDOM_ERROR(error_code, local_desc) cppdom::Error(error_code, local_desc, __FILE__, __LINE__ )

//! namespace of the cppdom project
namespace cppdom
{
   // Declare a version string constant that can be used at run time.
   CPPDOM_EXPORT(const char*) getVersion();

   // Helper methods
   template <class T> inline void ignore_unused_variable_warning(const T&) { }

   // True if there are characters references: ex: &amp;
   CPPDOM_EXPORT(bool) textContainsXmlEscaping(const std::string& data);

   // True if there are chars needing escaping
   CPPDOM_EXPORT(bool) textNeedsXmlEscaping(const std::string& data, bool isCdata);

    // Remove escaping from xml text
   CPPDOM_EXPORT(std::string) removeXmlEscaping(const std::string& data, bool isCdata);

   // Add escaping to xml text
   CPPDOM_EXPORT(std::string) addXmlEscaping(const std::string& data, bool isCdata);

   /** Method to split string base on seperator.
    *
    * If separator does not exist in string, then just return that string in the output.
    *
    * @example:
    *    std::string s = "apple, orange, cherry, peach, grapefruit, cantalope,watermelon";
    *    std::vector<std::string> v;
    *    split( s, " ,", std::back_inserter(v) );
    */
   template<class OutIt>
   void splitStr(
      const std::string& s,
      const std::string& sep,
      OutIt dest)
   {
      std::string::size_type left = s.find_first_not_of( sep );
      std::string::size_type right = s.find_first_of( sep, left );
      while( left < right )
      {
         *dest = s.substr( left, right-left );
         ++dest;
         left = s.find_first_not_of( sep, right );
         right = s.find_first_of( sep, left );
      }
   }

   //! xml parsing error codes enumeration
   enum ErrorCode
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

      xml_invalid_operation,        /**< Attempted to execute an xml operation that would cause invalid structure */
      xml_invalid_argument,         /**< Attempted to pass an invalid argument */
   // added by kevin for 0.7 compatibility...
      xml_filename_invalid,
      xml_file_access,
      xml_escaping_failure,         /**< Problem with escaping */

      xml_dummy                     /**< dummy error code */
   };
   
   // typedefs
   /** smart pointer to node */
   class Node;
   typedef cppdom_boost::shared_ptr<cppdom::Node> NodePtr;
   
   class Document;
   typedef cppdom_boost::shared_ptr<cppdom::Document> DocumentPtr;


   /** list of node smart pointer */
   typedef std::vector<NodePtr> NodeList;
   typedef NodeList::iterator NodeListIterator;


   // classes

   /**
    * xml error class
    * contains an ErrorCode and is thrown while parsing xml input
    */
   class CPPDOM_CLASS Error : public std::exception
   {
   public:
      /** constructor */
      Error(ErrorCode code, std::string localDesc, std::string location);

      /** constructor that can take line num */
      Error(ErrorCode code, std::string localDesc, std::string file, unsigned line_num);

      /** returns the error code */
      ErrorCode getError() const;

      virtual ~Error() throw();

      /** returns the string representation of the error code */
      std::string getStrError() const;

      std::string getString() const;

      /** return additional error info */
      std::string getInfo() const;

      virtual const char* what() const throw();

   protected:
      ErrorCode   mErrorCode;
      std::string mLocalDesc; /**< Local description of the error */
      std::string mLocation;  /**< The location text */
   };


   /**
    * xml stream position
    * represents the position in the xml input stream; usable if load()
    *    throws an error on parsing xml content
    */
   class CPPDOM_CLASS Location
   {
   public:
      /** Constructor */
      Location();

      /** returns current line */
      int getLine() const;

      /** returns current position in a line */
      int getPos() const;

      /** advances a char */
      void step(int chars = 1);

      /** indicates entering a new line */
      void newline();

      /** reset location */
      void reset();

   protected:
      int mLine;
      int mPos;
   };


   // typedefs
   /** smart pointer for Context */
   typedef cppdom_boost::shared_ptr<class Context> ContextPtr;

   /** smart pointer to the event handler */
   typedef cppdom_boost::shared_ptr<class EventHandler> EventHandlerPtr;

   /**
    * xml parsing context class.
    * the class is the parsing context for the parsed xml document.
    * the class has a tagname lookup table and an entity map
    */
   class CPPDOM_CLASS Context
   {
   public:
      /** ctor */
      Context();
      /** dtor */
      virtual ~Context();

      /** returns the tagname by the tagname handle */
      std::string getTagname(TagNameHandle handle);

      /** inserts a tag name and returns a tag name handle to the string */
      TagNameHandle insertTagname(const std::string& tagname);

      /** returns the current location in the xml stream */
      Location& getLocation();

      /** @name event handling methods */
      //@{
      /** sets the event handler; enables handling events */
      void setEventHandler(EventHandlerPtr ehptr);

      /** returns the currently used eventhandler (per reference) */
      EventHandler& getEventHandler();

      /** returns if a valid event handler is set */
      bool hasEventHandler() const;
      //@}

   protected:
      bool              mInit;            /**< indicates if init_context() was already called */
      int               mNextHandle;      /**< next available tagname handle */
      TagNameMap_t      mTagToName;       /**< matches TagNameHandles to the real std::string's */
      NameToTagMap_t    mNameToTag;       /**< Map name to tag for inserting. */
      Location          mLocation;        /**< location of the xml input stream */
      bool              mHandleEvents;    /**< indicates if the event handler is used */
      EventHandlerPtr   mEventHandler;    /**< current parsing event handler */
   };
   

   /**
    * XML attribute class.
    * Just wraps a string (this is really just and attribute VALUE)
    * This is just meant to be a "magic" class to provide some syntactic
    * sugar related to autoconversions and get<value>() usefullness.
    */
   class CPPDOM_CLASS Attribute
   {
   public:
      Attribute();
      Attribute(const Attribute& attr);
      Attribute(const std::string& val);

#ifndef CPPDOM_NO_MEMBER_TEMPLATES
      template<class T>
      Attribute(const T& val)
      {
         setValue<T>(val);
      }
#endif // ! CPPDOM_NO_MEMBER_TEMPLATES

      const std::string& getString() const;

#ifndef CPPDOM_NO_MEMBER_TEMPLATES
      /**
       * Set mData to the string value of val
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

      // Specializations of getValue<T> placed inline for Visual Studio 7.
      // MIPSpro and GCC do not handle this. They get out-of-line
      // specializations, found below.
#ifdef _MSC_VER
      /**
       * Specialization of getValue<T> for std::string so that more than just the
       * first word of the attribute string is returned.
       */
      template<>
      std::string getValue<std::string>() const
      {
         return mData;
      }
#endif // ! _MSC_VER
#endif // ! CPPDOM_NO_MEMBER_TEMPLATES

      /** Autoconversion to string (so old code should work) */
      operator std::string() const;

      bool operator==(const Attribute& rhs)
      {
         return rhs.mData == mData;
      }

      bool operator!=(const Attribute& rhs)
      {
         return !operator==(rhs);
      }

   protected:
      std::string mData;
   };

#ifndef CPPDOM_NO_MEMBER_TEMPLATES
#ifndef _MSC_VER
   template<>
   inline std::string Attribute::getValue<std::string>() const
   {
      return mData;
   }
#endif // ! _MSC_VER
#endif // ! CPPDOM_NO_MEMBER_TEMPLATES


   /**
    * XML tag attribute map.
    * Contains all attributes and values a tag has, represented in a map.
    * maps: attrib_name:string --> attrib_value:Attribute
    * Wraps a standard map with a couple of helper methods.
    *
    * WARNING: Since we are deriving from std::map and std::map has no
    * virtual destructor you can not use this class polymorphically.
    * In other words don't allocate an instance of this class with new
    * and store it with an std::map<>* .
    */
   class CPPDOM_CLASS Attributes : public std::map<std::string, Attribute>
   {
   public:
            /** ctor */
      Attributes();

      /**
       * Get the named attribute.
       * @returns empty string "" if not found, else the value.
       */
      Attribute get(const std::string& key) const;

      /**
       * Sets new attribute value.
       * If not found, then just insert the new attribute.
       */
      void set(const std::string& key, const Attribute value);

      /**
       * Check if the attribute map has the given attribute.
       * @return false if not found.
       */
      bool has(const std::string& key) const;
   };


   /** xml node.
   * A node has the following properties
   * name - The element name of the node
   * type - The type of the node. see NodeType
   * children - Child elements of the node
   * cdata - the cdata content if of type cdata
   *
   */
   class CPPDOM_CLASS Node
   {
   public:
         /** node type enumeration */
      enum Type
      {
         xml_nt_node,      /**< normal node, can contain subnodes */
         xml_nt_leaf,      /**< a leaf node, which contains no further nodes, eg. <img/> */
         xml_nt_document,  /**< document root node */
         xml_nt_cdata      /**< cdata node, which only contains char data */
      };

      friend class Parser;
   protected:
      /** Default Constructor */
      Node();

   public:
      /** constructor, takes xml context pointer */
      explicit Node(ContextPtr pctx);

      /** Construct a node with a given name */
      explicit Node(std::string nodeName, ContextPtr ctx);

      Node(const Node& node);

      ~Node();

      /** Create a node. */
      static NodePtr create(std::string nodeName, ContextPtr ctx);

      /** Create a node. */
      static NodePtr create(std::string nodeName, NodePtr parent);

   public:
      /** assign operator */
      Node& operator=(const Node& node);

      /** Returns true if the nodes are equal
      * @param ignoreAttribs - Attributes to ignore in the comparison
      * @param ignoreElements - Elements to ignore in the comparison
      */
      bool isEqual(NodePtr otherNode, const std::vector<std::string>& ignoreAttribs,
                   const std::vector<std::string>& ignoreElements,
                   bool dbgit=false, const unsigned debugIndent=0);

      bool isEqual(NodePtr otherNode)
      {
         std::vector<std::string> empty_strings;
         return isEqual(otherNode, empty_strings, empty_strings );
      }

      /** Returns the local name of the node (the element name) */
      std::string getName();
      /** set the node name */
      void setName(const std::string& name);

      /** @name Type information */
      //@{
      /** returns type of node */
      Node::Type getType() const;

      bool isNode()
      { return getType() == xml_nt_node;}
      bool isLeaf()
      { return getType() == xml_nt_leaf;}
      bool isDocument()
      { return getType() == xml_nt_document;}
      bool isCData()
      { return getType() == xml_nt_cdata;}

      /** sets new nodetype */
      void setType(Node::Type type);
      //@}

      /** @name Attribute information */
      //@{
      /** Check if the node has a given attribute */
      bool hasAttribute(const std::string& name) const;

      /**
       * Get the named attribute
       * @returns empty string ("") if not found, else the value
       * @post Object does not change.
       */
      Attribute getAttribute(const std::string& name) const;

      /**
       * sets new attribute value
       * @param attr - Attribute name to set.  There must not be ANY spaces in this name
       * @post Element.attr is set to value.  If it didn't exist before, now it does.
       */
      void setAttribute(const std::string& attr, const Attribute& value);

      /** Direct access to attribute map. */
      Attributes& attrib();

      /** Direct access to attribute map. */
      const Attributes& attrib() const;

      /** returns attribute map of the node
       * @deprecated
       */
      Attributes& getAttrMap();

      /** returns attribute map of the node
       * @deprecated
       */
      const Attributes& getAttrMap() const;

      //@}

      /** @name Children and parents */
      //@{
      /** Returns true if the node has a child of the given name.
       * @param name    Name can be a single element name or a chain of the form "tag/tag/tag"
       */
      bool hasChild(const std::string& name);

      /** Returns the first child of the given local name.
       */
      NodePtr getChild(const std::string& name);

      /** Return first child of the given name.
       * @param name    Name can be a single element name or a chain of the form "tag/tag/tag"
       */
      NodePtr getChildPath(const std::string& path);

      /** returns a list of the nodes children */
      NodeList& getChildren();

      /**
       * Returns a list of all children (one level deep) with local name of childName
       * \note currently no path-like childname can be passed, like in e.g. msxml
       * If has standard compose() functions, could impl this by calling getChildren(pred)
       */
      NodeList getChildren(const std::string& name);

      /**
       * Returns list of all children.
       * @see getChildren
       * @note Needed because of predicate template matching char*
       */
      NodeList getChildren(const char* name);

      /** Return a list of children that pass the given STL predicate */
      template<class Predicate>
      NodeList getChildrenPred(Predicate pred)
      {
         NodeList result(0);
         NodeList::const_iterator iter;

         // search for all occurances of nodename and insert them into the new list
         for(iter = mNodeList.begin(); iter != mNodeList.end(); ++iter)
         {
            if (pred(*iter))
            {
               result.push_back(*iter);
            }
         }
         return result;
      }

      /**
       * Returns our parent.  The returned value could be NULL.
       *
       * @return Our parent, which may be NULL if we have no parent.
       */
      Node* getParent() const;

      /**
       * Returns the full path of this node.
       */
      std::string getPath();

      void addChild(NodePtr& node);
      bool removeChild(NodePtr& node);
      bool removeChild(std::string& childName);
      bool removeChildren(std::string& childName);

      //@}

      /** @name CData methods */
      //@{
      /**
       * returns cdata string
       * @note: For node type "cdata", this returns the local cdata.
       *        For other nodes, this attempts to find the first child cdata
       *        node and returns its data.
       */
      std::string getCdata();

      /**
       * Returns the full cdata of the node or immediate children.
      * @note: For node type "cdata", this returns the local cdata.
      *        For other nodes, combines the cdata of all cdata children.
      */
      std::string getFullCdata();

      /** Sets the node cdata.
      * @post For cdata type nodes, this sets the contained cdata
      *       For other types, this sets the cdata of the first cdata node.
      *       If none exists, then one is created called "cdata".
      */
      void setCdata(const std::string& cdata);
      //@}


      /** @name load/save functions */
      //@{
      /** loads xml node from input stream */
      void load(std::istream& in, ContextPtr& context);

      /** saves node to xml output stream
      * @param indent - The amount to indent
      * @doIndent - If true, then indent the output
      * @doNewline - If true then use newlines in output
      */
      void save(std::ostream& out, int indent=0, bool doIndent=true, bool doNewline=true);
      //@}

      /** Returns the context used for this node. */
      ContextPtr getContext();

   protected:
      TagNameHandle  mNodeNameHandle;  /**< handle to the real tag name */

//#ifdef CPPDOM_DEBUG
      std::string    mNodeName_debug;  /**< The node name for debugging */
//#endif
      ContextPtr     mContext;         /**< smart pointer to the context class */
      Node::Type     mNodeType;        /**< The type of the node */
      Attributes     mAttributes;      /**< Attributes of the element */
      std::string    mCdata;           /**< Character data (if there is any) */
      NodeList       mNodeList;        /**< stl list with subnodes */
      Node*          mParent;          /**< Our parent */
   };


   /** XML document root node.
    *
    * Structure of a cppdom document
    *
    * Nested tree of nodes.  Each node has a type (Node::Type).
    * Root node is of type document.  Standard nodes are type node.
    * Under standard nodes there can be cdata nodes.  These nodes are nodes
    * for containing the raw text inside an element.
    *
    */
   class CPPDOM_CLASS Document: public Node
   {
      friend class Parser;
   public:
      Document();

      ~Document();

      /** constructor taking xml context pointer */
      explicit Document(ContextPtr context);

      /** constructor taking xml context pointer */
      explicit Document(std::string docName, ContextPtr context);

      /** returns a list of processing instruction nodes */
      NodeList& getPiList();

      /** returns a list of document type definition rules to check the xml file */
      NodeList& getDtdList();

      /** loads xml Document (node) from input stream */
      void load(std::istream& in, ContextPtr& context);

      /** saves node to xml output stream
      * @param doIndent - If true, then indent the output.
      * @param doNewline - If true, then use newlines in the output.
      */
      void save(std::ostream& out, bool doIndent=true, bool doNewline=true);

      /**
       * \exception throws cppdom::Error when the file name is invalid.
       */
      void loadFile(const std::string& filename) throw(Error);
      void loadFileChecked(const std::string& filename);

      /** Save the document to the given filename.
      * @todo Fix this method.  It doesn't work
      */
      void saveFile(std::string filename);


   protected:
      /** node list of parsed processing instructions */
      NodeList mProcInstructions;

      /** node list of document type definition rules */
      NodeList mDtdRules;
   };

   /** Interface for xml parsing event handler */
   class CPPDOM_CLASS EventHandler
   {
   public:
      /** ctor */
      EventHandler() {}

      /** virtual dtor */
      virtual ~EventHandler() {}

      /** called when parsing of an xml document starts */
      virtual void startDocument() {}

      /** called when ended parsing a document */
      virtual void endDocument() {}

      /** called when parsing a processing instruction */
      virtual void processingInstruction(Node& pinode)
      {  cppdom::ignore_unused_variable_warning(pinode);}

      /** called when start parsing a node */
      virtual void startNode(const std::string& nodename)
      { cppdom::ignore_unused_variable_warning(nodename); }
      /** called when an attribute list was parsed */
      virtual void parsedAttributes(Attributes& attr)
      { cppdom::ignore_unused_variable_warning(attr); }
      /** called when parsing of a node was finished */
      virtual void endNode(Node& node)
      { cppdom::ignore_unused_variable_warning(node);}

      /** called when a cdata section ended */
      virtual void gotCdata(const std::string& cdata)
      { cppdom::ignore_unused_variable_warning(cdata); }
   };


   // ----------------------------------- //
   /** @name Helper methods */
   //@{
   /** Merges data from one node to another.
    * @param fromNode   Node to read data from.
    * @param toNode     Node to merge data onto.
    * @post All elements and attributes in fromNode will exist in toNode.
    */
   CPPDOM_EXPORT(void) merge(NodePtr fromNode, NodePtr toNode);
   //@}
}

inline std::ostream& operator<<(std::ostream& out, const cppdom::Attribute& attrib)
{
   out << attrib.getString();
   return out;
}



#endif
