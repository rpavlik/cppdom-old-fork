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
class XmlBuilder
{
public:
   /** Constructe an XmlBuilder and init for document. */
   //XmlBuilder(cppdom::DocumentPtr doc, cppdom::ContextPtr context=cppdom::ContextPtr());

   /** Prepare for parsing a new document. */
   void reinit(cppdom::Document* doc, cppdom::ContextPtr context=cppdom::ContextPtr());

   /** Call when parser is done. */
   void finish();

   void startElement(char const* first, char const* last);
   void endElement(char const* first, char const* last);
   void startAttribute(char const* first, char const* last);
   void attribValue(char const* first, char const* last);
   void elementText(char const* first, char const* last);

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
* type: BUILDER_T must implement the interface concept similar to XmlBuilder above.
*/
template<typename BUILDER_T=XmlBuilder>
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
                        (str_p("/>"))[boost::bind(&BUILDER_T::endElement,self.mBuilder,_1,_2)] );

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


/** The actual parser class that we will use.
 *
 * Provides a simple interface for using the builder and grammar.
 */
class Parser
{
public:
   void parseDocument(cppdom::Document& doc, std::string& content)
   {
      mBuilder.reinit(&doc, doc.getContext());
      XmlGrammar<XmlBuilder> xml_grammar(&mBuilder);
      parse_info<char const*> result;

      result = bs::parse(content.c_str(), xml_grammar);
      if(!result.full)
      {
         throw CPPDOM_ERROR(xml_invalid_operation, "Invalid format of XML.");
      }
   }

   void parseDocument(cppdom::Document& doc, std::istream& instream)
   {

       /*
       typedef char char_t;
       typedef multi_pass<istreambuf_iterator<char_t> > iterator_t;

       typedef skip_parser_iteration_policy<space_parser> iter_policy_t;
       typedef scanner_policies<iter_policy_t> scanner_policies_t;
       typedef scanner<iterator_t, scanner_policies_t> scanner_t;

       typedef rule<scanner_t> rule_t;

       iter_policy_t iter_policy(space_p);
       scanner_policies_t policies(iter_policy);
       */
      typedef bs::multi_pass<std::istreambuf_iterator<char> > iterator_t;

      mBuilder.reinit(&doc,doc.getContext());

      XmlGrammar<XmlBuilder> xml_grammar(&mBuilder);
      parse_info<iterator_t> result;

      std::istreambuf_iterator<char> first(instream);
      std::istreambuf_iterator<char> last;

      std::string test(first,last);
      parseDocument(doc,test);

      /*
      result = bs::parse(bs::make_multi_pass(first), bs::make_multi_pass(last), xml_grammar);
      if(!result.full)
      {
         throw CPPDOM_ERROR(xml_invalid_operation, "Invalid format of XML.");
      }
      */
   }

public:
   XmlBuilder  mBuilder;   /**< The builder that we are using. */

};

}  // namespace spirit


} // namespace cppdom


#endif

