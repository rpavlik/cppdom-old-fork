/*
   xmlpp - an xml parser and validator written in C++
   (c) 2000 Michael Fink
   (c) 2001 Karl Pitrich
      
   $Id$
*/

/*! \file xmlhelpers.h

  miscelanneous helper methods

*/

#ifndef __xmlhelpers_h__
#define __xmlhelpers_h__

//! namespace of the xmlpp project
namespace xmlpp {

//! xml stream position
/*! represents the position in the xml input stream; usable if load()
    throws an error on parsing xml content */
class XMLPP_API xmllocation {
public:
   //! ctor
   xmllocation() { reset(); }

   //! returns current line
   int get_line() const { return line; }
   //! returns current position in a line
   int get_pos() const { return pos; }
   //! advances a char
   void step( int chars = 1 ) { pos+=chars; }
   //! indicates entering a new line
   void newline() { line++; pos=1; }
   //! reset location
   void reset() { line=pos=1; }

protected:
   int line,pos;
};

}; // end namespace xmlpp

#endif
/* vi: set ts=3: */
