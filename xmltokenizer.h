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
/*! \file XMLTokenizer.hpp

  the stream tokenizer class

*/

// prevent multiple includes
#ifndef CPPDOM_XML_TOKENIZER_H
#define CPPDOM_XML_TOKENIZER_H

// needed includes
#include <string>
#include <stack>
#include <iosfwd>


// namespace declaration
namespace cppdom
{
   /// xml token
   /** an XMLToken is a representation for a literal character or a
       generic string (not recognized as a literal) */
   class XMLToken
   {
      friend class xmlstream_iterator;
   public:
      /// ctor
      XMLToken()
      {
         isliteral = true;
         literal = 0;
         generic.empty();
      }

      /// ctor with init
      XMLToken(char ch)
      {
         isliteral = true;
         literal = ch;
         generic.empty();
      }

      /// ctor with init
      XMLToken(std::string& str)
         : generic(str)
      {
         isliteral = false;
         literal = 0;
      }

      /// returns if token is a literal
      bool isLiteral()
      {
         return isliteral;
      }

      /// returns if it is and end of xml stream token
      bool isEndOfStream()
      {
         return isliteral && literal==char(EOF)/*std::string::traits_type::eof()*/;
      }

      /// returns literal char
      char getLiteral()
      {
         return literal;
      }

      /// returns generic string
      std::string& getGeneric()
      {
         return generic;
      }

      // operators

      /// compare operator for literals
      bool operator==(char ch)
      {
        return !isliteral ? false : ch == literal;
      }

      /// compare operator for literals
      bool operator!=(char ch)
      {
         return !isliteral ? true : ch != literal;
      }

      /// compare operator for a generic string
      bool operator==(std::string str)
      {
         return !isliteral ? str == generic : false;
      }

      /// compare operator for a generic string
      bool operator!=(std::string str)
      {
         return !isliteral ? str != generic : true;
      }

      /// set generic string
      XMLToken& operator=(std::string& str)
      {
         generic.assign(str);
         isliteral = false;
         return *this;
      }

      /// set literal char
      XMLToken& operator=(char ch)
      {
         literal = ch;
         isliteral = true;
         return *this;
      }

   protected:
      /// indicates if token is a literal char
      bool isliteral;

      /// literal
      char literal;

      /// pointer to string
      std::string generic;
   };


   /// base tokenizer class
   /** base class for iterating through XMLToken */
   class XMLTokenizer
   {
   public:
      /** constructor */
      XMLTokenizer(std::istream& is, XMLLocation& loc)
         : instr(is), location(loc)
      {
      }

      virtual ~XMLTokenizer()
      {
      }

      /// dereference operator
      XMLToken& operator*()
      {
         return curtoken;
      }

      /// pointer access operator
      const XMLToken* operator->()
      {
         return &curtoken;
      }

      /// advances in the xml stream
      XMLTokenizer& operator++()
      {
         this->getNext();
         return *this;
      }

      /// advances in the xml stream
      XMLTokenizer& operator++(int)
      {
         this->getNext();
         return *this;
      }

      /// returns current token
      XMLToken& get()
      {
         return curtoken;
      }

      /// puts the token back into the stream
      void putBack(XMLToken& token)
      {
         tokenstack.push(token);
      }

      /// puts the last token back into the stream
      void putBack()
      {
         tokenstack.push(curtoken);
      }

   protected:

      /// internal: parses the next token
      virtual void getNext() = 0;

      // data members

      /** input stream */
      std::istream& instr;

      /** location in the stream */
      XMLLocation& location;

      /** current token */
      XMLToken curtoken;

      /** stack for put_back()'ed tokens */
      std::stack<XMLToken> tokenstack;
   };

   /**
    * xml input stream iterator
    * an iterator through all XMLToken contained in the xml input stream
    */
   class xmlstream_iterator : public XMLTokenizer
   {
   public:
      /** ctor */
      xmlstream_iterator(std::istream& is, XMLLocation& loc);

   protected:
      void getNext();

      // internally used to recognize chars in the stream
      bool isLiteral(char c);
      bool isWhiteSpace(char c);
      bool isNewLine(char c);
      bool isStringDelimiter(char c); // start-/endchar of a string

      /** cdata-mode doesn't care for whitespaces in generic strings */
      bool cdataMode;

      /** char which was put back internally */
      char putbackChar;
   };

   /**
    * dtd input stream iterator
    * an iterator through a dtd input stream
    */
   class xmldtd_iterator: public XMLTokenizer
   {
   public:
      /** ctor */
      xmldtd_iterator(std::istream& is, XMLLocation& loc);

   protected:
      void getNext(){}
   };
}

#endif
