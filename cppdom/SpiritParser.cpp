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
/** \file SpiritParser.cpp
  definitions for the spirit parsing classes
*/
#include <cppdom/SpiritParser.h>

namespace cppdom
{

namespace spirit
{
// Constructe an XmlBuilder
/*
XmlBuilder::XmlBuilder(cppdom::DocumentPtr doc, cppdom::ContextPtr context)
{
   reinit(doc,context);
}
*/

// Prepare for parsing a new document
void XmlBuilder::reinit(cppdom::Document* doc, cppdom::ContextPtr context)
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

void XmlBuilder::finish()
{
   assert(mNodeStack.size() == 1);     // Should only have one node left now
}


void XmlBuilder::startElement(char const* first, char const* last)
{
   std::string elt_name(first,last);
   std::cout << "Elt in: [" << elt_name << "]" << std::endl;

   // - Create node
   // - Add to tree
   // - Put on stack to get ready for more
   NodePtr new_elt(new cppdom::Node(elt_name,mContext));
   mNodeStack.back()->addChild(new_elt);
   mNodeStack.push_back(new_elt.get());
}

void XmlBuilder::endElement(char const* first, char const* last)
{
   std::cout << "Elt exit: " << std::string(first,last) << std::endl;
   // Should always have more then one on stack since doc root is
   // on stack at all times and we need one more to pop
   assert(mNodeStack.size() > 1);
   mNodeStack.pop_back();

#ifdef _DEBUG
   mCurAttribute.clear();     // If debug version, clear attribute so we detect problems
#endif
}

void XmlBuilder::startAttribute(char const* first, char const* last)
{
   std::cout << "  attrib: " << std::string(first,last);
   mCurAttribute = std::string(first,last);
   assert(!mCurAttribute.empty());
}

void XmlBuilder::attribValue(char const* first, char const* last)
{
   std::cout << " [" << std::string(first,last) << "] " << std::endl;
   assert(!mCurAttribute.empty());
   std::string attrib_value(first,last);
   if(cppdom::textContainsXmlEscaping(attrib_value))
   {  attrib_value = removeXmlEscaping(attrib_value, false); }
   mNodeStack.back()->getAttrMap().set(mCurAttribute, attrib_value);
}

void XmlBuilder::elementText(char const* first, char const* last)
{
   std::string elem_text(first,last);
   if(textContainsXmlEscaping(elem_text))
   {  elem_text = removeXmlEscaping(elem_text, true); }


   std::cout << "   Text: [" << std::string(first,last) << "] " << std::endl;
   if(!elem_text.empty())
   {
      cppdom::NodePtr cdata_node(new cppdom::Node("cdata",mContext));
      cdata_node->setType(Node::xml_nt_cdata);
      cdata_node->setCdata(elem_text);
      mNodeStack.back()->addChild(cdata_node);
   }
}



}  // namespace spirit
} // namespace cppdom

