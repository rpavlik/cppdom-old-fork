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
/** \file SpiritParser.h

  definitions for the spirit parsing classes

*/
#ifndef CPPDOM_SPIRIT_PARSER_H
#define CPPDOM_SPIRIT_PARSER_H

#include <cppdom/cppdom.h>

//#define BOOST_SPIRIT_DEBUG 1
//#define BOOST_SPIRIT_DEBUG_PRINT_SOME 100
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/spirit/utility.hpp>
#include <boost/bind.hpp>

#include <boost/spirit/iterator/multi_pass.hpp>
#include <iostream>


namespace bs = boost::spirit;
using namespace boost::spirit;

namespace cppdom
{

namespace spirit
{

/** Class for building an xml node tree from a spirit parser.
 */
template<typename IteratorType>
class XmlBuilder
{
public:
   /** Constructe an XmlBuilder and init for document. */
   //XmlBuilder(cppdom::DocumentPtr doc, cppdom::ContextPtr context=cppdom::ContextPtr());

   /** Prepare for parsing a new document. */
   void reinit(cppdom::Document* doc, cppdom::ContextPtr context)
   {
      mDocRoot = doc;
      mContext = doc->getContext();
      if(mContext.get() == NULL)
      {
         throw CPPDOM_ERROR(xml_invalid_argument, "Attempted to use doc with no context.");
      }

      mNodeStack.clear();
      mNodeStack.push_back(mDocRoot);     // Doc is at base of stack
   }

   /** Call when parser is done. */
   void finish()
   {
      assert(mNodeStack.size() == 1);     // Should only have one node left now
   }

   /** Called when element is found and starts. */
   void startElement(IteratorType first, IteratorType last)
   {
      std::string elt_name(first, last);
      std::cout << "Elt in: [" << elt_name << "]" << std::endl;

      // - Create node
      // - Add to tree
      // - Put on stack to get ready for more
      NodePtr new_elt(new cppdom::Node(elt_name,mContext));
      mNodeStack.back()->addChild(new_elt);
      mNodeStack.push_back(new_elt.get());
   }

   /** Called when element ends. */
   void endElement(IteratorType first, IteratorType last)
   {
      std::string elt_name(first,last);
      std::cout << "Elt exit: " << elt_name << std::endl;
      // Should always have more then one on stack since doc root is
      // on stack at all times and we need one more to pop
      assert(mNodeStack.size() > 1);
      mNodeStack.pop_back();

   #ifdef _DEBUG
      mCurAttribute.clear();     // If debug version, clear attribute so we detect problems
   #endif
   }

      /** Called when element ends. */
   void endElementQuick(IteratorType first, IteratorType last)
   {
      std::cout << "Elt exit quick.." << std::endl;
      // Should always have more then one on stack since doc root is
      // on stack at all times and we need one more to pop
      assert(mNodeStack.size() > 1);
      mNodeStack.pop_back();

   #ifdef _DEBUG
      mCurAttribute.clear();     // If debug version, clear attribute so we detect problems
   #endif
   }

   /** Called at the start of an attribute. */
   void startAttribute(IteratorType first, IteratorType last)
   {
      std::string attrib(first,last);
      std::cout << "  attrib: " << attrib << std::endl;
      mCurAttribute = attrib;
      assert(!mCurAttribute.empty());
   }

   /** Called with value of attribute. */
   void attribValue(IteratorType first, IteratorType last)
   {
      std::string attrib_value(first,last);
      std::cout << " [" << attrib_value << "] " << std::endl;
      assert(!mCurAttribute.empty());
      if(cppdom::textContainsXmlEscaping(attrib_value))
      {  attrib_value = removeXmlEscaping(attrib_value, false); }
      mNodeStack.back()->attrib().set(mCurAttribute, attrib_value);
   }

   /** Called with element content text. */
   void elementText(IteratorType first, IteratorType last)
   {
      std::string elem_text(first,last);
      if(textContainsXmlEscaping(elem_text))
      {  elem_text = removeXmlEscaping(elem_text, true); }

      std::cout << "   Text: [" << elem_text << "] " << std::endl;
      if(!elem_text.empty())
      {
         cppdom::NodePtr cdata_node(new cppdom::Node("cdata",mContext));
         cdata_node->setType(Node::xml_nt_cdata);
         cdata_node->setCdata(elem_text);
         mNodeStack.back()->addChild(cdata_node);
      }
   }

   cppdom::Document* getDocRoot()
   { return mDocRoot; }

public:
   cppdom::Document*             mDocRoot;      /**< The root of the document. */
   std::vector<cppdom::Node*>    mNodeStack;    /**< The current stack of nodes. */
   cppdom::ContextPtr            mContext;      /**< The context we are using in the builder. */

   std::string                   mCurAttribute; /**< Name of the current attribute we are using. */
};

/**
 * XML grammar.
 * Based on: http://www.w3.org/TR/2004/REC-xml-20040204/
 *
 * @tparam BUILDER_T must implement the interface concept similar to
 *         XmlBuilder above.
 */
template<typename BUILDER_T>
struct XmlGrammar : public grammar<XmlGrammar<BUILDER_T> >
{
   XmlGrammar(BUILDER_T* builder)
      : mBuilder(builder)
   {;}

   BUILDER_T* mBuilder;

