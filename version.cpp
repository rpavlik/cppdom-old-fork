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
#include "version.h"

// These helper macros are used to stringify a given macro
#define CPPDOM_STR(s)             # s
#define CPPDOM_XSTR(s)            CPPDOM_STR(s)

// These helper macros are used to build up the CPPDOM_VERSION_STRING macro.
#define CPPDOM_DOT(a,b)           a ## . ## b
#define CPPDOM_XDOT(a,b)          CPPDOM_DOT(a,b)

//--------------------------------------------------------------------------
// Define the CPPDOM_VERSION_STRING macros
//--------------------------------------------------------------------------

// Create the CPPDOM_VERSION_STRING macro
#define CPPDOM_VERSION_STRING \
   CPPDOM_XDOT( \
      CPPDOM_XDOT(CPPDOM_VERSION_MAJOR, CPPDOM_VERSION_MINOR), \
      CPPDOM_VERSION_PATCH \
   )

namespace cppdom
{
   const char* version = CPPDOM_XSTR(CPPDOM_VERSION_STRING);
}

// Undef all the helper macros
#undef CPPDOM_XSTR
#undef CPPDOM_STR
#undef CPPDOM_DOT
#undef CPPDOM_XDOT

// Undef the CPPDOM_VERSION_STRING temporary macro
#undef CPPDOM_VERSION_STRING

