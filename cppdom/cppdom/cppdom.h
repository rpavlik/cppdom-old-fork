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
 * the main declaration header
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
#include <list>
#include <map>
#include <iosfwd>
#include <fstream>
#include <iostream>
#include <cppdom/config.h>
#include <cppdom/shared_ptr.h>   // the boost::shared_ptr class

//! namespace of the cppdom project
namespace cppdom
{
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

   /**
    * xml error class
    * contains an XMLErrorCode and is thrown while parsing xml input
    */
   class CPPDOM_API XMLError
   {
   public:
      /** constructor */
      XMLError(XMLErrorCode code);

      /** returns the error code */
      XMLErrorCode getError() const;

      /** returns the string representation of the error code */
      void getStrError(std::string& error) const;

      std::string getString() const;

      /** return additional error info */
      std::string getInfo() const;

   protected:
      XMLErrorCode mErrorCode;
   };


   /**
    * xml stream position
    * represents the position in the xml input stream; usable if load()
    *    throws an error on parsing xml content
    */
   class CPPDOM_API XMLLocation
   {
   public:
      /** Constructor */
      XMLLocation();

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

   /** handle to a tagname string in a tagname map */
   typedef int XMLTagNameHandle;
   /** maps the tagname string to a handle */
   typedef std::map<XMLTagNameHandle,std::string> XMLTagNameMap;
   /** maps an entity to a string representation */
   typedef std::map<std::string,std::string> XMLEntityMap;
   /** smart pointer for XMLContext */
   typedef cppdom_boost::shared_ptr<class XMLContext> XMLContextPtr;
   /** smart pointer to the event handler */
   typedef cppdom_boost::shared_ptr<class XMLEventHandler> XMLEventHandlerPtr;

   /**
    * xml parsing context class.
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
      std::string getEntity(const std::string& entname);

      /** returns the tagname by the tagname handle */
      std::string getTagname(XMLTagNameHandle handle);

      /** inserts a tag name and returns a tag name handle to the string */
      XMLTagNameHandle insertTagname(const std::string& tagname);

      /** returns the current location in the xml stream */
      XMLLocation& getLocation();

      /** called once when the context instance starts up; overwrite to customize
      * @note: The base member should always be called, to set init to true
      */
      virtual void initContext();

      /** @name event handling methods */
      //@{
      /** sets the event handler; enables handling events */
      void setEventHandler(XMLEventHandlerPtr ehptr);

      /** returns the currently used eventhandler (per reference) */
      XMLEventHandler& getEventHandler();

      /** returns if a valid event handler is set */
      bool handleEvents() const;
      //@}

   protected:
      bool           mInit;               /**< indicates if init_context() was already called */
      int            mNextHandle;         /**< next available tagname handle */
      XMLTagNameMap  mTagNames;           /**< matches XMLTagNameHandles to the real std::string's */
      XMLEntityMap   mEntities;           /**< Contains entity codes and their string representations */
      XMLLocation    mLocation;           /**< location of the xml input stream */
      bool           mHandleEvents;       /**< indicates if the event handler is used */
      XMLEventHandlerPtr mEventHandler;   /**< current parsing event handler */
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



   /**
    * XML attribute class.
    * Just wraps a string (this is really just and attribute VALUE)
    */
   class CPPDOM_API XMLAttribute
   {
   public:
      XMLAttribute();
      XMLAttribute(const XMLAttribute& attr);
      XMLAttribute(const std::string& val);

#ifndef CPPDOM_NO_MEMBER_TEMPLATES
      template<class T>
      XMLAttribute(const T& val)
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
         std::string value(mData);
         return value;
      }
#endif // ! _MSC_VER
#endif // ! CPPDOM_NO_MEMBER_TEMPLATES

      /** Autoconversion to string (so old code should work) */
      operator std::string() const;

   protected:
      std::string mData;
   };

#ifndef CPPDOM_NO_MEMBER_TEMPLATES
#ifndef _MSC_VER
   template<>
   inline std::string XMLAttribute::getValue<std::string>() const
   {
      std::string value(mData);
      return value;
   }
#endif // ! _MSC_VER
#endif // ! CPPDOM_NO_MEMBER_TEMPLATES


   /**
    * xml tag attribute map
    * contains all attributes and values a tag has, represented in a map
    */
   class CPPDOM_API XMLAttributes: public std::map<std::string, std::string>
   {
      friend class XMLParser;
   public:
      /** ctor */
      XMLAttributes();

      /**
       * Get the named attribute
       * @returns empty string "" if not found, else the value
       */
      std::string get(const std::string& key) const;

      /**
       * Sets new attribute value
       * If not found, then just insert the new attribute
       */
      void set(const std::string& key, const std::string& value);

      /**
       * Check if the attribute map has the given attribute
       * @return false if not found
       */
      bool has(const std::string& key) const;
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
      XMLNode();

   public:
      /** constructor, takes xml context pointer */
      explicit XMLNode(XMLContextPtr pctx);

      XMLNode(const XMLNode& node);
      ~XMLNode();

