/*
   xmlpp - an xml parser and validator written in C++
   (c) 2000 Michael Fink
   (c) Karl Pitrich
      
   $Id$
*/

/*! \file xmltokenizer.h

  the stream tokenizer class

*/

#ifndef __xmltokenizer_h__
#define __xmltokenizer_h__

#include <string>
#include <stack>
#include <iosfwd>
#include "xmlhelpers.h"

// namespace declaration
namespace xmlpp {

//! xml token
/*! an xmltoken is a representation for a literal character or a 
    generic string (not recognized as a literal) */
class xmltoken
{
   friend class xmlstream_iterator;
public:
   //! ctor
   xmltoken(){ isliteral=true; literal=0; generic.empty(); }
   //! ctor with init
   xmltoken( char ch ){ isliteral=true; literal=ch; generic.empty(); }
   //! ctor with init
   xmltoken( string &str ):generic(str){ isliteral=false; literal=0; }

   //! returns if token is a literal
   bool is_literal(){ return isliteral; }
   //! returns if it is and end of xml stream token
   bool is_endofstream(){ return isliteral && literal==EOF/*xmlstring::traits_type::eof()*/; }
   //! returns literal char
   char get_literal(){ return literal; }
   //! returns generic string
   string &get_generic(){ return generic; }

   // operators

   //! compare operator for literals
   bool operator ==(char ch){ return !isliteral?false:ch==literal; }
   //! compare operator for literals
   bool operator !=(char ch){ return !isliteral?true:ch!=literal; }

   //! compare operator for a generic string
   bool operator ==(string str){ return !isliteral?str==generic:false; }
   //! compare operator for a generic string
   bool operator !=(string str){ return !isliteral?str!=generic:true; }

   //! set generic string
   xmltoken &operator =(string &str){ generic.assign(str); isliteral=false; return *this; }
   //! set literal char
   xmltoken &operator =(char ch){ literal=ch; isliteral=true; return *this; }

protected:
   //! indicates if token is a literal char
   bool isliteral;
   //! literal
   char literal;
   //! pointer to string
   string generic;
};


//! base tokenizer class
/*! base class for iterating through xmltoken */
class xmltokenizer
{
public:
   //! ctor
   xmltokenizer(std::istream &is,xmllocation &loc):instr(is),location(loc){}
   virtual ~xmltokenizer(){}

   //! dereference operator
   xmltoken& operator*(){ return curtoken; }
   //! pointer access operator
   const xmltoken* operator->(){ return &curtoken; }
   //! advances in the xml stream
   xmltokenizer &operator++(){ get_next(); return *this; }
   //! advances in the xml stream
   xmltokenizer &operator++(int){ get_next(); return *this; }

   //! returns current token
   xmltoken& get(){ return curtoken; }

   //! puts the token back into the stream
   void put_back( xmltoken &token ){ tokenstack.push(token); }

   //! puts the last token back into the stream
   void put_back(){ tokenstack.push(curtoken); }

protected:

   //! internal: parses the next token
   virtual void get_next() = 0;

   // data members

   //! input stream
   std::istream &instr;

   //! location in the stream
   xmllocation &location;

   //! current token
   xmltoken curtoken;

   //! stack for put_back()'ed tokens
   std::stack<xmltoken> tokenstack;
};

//! xml input stream iterator
/*! an iterator through all xmltoken contained in the xml input stream */
class xmlstream_iterator:public xmltokenizer
{
public:
   //! ctor
   xmlstream_iterator(std::istream &is,xmllocation &loc);

   //! set to true to ignore parsing whitespace
   void set_cdata_mode(bool to=false) { cdata_mode=to; }
   
protected:
   void get_next();

   // internally used to recognize chars in the stream
   bool is_literal( char c );
   bool is_whitespace( char c );
   bool is_newline( char c );
   bool is_stringdelimiter( char c ); // start-/endchar of a string

   //! cdata-mode doesn't care for whitespaces in generic strings
   bool cdata_mode;

   //! char which was put back internally
   char putback_char;
};

//! dtd input stream iterator
/*! an iterator through a dtd input stream */
class xmldtd_iterator:public xmltokenizer
{
public:
   //! ctor
   xmldtd_iterator(std::istream &is,xmllocation &loc);

protected:
   void get_next(){}
};


}; // namespace end

#endif
/* vi: set ts=3: */
