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
/*! \file xmltokenizer.cpp

  contains the xml stream tokenizer

*/

// needed includes
#include <xmlpp/xmlpp.h>
#include <xmlpp/xmltokenizer.h>


// namespace declaration
namespace xmlpp {


// xmlstream_iterator methods

xmlstream_iterator::xmlstream_iterator(std::istream &is,xmllocation &loc)
:xmltokenizer(is,loc)
{
   putback_char = char(-1);
   cdata_mode = false;
}

//! \todo check for instr.eof()
void xmlstream_iterator::get_next()
{
   // first use the token stack if filled
   if (tokenstack.size() != 0)
   {
      // get the token from the stack and return it
      xmltoken tok;
      curtoken = tokenstack.top();
      tokenstack.pop();

      return;
   }

   bool finished = false;

   xmlstring generic;

   // get next char
   xml_char_type c;

   do
   {
      if (putback_char == char(-1) )
      {
         c = instr.get();
         location.step();
      }
      else
      {
         c = putback_char;
         putback_char = char(-1);
         location.step();
      }

      // do we have an eof?
      // TODO: check for instr.eof()
      if (c == char(EOF))
      {
         if (generic.length()!=0)
         {
            curtoken = c;
            return;
         }
         else
            break;
      }

      // is it a literal?
      if (is_literal(c))
      {
         cdata_mode = false;
         if (generic.length()==0)
         {
            curtoken = c;

            // quick fix for removing set_cdata_mode() functionality
            if (c=='>')
               cdata_mode = true;

            return;
         }
         putback_char = c;
         location.step(-1);
         break;
      }

      // a string delimiter and not in cdata mode?
      if (is_stringdelimiter(c) && !cdata_mode)
      {
         generic = c;
         xml_char_type delim = c;
         do
         {
            c = instr.get();
            location.step();
            if (c==char(EOF))
               break;
            generic += c;
         } while (c != delim);
         break;
      }

      // a whitespace?
      if (is_whitespace(c))
      {
         if (generic.length()==0)
            continue;
         else
            if (!cdata_mode)
               break;
      }

      // a newline char?
      if (is_newline(c) )
      {
         if (cdata_mode && generic.length()!=0)
            c = ' ';
         else
            continue;
      }

      // add to generic string
      generic += c;
   }
   while (!finished);

   // set the generic string
   curtoken = generic;
}

// returns if we have a literal char
bool xmlstream_iterator::is_literal( xml_char_type c )
{
   switch(c)
   {
   case '?':
   case '=':
   case '!':
   case '/':
      if (cdata_mode)
         return false;
   case '<':
   case '>':
      return true;
   }
   return false;
}

// returns if we have a white space char
bool xmlstream_iterator::is_whitespace( xml_char_type c )
{
   switch(c)
   {
   case ' ':
   case '\t':
      return true;
   }
   return false;
}

// returns if we have a newline
bool xmlstream_iterator::is_newline( xml_char_type c )
{
   switch(c)
   {
   case '\n':
      location.newline();
   case '\r':
      return true;
   }
   return false;
}

// returns if we have a string delimiter (separating " and ')
bool xmlstream_iterator::is_stringdelimiter( xml_char_type c )
{
   switch(c)
   {
   case '\"':
   case '\'':
      return true;
   }
   return false;
}

// end namespace
}