      /** assign operator */
      XMLNode& operator=(const XMLNode& node);

      /** @name access to node info */
      //@{
      /** returns type of node */
      XMLNodeType getType() const;

      /** Returns the local name of the node (the element name) */
      std::string getName();

      /** returns attribute map of the node */
      XMLAttributes& getAttrMap();

      /**
       * Get the named attribute
       * @returns empty string ("") if not found, else the value
       * @post Object does not change.
       */
      XMLAttribute getAttribute(const std::string& name) const;

      /** Check if the node has a given attribute */
      bool hasAttribute(const std::string& name) const;

      /**
       * returns cdata string
       * @note: This only returns data for nodes that are leaf nodes of type "cdata".
       *        Calling this on a node that has cdata as a child does nothing
       */
      const std::string& getCdata();
      //@}

      /** @name node data manipulation */
      //@{
      /** sets new nodetype */
      void setType(XMLNodeType type);

      /** set the node name */
      void setName(const std::string& name);

      /** sets new cdata */
      void setCdata(const std::string& cdata);

      /**
       * sets new attribute value
       * @post Element.attr is set to value.  If it didn't exist before, now it does.
       */
      void setAttribute(const std::string& attr, const XMLAttribute& value);

      void addChild(XMLNodePtr& node);
      bool removeChild(XMLNodePtr& node);
      bool removeChild(std::string& childName);
      bool removeChildren(std::string& childName);
      //@}

      /** @name navigation through the nodes */
      //@{

      /** returns a list of the nodes children */
      XMLNodeList& getChildren();

      /** Returns the first child of the given local name */
      XMLNodePtr getChild(const std::string& name);

      /**
       * Returns a list of all children (one level deep) with local name of childName
       * \note currently no path-like childname can be passed, like in e.g. msxml
       * If has standard compose() functions, could impl this by calling getChildren(pred)
       */
      XMLNodeList getChildren(const std::string& name);

      /**
       * Returns list of all children.
       * @see getChildren
       * @note Needed because of predicate template matching char*
       */
      XMLNodeList getChildren(const char* name);

      /** Return a list of children that pass the given STL predicate */
      template<class Predicate>
      XMLNodeList getChildrenPred(Predicate pred)
      {
         XMLNodeList result(0);
         XMLNodeList::const_iterator iter;

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
      XMLNode* getParent() const;
      //@}

      /** @name load/save functions */
      //@{
      /** loads xml node from input stream */
      void load(std::istream& in, XMLContextPtr& context);

      /** saves node to xml output stream */
      void save(std::ostream& out, int indent=0);
      //@}

      XMLContextPtr getContext();

   protected:
      XMLTagNameHandle  mNodeNameHandle;  /**< handle to the real tag name */
      XMLContextPtr     mContext;         /**< smart pointer to the context class */
      XMLNodeType       mNodeType;        /**< The type of the node */
      XMLAttributes     mAttributes;      /**< Attributes of the element */
      std::string       mCdata;           /**< Character data (if there is any) */
      XMLNodeList       mNodeList;        /**< stl list with subnodes */
      XMLNode*          mParent;          /**< Our parent */
   };


   /** xml document */
   class CPPDOM_API XMLDocument: public XMLNode
   {
      friend class XMLParser;
   public:
      XMLDocument();

      /** constructor taking xml context pointer */
      explicit XMLDocument(XMLContextPtr context);

      /** returns a list of processing instruction nodes */
      XMLNodeList& getPiList();

      /** returns a list of document type definition rules to check the xml file */
      XMLNodeList& getDtdList();

      /** loads xml Document (node) from input stream */
      void load(std::istream& in, XMLContextPtr& context);

      /** saves node to xml output stream */
      void save(std::ostream& out);

      /**
       * \exception throws cppdom::XMLError when the file name is invalid.
       */
      void loadFile(const std::string& filename) throw(XMLError);

      void saveFile(const std::string& filename);


   protected:
      /** node list of parsed processing instructions */
      XMLNodeList mProcInstructions;

      /** node list of document type definition rules */
      XMLNodeList mDtdRules;
   };

   typedef cppdom_boost::shared_ptr<class XMLDocument> XMLDocumentPtr;


   /** Interface for xml parsing event handler */
   class CPPDOM_API XMLEventHandler
   {
   public:
      /** ctor */
      XMLEventHandler() {}

      /** virtual dtor */
      virtual ~XMLEventHandler() {}

      /** called when parsing of an xml document starts */
      virtual void startDocument() {}

      /** called when ended parsing a document */
      virtual void endDocument() {}

      /** called when parsing a processing instruction */
      virtual void processingInstruction(XMLNode& pinode) {}

      /** called when start parsing a node */
      virtual void startNode(const std::string& nodename) {}
      /** called when an attribute list was parsed */
      virtual void parsedAttributes(XMLAttributes& attr) {}
      /** called when parsing of a node was finished */
      virtual void endNode(XMLNode& node) {}

      /** called when a cdata section ended */
      virtual void gotCdata(const std::string& cdata) {}
   };
}

#endif
