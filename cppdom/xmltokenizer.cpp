/*
   xmlpp - an xml parser and validator written in C++
   (c) 2000 Michael Fink
   (c) 2001 Karl Pitrich
    
   $Id$
*/

/*! \file xmltokenizer.cpp

  contains the xml stream tokenizer

*/

// needed includes
#include "xmlpp.h"
#include "xmltokenizer.h"


// namespace declaration
namespace xmlpp {


// xmlstream_iterator methods

xmlstream_iterator::xmlstream_iterator(std::istream &is,xmllocation &loc)
:xmltokenizer(is,loc)
{
   putback_char = -1;
   cdata_mode = false;
}

//! \todo check for instr.eof()
void xmlstream_iterator::get_next() {
   // first use the token stack if filled
   if (tokenstack.size() != 0) {
      // get the token from the stack and return it
      xmltoken tok;
      curtoken = tokenstack.top();
      tokenstack.pop();
      return;
   }

   bool finished = false;
   string generic;

   // get next char
   char c;

   do {
      if (putback_char == -1 ) {
         c = instr.get();
         location.step();
      } else {
         c = putback_char;
         putback_char = -1;
         location.step();
      }

      // do we have an eof?
      // TODO: check for instr.eof()
      if (c==EOF) {
         if (generic.length()!=0) {
            curtoken = c;
            return;
         } else {
            break;
			}	
      }

      // is it a literal?
      if (is_literal(c)) {
         cdata_mode = false;
         if (generic.length()==0) {
            curtoken = c;

            // automating set_cdata_mode() functionality
            if (c=='>') cdata_mode = true;
            
				return;
         }
         putback_char = c;
         location.step(-1);
         break;
      }

      // a string delimiter and not in cdata mode?
      if (is_stringdelimiter(c) && !cdata_mode) {
         generic = c;
         char delim = c;
         do {
            c = instr.get();
            location.step();
            if (c==EOF) break;
            generic += c;
         } while (c != delim);
         break;
      }

      // a whitespace?
      if (is_whitespace(c)) {
         if (generic.length()==0) {
            continue;
			} else {
            if (!cdata_mode) break;
			}
      }

      // a newline char?
		if (is_newline(c)) {
			if (cdata_mode && generic.length()!=0) {
				//c = ' ';
			} else {
				continue;
			}
		}

      // add to generic string
      generic += c;
   
	} while (!finished);

   // set the generic string
   curtoken = generic;
}

// returns if we have a literal char
bool xmlstream_iterator::is_literal( char c ) {
   switch(c) {
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
bool xmlstream_iterator::is_whitespace( char c ) {
   switch(c) {
	   case ' ':
	   case '\t':
	      return true;
   }
   return false;
}

// returns if we have a newline
bool xmlstream_iterator::is_newline( char c ) {
   switch(c) {
	   case '\n':
   	   location.newline();
	   case '\r':
	      return true;
   }
   return false;
}

// returns if we have a string delimiter (separating " and ')
bool xmlstream_iterator::is_stringdelimiter( char c ) {
   switch(c) {
	   case '\"':
	   case '\'':
	      return true;
   }
   return false;
}

}; // end namespace

/* vi: set ts=3: */
