# ************** <auto-copyright.pl BEGIN do not edit this line> **************
# Doozer
#
# Original Authors:
#   Patrick Hartling, Kevin Meinert
# -----------------------------------------------------------------------------
# VR Juggler is (C) Copyright 1998, 1999, 2000, 2001 by Iowa State University
#
# Original Authors:
#   Allen Bierbaum, Christopher Just,
#   Patrick Hartling, Kevin Meinert,
#   Carolina Cruz-Neira, Albert Baker
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# -----------------------------------------------------------------
# File:          $RCSfile$
# Date modified: $Date$
# Version:       $Revision$
# -----------------------------------------------------------------
# *************** <auto-copyright.pl END do not edit this line> ***************

# -----------------------------------------------------------------------------
# Basic application mk file.  This sets all the general variables that will be
# needed to compile and link applications.
#
# $Id$
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# The following is the list of variables that users can set in their makefiles
# to extend the default values:
#
# EXTRA_CFLAGS   - Extra flags for the C compiler.
# EXTRA_CXXFLAGS - Extra flags for the C++ compiler.
# EXTRA_INCLUDES - Extensions to the default include path in the form
#                  -I/some/dir/include.
# EXTRA_LDFLAGS  - Extra flags for the linker.
# EXTRA_SYS_LIBS - Extra system libraries needed for linking.
# -----------------------------------------------------------------------------

# Determine the name of this platform.
include $(DZR_BASE_DIR)/mk/dzr.hosttype.mk

# Basic comipler debugging and optimization flags.
DEBUG_FLAGS?=	$(OS_DEBUG_FLAGS) $(EXTRA_DEBUG_FLAGS)
OPTIM_FLAGS?=	$(OS_OPTIM_FLAGS) $(EXTRA_OPTIM_FLAGS)

DEBUG_LINK_FLAGS?=	$(OS_DEBUG_LINK_FLAGS) $(EXTRA_DEBUG_LINK_FLAGS)
OPTIM_LINK_FLAGS?=	$(OS_OPTIM_LINK_FLAGS) $(EXTRA_OPTIM_LINK_FLAGS)

# Add debugging or optimizing flags to $(EXTRA_CFLAGS) and $(EXTRA_CXXFLAGS)
# depending on whether this application is compiled as a debugging application
# or an optimized application.
ifeq ($(DEBUG_APP), TRUE)
   EXTRA_CFLAGS+=	$(DEBUG_FLAGS)
   EXTRA_CXXFLAGS+=	$(DEBUG_FLAGS)
   EXTRA_LDFLAGS+=	$(DEBUG_LINK_FLAGS)
else
   ifeq ($(OPTIM_APP), TRUE)
     EXTRA_CFLAGS+=	$(OPTIM_FLAGS)
     EXTRA_CXXFLAGS+=	$(OPTIM_FLAGS)
     EXTRA_LDFLAGS+=	$(OPTIM_LINK_FLAGS)
   endif
endif

# Basic compiler flags.  They are as follows:
#
#    CFLAGS   - Standard C compiler flags
#    CXXFLAGS - Standard C++ compiler flags
#    LDFLAGS  - Standard linker flags
#
CFLAGS=		$(OS_ABI_FLAGS) $(OS_CFLAGS) $(INCLUDES) $(EXTRA_CFLAGS)
CXXFLAGS=	$(OS_ABI_FLAGS) $(OS_CXXFLAGS) $(INCLUDES) $(EXTRA_CXXFLAGS)
LDFLAGS=	$(OS_ABI_FLAGS) $(OS_LDFLAGS) $(EXTRA_LDFLAGS)

# The default include path.  We assume that the users will have .h files in
# their source directory.  It should not hurt to assume that unless the user
# is doing something bad.  In that case, it is their responsibility to resolve
# it.
INCLUDES=	-I$(srcdir) $(OS_INCLUDES) $(EXTRA_INCLUDES)

# Wrapper options around the list of libraries needed at link time.  These
# are to be put at the start and end of $(LIBS) respectively when it is set.
LIBS_BEGIN=	$(OS_LIBS_BEGIN)
LIBS_END=	$(OS_LIBS_END)

# Basic libraries needed by various types of applications.  They are as
# follows:
#
#    SYS_LIBS - The system libraries needed by all applications
#
SYS_LIBS=	$(OS_SYS_LIBS) $(EXTRA_SYS_LIBS)

# Command to set the umask used for compiled files.
ifneq ($(UMASK), )
SET_UMASK=	$(UMASK) $(UMASK_VAL) ;
endif

# The compilers and the linker with all their flags.
C_COMPILE=	$(SET_UMASK) $(CC) $(CFLAGS)
CXX_COMPILE=	$(SET_UMASK) $(CXX) $(CXXFLAGS) 
LINK=		$(SET_UMASK) $(LD) $(LDFLAGS)
LINK_OUT=	$(OS_LD_NAME_FLAG)
EXE_EXT=		$(OS_EXE_EXT)
LINKALL_BEGIN = 	$(OS_LINKALL_BEGIN)
LINKALL_END = 	$(OS_LINKALL_END)
