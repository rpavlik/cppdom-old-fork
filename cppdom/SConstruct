#!python
try:
   import wing.wingdbstub;       # stuff for debugging
except:
   pass
   
import os, string, sys, re
pj = os.path.join

Default('.')

# Bring in the AutoDist build helper
sys.path.append('tools/build')

import SCons.Environment
import SCons

#------------------------------------------------------------------------------
# Define some generally useful functions
#------------------------------------------------------------------------------
def GetCppDomVersion():
   "Gets the CppDom version from cppdom/version.h"
   import re

   contents = open('cppdom/version.h', 'r').read()
   major = re.compile('.*(#define *CPPDOM_VERSION_MAJOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   minor = re.compile('.*(#define *CPPDOM_VERSION_MINOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   patch = re.compile('.*(#define *CPPDOM_VERSION_PATCH *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   return (int(major), int(minor), int(patch))

def GetPlatform():
   "Determines what platform this build is being run on."
   if string.find(sys.platform, 'irix') != -1:
      return 'irix'
   elif string.find(sys.platform, 'linux') != -1:
      return 'linux'
   elif string.find(sys.platform, 'freebsd') != -1:
      return 'linux'
   elif string.find(sys.platform, 'cygwin') != -1:
      return 'win32'
   elif string.find(os.name, 'win32') != -1:
      return 'win32'
   elif string.find(sys.platform, 'sun') != -1:
      return 'sun'
   else:
      return sys.platform
Export('GetPlatform')

def CreateConfig(target, source, env):
   "Creates the prefix-config file users use to compile against this library"
   import re

   targets = map(lambda x: str(x), target)
   sources = map(lambda x: str(x), source)

   submap = env['submap']

   # Build each target from its source
   for i in range(len(targets)):
      print "Generating config file " + targets[i]
      contents = open(sources[i], 'r').read()

      # Go through the substitution dictionary and modify the contents read in
      # from the source file
      for key, value in submap.items():
         contents = re.sub(re.escape(key), value, contents)

      # Write out the target file with the new contents
      open(targets[0], 'w').write(contents)
      os.chmod(targets[0], 0755)
   return 0

def BuildLinuxEnvironment():
   "Builds a base environment for other modules to build on set up for linux"
   global optimize, profile, builders

   CXX = os.path.basename(WhereIs('g++3') or 'g++')
   LINK = CXX
   CXXFLAGS = ['-Wall']
   LINKFLAGS = []

   # Enable profiling?
   if profile != 'no':
      CXXFLAGS.extend(['-pg'])
      LINKFLAGS.extend(['-pg'])

   # Debug or optimize build?
   if optimize != 'no':
      CXXFLAGS.extend(['-DNDEBUG', '-O2'])
   else:
      CXXFLAGS.extend(['-D_DEBUG', '-g'])

   return Environment(
      ENV         = os.environ,
      CXX         = CXX,
      CXXFLAGS    = CXXFLAGS,
      LINK        = LINK,
      LINKFLAGS   = LINKFLAGS
   )

def BuildIRIXEnvironment():
   "Builds a base environment for other modules to build on set up for IRIX"
   global optimize, profile, builders

   CXX = 'CC'
   LINK = CXX
   CXXFLAGS = ['-n32', '-mips3', '-LANG:std', '-w2']
   LINKFLAGS = CXXFLAGS

   # Enable profiling?
   if profile != 'no':
      CXXFLAGS.extend([])
      LINKFLAGS.extend([])

   # Debug or optimize build?
   if optimize != 'no':
      CXXFLAGS.extend(['-DNDEBUG', '-O2'])
   else:
      CXXFLAGS.extend(['-D_DEBUG', '-g', '-gslim'])
   # IRIX sucks; no environment variable to specify additional header paths
   CPPPATH = ''
   if os.environ.has_key('CPLUS_INCLUDE_PATH'):
      path = os.environ['CPLUS_INCLUDE_PATH']
      CPPPATH = string.split(path, ':')
      vjbasedir = os.environ['VJ_BASE_DIR']
      CPPPATH.append(pj(vjbasedir, 'include', 'boost', 'compatibility', 'cpp_c_headers'))
      return Environment( 
         ENV = os.environ,
         CXX = CXX,
         CXXFLAGS = CXXFLAGS,
         CPPPATH = CPPPATH,
         LINK = LINK,
         LINKFLAGS = LINKFLAGS
       )
   return Environment(
      ENV         = os.environ,
      CXX         = CXX,
      CXXFLAGS    = CXXFLAGS,
      LINK        = LINK,
      LINKFLAGS   = LINKFLAGS
   )

def BuildSunEnvironment():
   "Builds a base environment for other modules to build on set up for linux"
   global optimize, profile, builders

   CXXFLAGS = ['-Wall', '-fexceptions']
   LINKFLAGS = []

   # Enable profiling?
   if profile != 'no':
      CXXFLAGS.extend(['-pg'])
      LINKFLAGS.extend(['-pg'])

   # Debug or optimize build?
   if optimize != 'no':
      CXXFLAGS.extend(['-DNDEBUG', '-O2'])
   else:
      CXXFLAGS.extend(['-D_DEBUG', '-g'])

   ret_env = Environment( ENV = os.environ )
   
   # Override the tool chains used
   for t in ['gcc', 'gnulink', 'g++']:
      Tool(t)(ret_env)    
   
   ret_env['CXXFLAGS'] = CXXFLAGS;
   return ret_env;

def BuildWin32Environment():
   return Environment(ENV = os.environ)

def HasCppUnit(env):
   "Tests if the user has CppUnit available"
   sys.stdout.write('checking for cppunit... ')
   if env['WithCppUnit'] == None:
      found = 0
   else:
      cfg = os.path.join(env['WithCppUnit'], 'bin', 'cppunit-config')
      found = os.path.isfile(cfg)

   if found:
      sys.stdout.write('yes\n')
   else:
      sys.stdout.write('no\n')
   return found
Export('HasCppUnit')

def SetupCppUnit(env):
   "Sets up env for CppUnit"
   if not HasCppUnit(env):
      print 'ERROR: Could not find CppUnit installation.'
      sys.exit(1)
   cfg = pj(env['WithCppUnit'], 'bin', 'cppunit-config')
   env.ParseConfig(cfg + ' --cflags --libs')
Export('SetupCppUnit')

def ValidateBoostOption(key, value, environ):
   "Validate the boost option settings"
   req_boost_version = 103100;
   sys.stdout.write("checking for %s [%s]..." % (key, value));
   environ["BoostAvailable"] = False

   if "BoostIncludeDir" == key:
      # Get the boost version
      boost_ver_filename = pj(value, 'boost', 'version.hpp');
      if not os.path.isfile(boost_ver_filename):
         print "[%s] not found. Boost not available." % boost_ver_filename
         return False;
      ver_file = file(boost_ver_filename);
      ver_num = int(re.search("define\s+?BOOST_VERSION\s+?(\d*)", ver_file.read()).group(1));   # Matches 103000
      sys.stdout.write("found version: %s\n" % ver_num);
      if ver_num < req_boost_version:
         print "   Boost version to old: required version:%s\n" % req_boost_version;
         return False;
      
      # Check on the libraries that I need to use
#      boost_fs_lib_name = pj(value, 'lib', 'libboost_filesystem.a');
#      if not os.path.isfile(boost_fs_lib_name):
#         print "[%s] not found."%boost_fs_lib_name;
#         Exit();
#         return False;
      # Add the boost stuff to the environment
      environ.Append(BoostCPPPATH = [value,]);
      environ["BoostAvailable"] = True

   elif "BoostLibDir" == key:
      environ.Append(BoostLIBPATH = [value,])

   else:
      assert False, "Invalid boost key";

def AddBoostOptions(opts):
   opts.Add('BoostIncludeDir',
           help = 'Boost header path [only required for spirit parsing]: default: "/usr/local"',
           finder = '/usr/local',
           validator=ValidateBoostOption);
#   opts.Add('BoostLibDir',
#           help = 'Boost lib directory.": default: BoostDir="/usr/local/lib"',
#           finder = '/usr/local/lib',
#           validator=ValidateBoostOption);

#------------------------------------------------------------------------------
# Grok the arguments to this build
#------------------------------------------------------------------------------
EnsureSConsVersion(0,94)

# Figure out what vesion of CppDom we're using
CPPDOM_VERSION = GetCppDomVersion()
print 'Building CppDom Version: %i.%i.%i' % CPPDOM_VERSION

# Get command-line arguments
optimize = ARGUMENTS.get('optimize', 'no')
profile = ARGUMENTS.get('profile', 'no')

# Create the extra builders
# Define a builder for the cppdom-config script
builders = {
   'ConfigBuilder'   : Builder(action = CreateConfig)
}

# Create and export the base environment
if GetPlatform() == 'irix':
   baseEnv = BuildIRIXEnvironment()
elif GetPlatform() == 'linux' or GetPlatform() == 'freebsd':
   baseEnv = BuildLinuxEnvironment()
elif GetPlatform() == 'win32':
   baseEnv = BuildWin32Environment()
elif GetPlatform() == 'sun':
   baseEnv = BuildSunEnvironment()
else:
   print 'Unsupported build environment: ' + GetPlatform()
   print 'Attempting to use standard SCons toolchains.'
   baseEnv = Environment()
Export('baseEnv')

# Do we have the super cool savable version
opts = Options('config.cache')
opts.Add('WithCppUnit',
         'CppUnit installation directory',
         '/usr/local',
         lambda k,v,env=None: WhereIs(pj(v, 'bin', 'cppunit-config')) != None
        )
AddBoostOptions(opts)        
opts.Add('prefix',
         'Installation prefix',
         '/usr/local')
opts.Add('StaticOnly', 'If not "no" then build only static library', 'no')
  
help_text = """--- CppDom Build system ---
Targets:
   install - Install this puppy
      ex: 'scons install prefix=$HOME/software' to install in your account
   Type 'scons' to just build it
 
"""

help_text = help_text + """Options:
   optimize=yes    Generate optimized code.
   profile=yes     Turn on generation of profiling code.
   
"""

help_text = help_text + opts.GenerateHelpText(baseEnv)
#help_text += "Options:\n" + opts.GenerateHelpText(baseEnv)
Help(help_text)

# If we are running the build
if not SCons.Script.options.help_msg:  
   opts.Update(baseEnv)

   # Try to save the options if possible
   try:
      opts.Save('config.cache', baseEnv);
   except LookupError, le:
      pass

   # Handle options
   PREFIX = baseEnv['prefix']
   PREFIX = os.path.abspath(PREFIX)
   Export('PREFIX')

   # Update environment for options   
   if baseEnv["BoostAvailable"]:
      baseEnv.Append(CPPPATH = baseEnv["BoostCPPPATH"]);

   tar_sources = Split("""
	 	  AUTHORS
                  ChangeLog
		  COPYING
		  README
		  cppdom-config.in
		  SConstruct
		  doc/cppdom.doxy
		  doc/dox/examples_index.dox
		  doc/dox/mainpage.dox
		  tools/build/AutoDist.py
		  vc7/cppdom.sln
		  vc7/cppdom.vcproj
		  cppdom/
		  test/
   """)
   baseEnv.Append(TARFLAGS = '-z',)
   baseEnv.Tar('cppdom-' + '%i.%i.%i' % CPPDOM_VERSION + '.tar.gz', tar_sources)

   # Build in a build directory
   buildDir = "build." + GetPlatform()
   Export('buildDir')

#   BuildDir(pj(buildDir, 'test'), 'test', duplicate=0)
#   BuildDir(pj(buildDir, 'cppdom'), 'cppdom', duplicate = 0)
   # Process subdirectories
   subdirs = Split('cppdom test')

   for d in subdirs:
      SConscript(pj(d,'SConscript'), build_dir=pj(buildDir, d), duplicate=0)

   # Setup the builder for cppdom-config
   env = baseEnv.Copy(BUILDERS = builders)
   cppdom_config  = env.ConfigBuilder('cppdom-config', 'cppdom-config.in',
      submap = {
         '@prefix@'                    : PREFIX,
         '@exec_prefix@'               : '${prefix}',
         '@cppdom_cxxflags@'           : '',
         '@includedir@'                : pj(PREFIX, 'include'),
         '@cppdom_extra_cxxflags@'     : '',
         '@cppdom_extra_include_dirs@' : '',
         '@cppdom_libs@'               : '-lcppdom',
         '@libdir@'                    : pj(PREFIX, 'lib'),
         '@VERSION_MAJOR@'             : str(CPPDOM_VERSION[0]),
         '@VERSION_MINOR@'             : str(CPPDOM_VERSION[1]),
         '@VERSION_PATCH@'             : str(CPPDOM_VERSION[2]),
      }
   )

   env.Depends('cppdom-config', 'cppdom/version.h')
   env.Install(pj(PREFIX, 'bin'), cppdom_config)
   env.Alias('install', PREFIX)