   /** Grammar definition. */
   template <typename ScannerT>
   struct definition
   {
      definition(XmlGrammar const& self)
      {
         document = prolog >> element >> *misc;       // Main document root
         ws = +space_p;                               // Whitespace, simplified from XML spec

         chset<> char_data_set(anychar_p - chset_p("<"));   // May need & in this, but that doesn't seem right for text data

         //char_data_char = anychar_p - (chset_p('<') | chset_p('&'));
         char_data = *char_data_set;

         // Name specifications.  Pretty strict for now
         name_char = alnum_p | '_' | '-' | ':';
         name = (alpha_p | '_' | ':') >> *(alnum_p | '_' | '-' | ':');
         names = name >> *(blank_p >> name);
         nmtoken = +name_char;

         comment = confix_p("<!--", *anychar_p, "-->");  // Slightly less strict.

         pi = str_p("<?") >> name >> anychar_p >> str_p("?>");

         cdata_sect = str_p("<![CDATA[") >> cdata >> str_p("]]>");
         cdata = *anychar_p;

         misc = comment | pi | ws;
         prolog = !xmldecl >> *misc >> !(doctypedecl >> *misc);
         xmldecl = confix_p("<?xml", *anychar_p, "?>");        // Use confix since any_char cosumes everything

         doctypedecl = str_p("<!DOCTYPE")
                           >> *(anychar_p - chset_p("[>"))
                           >> !('[' >> *(anychar_p - ']') >> ']')
                           >> *space_p >> '>';

         // element = (start element) >> ( (more elts >> end) | (quick end))
         element = ch_p('<') >> name[boost::bind(&BUILDER_T::startElement,self.mBuilder,_1,_2)] >> *(ws >> attribute) >> *space_p
                   >> ( (ch_p('>') >> elem_content >> str_p("</")
                                   >> name[boost::bind(&BUILDER_T::endElement,self.mBuilder,_1,_2)] >> *space_p >> ch_p('>')) |
                        (str_p("/>"))[boost::bind(&BUILDER_T::endElementQuick,self.mBuilder,_1,_2)] );

         // elementText: eliminate extra space by consuming initial whitespace & only call when char_data matches
         elem_content = *space_p >> !(char_data[boost::bind(&BUILDER_T::elementText,self.mBuilder,_1,_2)])
                          >> *( (element | comment | pi | cdata_sect) >> *space_p >> !(char_data[boost::bind(&BUILDER_T::elementText,self.mBuilder,_1,_2)]) );

         attribute = name[boost::bind(&BUILDER_T::startAttribute,self.mBuilder,_1,_2)] >> *space_p >> '=' >> *space_p >> attrib_value;
         attrib_value = ('"' >> (*(anychar_p-'"'))[boost::bind(&BUILDER_T::attribValue,self.mBuilder,_1,_2)] >> '"') |
                        ("'" >> (*(anychar_p-"'"))[boost::bind(&BUILDER_T::attribValue,self.mBuilder,_1,_2)] >> "'");

         BOOST_SPIRIT_DEBUG_RULE(attribute);
         BOOST_SPIRIT_DEBUG_RULE(attrib_value);
         BOOST_SPIRIT_DEBUG_RULE(cdata_sect);
         BOOST_SPIRIT_DEBUG_RULE(cdata);
         BOOST_SPIRIT_DEBUG_RULE(char_data);
         BOOST_SPIRIT_DEBUG_RULE(char_data_char);
         BOOST_SPIRIT_DEBUG_RULE(comment);
         BOOST_SPIRIT_DEBUG_RULE(document);
         BOOST_SPIRIT_DEBUG_RULE(doctypedecl);
         BOOST_SPIRIT_DEBUG_RULE(element);
         BOOST_SPIRIT_DEBUG_RULE(elem_content);
         BOOST_SPIRIT_DEBUG_RULE(misc);
         BOOST_SPIRIT_DEBUG_RULE(name);
         BOOST_SPIRIT_DEBUG_RULE(name_char);
         BOOST_SPIRIT_DEBUG_RULE(names);
         BOOST_SPIRIT_DEBUG_RULE(nmtoken);
         BOOST_SPIRIT_DEBUG_RULE(pi);
         BOOST_SPIRIT_DEBUG_RULE(prolog);
         BOOST_SPIRIT_DEBUG_RULE(ws);
         BOOST_SPIRIT_DEBUG_RULE(xmldecl);
      }

      rule<ScannerT> attribute,
                     attrib_value,
                     cdata_sect,
                     cdata,
                     char_data,
                     char_data_char,
                     comment,
                     document,
                     doctypedecl,
                     element,
                     elem_content,
                     misc,
                     name,
                     name_char,
                     names,
                     nmtoken,
                     pi,
                     prolog,
                     ws,
                     xmldecl
                     ;

      rule<ScannerT> const& start() const
      { return document; }
   };
};


/**
 * The actual parser class that we will use.
 *
 * Provides a simple interface for using the builder and grammar.
 */
class Parser
{
public:
   /** Parse document coming in from string. */
   void parseDocument(cppdom::Document& doc, std::string& content);

   /** Parse document from istream input. */
   void parseDocument(cppdom::Document& doc, std::istream& instream);

public:
   XmlBuilder<const char*>  mCharBuilder;   /**< The builder that we are using. */
   //XmlBuilder<const char*>  mCharBuilder;   /**< The builder that we are using. */

};

}  // namespace spirit


} // namespace cppdom


#endif

