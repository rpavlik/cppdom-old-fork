# =============================================================================
# Basic template makefile for applications.
# $Id$
#
# *** IMPORTANT NOTES -- READ THESE FIRST ***
#     1) This file _requires_ the use of GNU make (gmake).
# =============================================================================

DZR_BASE_DIR=	..
OBJDIR=objs
DEPDIR=deps

# Nothing but comments should come before this line.
default: all

# -----------------------------------------------------------------------------
# Application-specific variable settings.  It is safe to modify these.
# -----------------------------------------------------------------------------

# The application name that is built.  A value must be set for this!
APP_NAME1=	nodetest
APP_NAME2=	parsetest
APP_NAME3=	predtest

# The directory where the source files for this application are located.
srcdir=  	.

# Set a langauge for compiling.  This is optional and will default to C++ if
# nothing is specified.  Currently supported langauges are C and C++.
#LANGUAGE=	

# Compiler flags needed for this application.  They are used as follows:
#
#    DEBUG_APP         - If set to TRUE, this application will be compiled with
#                        debugging symbols and no optimized symbols
#    OPTIM_APP         - If set to TRUE, this application will be compiled with
DEBUG_APP=		TRUE
OPTIM_APP=		FALSE

# The list of all source files (needed for dep generation)
SRCS=	cppdom.cpp xmlparser.cpp xmltokenizer.cpp \
         nodetest.cpp parsetest.cpp predtest.cpp

# source files for each application
SRCS1=   cppdom.cpp xmlparser.cpp xmltokenizer.cpp nodetest.cpp
SRCS2=   cppdom.cpp xmlparser.cpp xmltokenizer.cpp parsetest.cpp
SRCS3=   cppdom.cpp xmlparser.cpp xmltokenizer.cpp predtest.cpp

# This is for extra libraries needed specfically for your application.  The
# general libraries needed for applications are in $(LIBS).
EXTRA_INCLUDES= -I..
EXTRA_LIBS=		

# Extend this as necessary to find source files that are not in the current
# directory.  Set EXTRA_PATH_FOR_SOURCES to all the directories that you
# have sources in. (current dir is already included by default)
EXTRA_PATH_FOR_SOURCES=../cppdom

# Additional files and directories besides the standard ones that need to be
# removed by the 'clean' target.
CLEAN_FILES=		
CLEAN_DIRS=		
CLOBBER_FILES= $(APP_NAME1)$(EXE_EXT) $(APP_NAME2)$(EXE_EXT) $(APP_NAME3)$(EXE_EXT)

# The following include line MUST COME BEFORE the targets and objs for 
# compiling the application.
include $(DZR_BASE_DIR)/mk/dzr.basicapp.mk

# Construct $(OBJS) from $(SRCS) using the makeobjs function.
OBJS1=		$(addprefix $(OBJDIR)/, $(call makeobjs, $(SRCS1)))
OBJS2=		$(addprefix $(OBJDIR)/, $(call makeobjs, $(SRCS2)))
OBJS3=		$(addprefix $(OBJDIR)/, $(call makeobjs, $(SRCS3)))

# -----------------------------------------------------------------------------
# Targets.
# -----------------------------------------------------------------------------
all: $(APP_NAME1)$(EXE_EXT) $(APP_NAME2)$(EXE_EXT) $(APP_NAME3)$(EXE_EXT)

# Target for the application to be built.
$(APP_NAME1)$(EXE_EXT): $(OBJS1)
	$(LINK) $(LINK_OUT)$@ $(OBJS1) $(EXTRA_LIBS) $(LIBS)
$(APP_NAME2)$(EXE_EXT): $(OBJS2)
	$(LINK) $(LINK_OUT)$@ $(OBJS2) $(EXTRA_LIBS) $(LIBS)
$(APP_NAME3)$(EXE_EXT): $(OBJS3)
	$(LINK) $(LINK_OUT)$@ $(OBJS3) $(EXTRA_LIBS) $(LIBS)

