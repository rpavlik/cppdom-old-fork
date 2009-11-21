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
/*! \file xmltokenizer.cpp

  contains the xml stream tokenizer

*/

// needed includes
#include <cstdio>
#include "cppdom.h"
#include "xmltokenizer.h"


// namespace declaration
namespace cppdom
{
   // Token methods
   Token::Token()
      : mIsLiteral(true)
      , mLiteral(0)
   {}

   Token::Token(char ch)
      : mIsLiteral(true)
      , mLiteral(ch)
   {}

   Token::Token(const std::string& str)
      : mIsLiteral(false)
      , mLiteral(0)
      , mGeneric(str)
   {}

   bool Token::isLiteral() const
   {
      return mIsLiteral;
   }

   bool Token::isEndOfStream() const
   {
      return mIsLiteral && mLiteral == char(EOF);
   }

   char Token::getLiteral() const
   {
      return mLiteral;
   }

   const std::string& Token::getGeneric() const
   {
      return mGeneric;
   }

   bool Token::operator==(char ch) const
   {
      return !isLiteral() ? false : ch == mLiteral;
   }

   bool Token::operator!=(char ch) const
   {
      return ! operator==(ch);
   }

   bool Token::operator==(const std::string& str) const
   {
      return !isLiteral() ? str == mGeneric : false;
   }

   bool Token::operator!=(const std::string& str) const
   {
      return ! operator==(str);
   }

   Token& Token::operator=(const std::string& str)
   {
      mGeneric = str;
      mIsLiteral = false;
      return *this;
   }

   Token& Token::operator=(char ch)
   {
      mLiteral = ch;
      mIsLiteral = true;
      return *this;
   }

   // Tokenizer methods

   Tokenizer::Tokenizer(std::istream& in, Location& loc)
      : mInput(in), mLocation(loc)
   {}

   Tokenizer::~Tokenizer()
   {}

   Token& Tokenizer::operator*()
   {
      return mCurToken;
   }

   const Token* Tokenizer::operator->()
   {
      return &mCurToken;
   }

   Tokenizer& Tokenizer::operator++()
   {
      getNext();
      return *this;
   }

   Tokenizer& Tokenizer::operator++(int)
   {
      getNext();
      return *this;
   }

   Token& Tokenizer::get()
   {
      return mCurToken;
   }

   void Tokenizer::putBack(Token& token)
   {
      mTokenStack.push(token);
   }

   void Tokenizer::putBack()
   {
      mTokenStack.push(mCurToken);
   }

   // xmlstream_iterator methods
   xmlstream_iterator::xmlstream_iterator(std::istream& in, Location& loc)
      : Tokenizer(in, loc)
      , mCdataMode(false)
      , mPutbackChar(-1)
   {}

   /** \todo check for instr.eof() */
   void xmlstream_iterator::getNext()
   {
      // first use the token stack if filled
      if (mTokenStack.size() != 0)
      {
         // get the token from the stack and return it
         Token tok;
         mCurToken = mTokenStack.top();
         mTokenStack.pop();

         return;
      }

      bool finished = false;

      std::string generic;

      // get next char
      char c;

      do
      {
         if (mPutbackChar == char(-1))
         {
            c = mInput.get();
            mLocation.step();
         }
         else
         {
            c = mPutbackChar;
            mPutbackChar = char(-1);
            mLocation.step();
         }

         // do we have an eof?
         // TODO: check for instr.eof()
         if (c == char(EOF))
         {
            if (generic.length() != 0)
            {
               mCurToken = c;
               return;
            }
            else
            {
               break;
            }
         }

         // is it a literal?
         if (isLiteral(c))
         {
            mCdataMode = false;
            if (generic.length() == 0)
            {
               mCurToken = c;

               // quick fix for removing set_cdataMode() functionality
               if (c == '>')
               {
                  mCdataMode = true;
               }

               return;
            }
            mPutbackChar = c;
            mLocation.step(-1);
            break;
         }

         // a string delimiter and not in cdata mode?
         if (isStringDelimiter(c) && !mCdataMode)
         {
            generic = c;
            char delim = c;
            do
            {
               c = mInput.get();
               mLocation.step();
               if (c == char(EOF))
               {
                  break;
               }
               generic += c;
            }
            while (c != delim);
            break;
         }

         // a whitespace?
         if (isWhiteSpace(c))
         {
            if (generic.length() == 0)
            {
               continue;
            }
            else
            {
               if (!mCdataMode)
               {
                  break;
               }
            }
         }

         // a newline char?
         if (isNewLine(c) )
         {
            if (mCdataMode && generic.length() != 0)
            {
               c = ' ';
            }
            else
            {
               continue;
            }
         }

         // add to generic string
         generic += c;
      }
      while (!finished);

      // set the generic string
      mCurToken = generic;
   }

   // returns if we have a literal char
   bool xmlstream_iterator::isLiteral(char c)
   {
      switch(c)
      {
      case '?':
      case '=':
      case '!':
      case '/':
         if (mCdataMode)
         {
            return false;
         }
      case '<':
      case '>':
         return true;
      }
      return false;
   }

   // returns if we have a white space char
   bool xmlstream_iterator::isWhiteSpace(char c)
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
   bool xmlstream_iterator::isNewLine(char c)
   {
      switch(c)
      {
      case '\n':
         mLocation.newline();
      case '\r':
         return true;
      }
      return false;
   }

   // returns if we have a string delimiter (separating " and ')
   bool xmlstream_iterator::isStringDelimiter(char c)
   {
      switch(c)
      {
      case '\"':
      case '\'':
         return true;
      }
      return false;
   }
}
