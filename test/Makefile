default: all

# Basic options.
srcdir		= .
CFLAGS		=  $(EXTRA_CFLAGS) $(INCLUDES) $(DEFS)
CXXFLAGS	= -Wall -Werror-implicit-function-declaration -Wall -Werror-implicit-function-declaration $(EXTRA_CFLAGS) $(INCLUDES) $(DEFS)
DEFS		= 
EXTRA_CFLAGS	=  $(DEBUG_CFLAGS)
DEBUG_CFLAGS	= -g -D_DEBUG
OPTIM_CFLAGS	= -O2 -fno-strict-aliasing -D_OPT
INCLUDES	= -I$(srcdir) -I$(srcdir)/../..

EXTRA_LFLAGS	=  $(DEBUG_LFLAGS)
DEBUG_LFLAGS	= 
OPTIM_LFLAGS	= 
LINK_FLAGS	=   $(EXTRA_LFLAGS)
LINKALL_ON	= -Wl,--whole-archive
LINKALL_OFF	= -Wl,--no-whole-archive

# Libraries needed for linking.
BASIC_LIBS	= -Wl,-Bstatic $(LINKALL_ON)  $(LINKALL_OFF) -Wl,-Bdynamic
EXTRA_LIBS	= -Wl,-Bdynamic -lm  

# Commands to execute.
C_COMPILE	= gcc $(CFLAGS)
CXX_COMPILE	= c++ $(CXXFLAGS)
LINK		= c++ $(LINK_FLAGS)

VPATH	= $(srcdir):$(srcdir)/..

nodetest_OBJS	= nodetest.o xmlparser.o xmlpp.o xmltokenizer.o
parsetest_OBJS	= parsetest.o xmlparser.o xmlpp.o xmltokenizer.o
predtest_OBJS	= predtest.o xmlparser.o xmlpp.o xmltokenizer.o

# -----------------------------------------------------------------------------
# Application build targets.
# -----------------------------------------------------------------------------
all: nodetest parsetest predtest

nodetest: $(nodetest_OBJS)
	$(LINK) -o $@ $(nodetest_OBJS) $(BASIC_LIBS) $(EXTRA_LIBS)

parsetest: $(parsetest_OBJS)
	$(LINK) -o $@ $(parsetest_OBJS) $(BASIC_LIBS) $(EXTRA_LIBS)

predtest: $(predtest_OBJS)
	$(LINK) -o $@ $(predtest_OBJS) $(BASIC_LIBS) $(EXTRA_LIBS)

# Suffix rules for building object files.
.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX_COMPILE) -o $@ -c $<

# -----------------------------------------------------------------------------
# Clean-up targets.
# -----------------------------------------------------------------------------
clean:
	rm -f Makedepend *.o OpenSGNav.ilk  so_locations *.?db core*
	rm -rf ii_files

clobber:
	@$(MAKE) clean
	rm -f nodetest
