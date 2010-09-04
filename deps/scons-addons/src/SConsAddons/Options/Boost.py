"""SConsAddons.Options.Boost

Definds options for boost project
"""

__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

#!python
# SCons based build file for Boost
# Base file
import SCons.Environment
import SCons
import SConsAddons.Options
import SConsAddons.Util as sca_util
import SCons.Util
import distutils.sysconfig

import string
import sys
import os
import re

pj = os.path.join

from SCons.Util import WhereIs

import SCons.SConf
Configure = SCons.SConf.SConf
# ##############################################
# Options
# ##############################################
class Boost(SConsAddons.Options.PackageOption):
   def __init__(self, name, requiredVersion, 
                useDebug=False, useMt=True, libs=[], 
                required=True, useCppPath=False, 
                toolset="auto", useVersion=False,
                preferDynamic=True, autoLink = False,
                allowLibNameFallbacks=True):
      """
         name - The name to use for this option
         requiredVersion - The version of Boost required (ex: "1.30.0")
         useDebug - Should we use debug boost libraries [default: False]
         useMt - Should we use multi-threaded boost libs [default: True]         
         libs - Boost libraries needed that are actually compiled (base library names. ex: python)
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, then include path is added to CPPPATH if not, then added to CPPFLAGS directly
	 toolset - The toolset name to use (ex: "auto","gcc", "il")
         useVersion - Attempt to use version in the library naming.
         preferDynamic - If true, prefer linking against a dynamic library.in your opinion, should I be looking at boost or at pyopensg on this one?
         autoLink - If true and using msvc, then attempt to use boost's autolinking capabilies.
         allowLibNameFallbacks - If true, then we can fallback on less explicit names for libraries.
      """
      help_text = ["Base directory for Boost. include, and lib should be under this directory.",
                   "Include directory for boost (if not under base)."]
      self.baseDirKey = "BoostBaseDir"
      self.incDirKey = "BoostIncludeDir"
      SConsAddons.Options.PackageOption.__init__(self, name, 
                                                 [self.baseDirKey, self.incDirKey], 
                                                 help_text)      
      self.requiredVersion = requiredVersion
      self.libVersionStr = None
      self.lib_names = libs
      self.required = required
      self.useCppPath = useCppPath
      self.toolset = toolset
      self.preferDynamic = preferDynamic
      self.autoLink = autoLink
      if sca_util.GetPlatform() != "win32":
         self.autoLink = False
      self.available = False            # Track availability
      
      self.found_incs = []
      self.found_incs_as_flags = ""  # The includes as flags to add on command line
      self.found_lib_paths = []
      self.found_libs    = {}        # Name of actual libraries found (with or without toolset, etc)
      self.found_defines = []

      # configurable options
      self.baseDir = None
      self.incDir = None
      self.setupLibrarySettings()
      
      # Options for which libraries to use
      self.use_mt = useMt      
      self.use_debug = useDebug 
      self.use_ver = useVersion
      self.allowLibNameFallbacks = allowLibNameFallbacks
      #print "Use debug set to:", self.use_debug

   def setToolset(self, toolset):
      self.toolset = toolset
   
   def setUseMt(self, bval=True):
      self.use_mt = bval
   
   def setAllowLibNameFallbacks(self, bval=True):
      self.allowLibNameFallbacks = bval
      
   def setUseDebug(self, bval=True):
      self.use_debug = bval

   def setUseVersion(self, useVersion = True):
      self.use_ver = useVersion   

   def setupLibrarySettings(self):
      " Setup some default settings and data that will be used on this platform. "
      # Map from library name to header to check for       
      self.headerMap = { 'program_options':'boost/program_options.hpp',
                         #'python':'boost/python.hpp',
                         'python':'boost/python/enum.hpp',
                         'signals':'boost/signal.hpp',
                         'thread':'boost/thread.hpp',
                         'filesystem':'boost/filesystem/path.hpp' }
      
      # Map for extra libs needed for config test
      self.extraEnvOptions = {}

      # --- Build up settings using distutils.sysconfig to get Python build options --- #
      # distutils.sysconfig.get_config_vars()
      try:
         self.python_version = distutils.sysconfig.get_python_version()    # ex: '2.3'
      except:
         self.python_version = distutils.sysconfig.get_config_var("VERSION")    # ex: '2.3'
      self.python_inc_dir = distutils.sysconfig.get_python_inc()
      self._extraBoostLibs = []

      if sca_util.GetPlatform() == "win32":
         self.python_embedded_link_flags = []
         self.python_lib_path = [pj(sys.prefix,'libs')]
         self.python_static_lib_path = [""]                   # There is no static lib on win32
         lib_python_fname = 'python' + self.python_version.replace('.','')
         self.python_extra_libs = [lib_python_fname]
         #self.python_link_flags = ["/NODEFAULTLIB:"+lib_python_fname+"_d"]
         self.python_link_flags = []
         self.thread_extra_libs = []
      else:
         # Link flags that may be needed on unix for the embedded case
         #self.python_embedded_link_flags = [distutils.sysconfig.get_config_var('LINKFORSHARED')]
         self.python_embedded_link_flags = \
            distutils.sysconfig.get_config_var('LINKFORSHARED').split(' ')
         self.python_lib_path = \
            [distutils.sysconfig.get_config_var('LIBDIR')]
         self.python_static_lib_path = self.python_lib_path
         self.python_link_flags = []
         self.python_extra_libs = []
         python_extra_libs = \
            distutils.sysconfig.get_config_var('LIBS').split(' ') + \
            distutils.sysconfig.get_config_var('LOCALMODLIBS').split(' ')

         for item in python_extra_libs:
            if item.startswith("-l"):
               self.python_extra_libs.append(item)
            else:
               self.python_link_flags.append(item)

         # TODO: Figure out a more portable way (ideally extracting this
         # information from distutils.sysconfig).
         if sca_util.GetPlatform() == 'darwin':
            self.python_link_flags = \
               ['-F%s' % distutils.sysconfig.get_config_var('PYTHONFRAMEWORKPREFIX'),
                '-framework', 'Python'] + self.python_link_flags
         else:
            lib_python_fname = 'python' + self.python_version
            self.python_extra_libs.insert(0, lib_python_fname)

         self.thread_extra_libs = []
      
   def _getLibNameGenerators(self, env):
      """
      Constructs a list of callables that can be used to generate variants for a given Boost
      library name. This takes into account whether we can strip off parts of the name.

      @rtype: list of callables
      @return: A list of callable objects is returned to the caller. Each takes a single string
               parameter that is the basic name of the Boost library (such as "filesystem"). The
               string returned is a possible variant of that library based on factors including
               the Boost version and whether multi-threading should be used.
      """
      def generateName(libname, variant):
         return "boost_" + libname + variant

      debug_ext = "-d"
      if sca_util.GetPlatform() == "win32":
         debug_ext = "-gd"     # Hack for now assuming debug code and runtime

      toolset_part   = ""
      threading_part = ""
      runtime_part   = ""
      version_part   = ""
      
      if self.use_ver and self.libVersionStr is not None:
         version_part = "-" + self.libVersionStr

      if self.use_mt:
         threading_part = "-mt"

      if sca_util.GetPlatform() == 'darwin' and self.version_int_list[1] >= 37:
         toolset_part = '-xgcc' + "".join(env["CXXVERSION"].split('.')[:2])
      else:
         if self.toolset:
            toolset_part = "-" + self.toolset

         if self.use_debug:
            runtime_part = debug_ext
         elif env and env.has_key("variant") and env["variant"].has_key("type"):
            var_type = env["variant"]["type"]
            if "debugrt" == var_type:
               runtime_part = debug_ext

      generators = [lambda n: generateName(n, toolset_part + threading_part + runtime_part + version_part)]

      if self.allowLibNameFallbacks:
         # Prefer multi-threaded variants.
         if threading_part:
            generators.append(lambda n: generateName(n, toolset_part + threading_part + runtime_part + version_part))
            generators.append(lambda n: generateName(n, threading_part + runtime_part + version_part))
            generators.append(lambda n: generateName(n, threading_part + runtime_part))

         generators.append(lambda n: generateName(n, toolset_part + runtime_part + version_part))
         generators.append(lambda n: generateName(n, runtime_part + version_part))
         generators.append(lambda n: generateName(n, runtime_part))

      return generators

   def getFullLibName(self, libname, env, useDebug=False):
      """ Return the full name of the library we should link against
          to get the symbols for the library named "libname" """
      if not self.found_libs.has_key(libname):
         return libname
      else:
         return self.found_libs[libname]
      
         #if useDebug is None:
         #   if self.use_debug:
         #      fullname += debug_ext
         #   elif env and env.has_key("variant") and env["variant"].has_key("type"):
         #      var_type = env["variant"]["type"]
         #      if "debugrt" == var_type:
         #         fullname += debug_ext
         #elif useDebug is True:
         #   fullname += debug_ext
         
   def startProcess(self):
      """ Called at beginning of processing.  Perform any intialization or notification here. """
      print "Updating ", self.name
       
   def setInitial(self, optDict):
      " Set initial values from given options dictionary. "
      print "Loading initial settings for boost"
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         print "   %s specified or cached. [%s]."% (self.baseDirKey, self.baseDir)
      if optDict.has_key(self.incDirKey):
         self.incDir = optDict[self.incDirKey]
         print "   %s specified or cached. [%s]."% (self.incDirKey, self.incDir)

   def find(self, env):
      """ If base dir was not specified, attempt to find boost
          using some very basic searches.
      """
      # --- Determine toolset --- #
      if self.toolset == "auto":
         print "   Boost, autofinding toolset... ",
         
         if env["CC"] == "gcc":
            if sca_util.GetPlatform() == 'darwin':
               self.toolset = "darwin"
            else:
               self.toolset = "gcc"
         elif env["CC"] == "cl" and env.has_key("MSVS"):
            ver = env["MSVS"]["VERSION"]
            if "7.0" == ver:
               self.toolset = "vc7"
            else:
               (major, minor) = ver.rstrip("Exp").split(".")[:2]
               self.toolset = "vc%s%s" % (major, minor)
         elif sca_util.GetPlatform() == 'darwin' and env['CC'] == 'cc':
            self.toolset = "darwin"
         else:
            self.checkRequired("Could not auto determine boost toolset.")
            return
         
         print " toolset: [%s]"%self.toolset
         
      # Quick exit if nothing to find
      if self.baseDir != None:
         return
      
      ver_header = None
      boost_header = pj("boost","version.hpp")

      # Find boost/version.hpp
      print "   searching for boost..."
      
      # May be able to use configure context here...
      directories_to_check = [env.Dictionary().get("CPPPATH"), pj("/","usr","include"),
                              pj("/","usr","local","include")]

      for d in directories_to_check:
         if None != d:
            ver_header = env.FindFile(boost_header, d)
            if ver_header:
               break

      if None == ver_header:
         self.checkRequired("   could not find boost header [%s] in paths: %s"%(boost_header,directories_to_check))
      else:
         ver_header = str(ver_header)
         print "   found at: %s\n"%ver_header

         # find base dir
         self.incDir = os.path.dirname(os.path.dirname(ver_header))
         self.baseDir = os.path.dirname(self.incDir)         

   def validate(self, env):
      " Check to make sure that the current settings work and are valid. """
      # Check that path exist
      # Check that an include file: boost/version.hpp  exists
      passed = True
      self.available = False
      
      if not self.baseDir:
         self.checkRequired("Boost base dir not set")
         return
      
      if not os.path.isdir(self.baseDir):    # If we don't have a directory
         self.checkRequired("Boost base dir is not a directory: %s" % self.baseDir)
         return

      # Make sure we have a valid inc dir or try to find one
      if self.incDir:
         # Check the version header is there         
         version_header = pj(self.incDir, 'boost', 'version.hpp')         
         if not os.path.isfile(version_header):
            self.checkRequired("Boost version.hpp header does not exist:%s"%version_header)
            return
         
      # --- Find include path --- #
      # If inc dir not set, try to find it      
      # - Try just include, try local, then try subdirs in reverse sorted order (to get most recent)
      else:
         print "   Searching for correct boost include dir...",
         base_include_dir = pj(self.baseDir, 'include')
         potential_dirs = [base_include_dir, self.baseDir]
         if os.path.isdir(base_include_dir):
            inc_dirs = [pj(base_include_dir,d) for d in os.listdir(base_include_dir)]
            inc_dirs.sort()
            inc_dirs.reverse()
            potential_dirs.extend(inc_dirs)
         
         for d in potential_dirs:
            if os.path.isfile(pj(d,'boost','version.hpp')):
               self.incDir = d
               break
         
         if self.incDir:
            print "  found: ", self.incDir
         else:
            print "  not found."
            self.checkRequired("Can not find boost include directory.")
            return

         
      print "   boost include path: ", self.incDir
      
      # --- Check version requirement --- #
      version_header = pj(self.incDir, 'boost', 'version.hpp')
      ver_file = file(version_header)
      ver_file_contents = ver_file.read()
      ver_match = re.search("define\s+?BOOST_VERSION\s+?(\d*)",
                            ver_file_contents)
      if not ver_match:
         self.checkRequired("   could not find BOOST_VERSION in file: %s"%version_header)
         return

      lib_ver_match = re.search("define\s+?BOOST_LIB_VERSION\s+?\"(.*)\"",
                                ver_file_contents)
      if lib_ver_match:
         self.libVersionStr = lib_ver_match.group(1)
      else:         
         print "WARNING: Could not determine library version string"
         self.libVersionStr = None

      self.version_int = int(ver_match.group(1))
      self.version_str = str(self.version_int / 100000) + '.' + \
         str(self.version_int / 100 % 1000) + '.' + str(self.version_int % 100)
      req_ver = [int(n) for n in self.requiredVersion.split('.')]
      self.version_int_list = [int(n) for n in self.version_str.split('.')]
      print "   boost version:", self.version_str
      if self.version_int_list < req_ver:
         self.checkRequired("   Boost version is too old! Required %s but found %s"%(self.requiredVersion,self.version_str))
         return

      # XXX: If we are using boost 1.34 or newer with gcc we need to append version.
      if "gcc" == self.toolset and self.version_int_list[1] >= 34:
         self.toolset += "".join(env["CXXVERSION"].split('.')[:2])

      arch_str = SConsAddons.Util.GetArch()

      # Set lists of the options we want
      self.found_incs = [self.incDir]
      self.found_incs_as_flags = [env["INCPREFIX"] + p for p in self.found_incs];      
      self.found_defines = []
      self.found_lib_paths = [pj(self.baseDir, 'lib')] 
      if re.search(r'64', arch_str):
         lib64_dir = pj(self.baseDir, 'lib64')
         if os.path.exists(lib64_dir):
            self.found_lib_paths = [lib64_dir]

      if self.version_int_list[1] >= 35:
         self._extraBoostLibs.append("system")

      if self.preferDynamic:
         self.found_defines.append("BOOST_ALL_DYN_LINK")
      #if not self.autoLink:
      #   self.found_defines.append("BOOST_ALL_NO_LIB")

      ######## BUILD CHECKS ###########  
      # --- Check building against libraries --- #   
      def check_lib(libname, lib_filename, extraLibs, env):         
         """ Helper method that checks if we can compile code that uses
             capabilities from the boost library 'libname' and get the
             symbols from the library lib_filename.
         """
         header_to_check = pj('boost','config.hpp')
         if self.headerMap.has_key(libname):
            header_to_check = self.headerMap[libname]

         # Create config environment
         # - Need to extend the environment
         conf_env = env.Clone()
         conf_env.Append(CPPPATH= self.found_incs, 
                         LIBPATH = self.found_lib_paths,
                         LIBS = extraLibs,
                         CPPDEFINES = self.found_defines)
         if "python" == libname:
            conf_env.Append(CPPPATH = self.python_inc_dir,
                            LIBPATH = self.python_lib_path,
                            LINKFLAGS = self.python_link_flags,
                            #LIBS = [lib_filename] + self.python_extra_libs
                            LIBS = self.python_extra_libs
                         )
         
         # Thread library needs some additional libraries on Linux... (yuck)
         if "thread" == libname:
            conf_env.Append(LIBS = [lib_filename] + self.thread_extra_libs)

         platform = sca_util.GetPlatform()

         # We have to use the dynamic MSVC runtime in order to allow dynamic
         # linking of Boost libraries during this phase of testing.
         if self.preferDynamic and platform == "win32":
            conf_env.Append(CXXFLAGS = ["/MD"])

         conf_ctxt = Configure(conf_env)
         result = conf_ctxt.CheckLibWithHeader(lib_filename, header_to_check, "c++")
         conf_ctxt.Finish()         
         return result

      # XXX: Hack to account for the fact that boost_python did not support
      #      auto-linking until Boost 1.34.
      libs_to_find = []
      if not self.autoLink:
         libs_to_find += self.lib_names
      elif 'python' in self.lib_names and self.version_str < "1.34":
         libs_to_find.append('python')

      # For each lib we are supposed to find.
      #  - Search through possible names for that library
      #     - If we find one that works, store it
      generators = self._getLibNameGenerators(env)
      possible_lib_names = []

      for libname in libs_to_find:
         found_full_name = None

         for generator in generators:
            test_name = generator(libname)
            possible_lib_names.append(test_name)
            extra_boost_libs = [generator(l) for l in self._extraBoostLibs]

            result = check_lib(libname, test_name, extra_boost_libs, env)
            if result:
               found_full_name = test_name
               break
            
         if not found_full_name:
            passed = False
            self.checkRequired("Unable to find library '%s'; tried %s" % 
                               (libname, ", ".join(possible_lib_names)))
         else:
            self.found_libs[libname] = found_full_name
            print "  %s: %s" % (libname, found_full_name)

      # --- Handle final settings ---- #     
      if not passed:
         # Clear everything
         self.baseDir = None
         self.incDir = None
         edict = env.Dictionary()
         for k in (self.baseDirKey, self.incDirKey):
            if edict.has_key(k):
               del edict[k]
         self.found_incs = None
         self.found_lib_paths = []
         self.found_libs = {}
         self.found_defines = []
      else:
         self.available = True


   def apply(self, env, libs=None, useCppPath=False, useDebug=None):
      """ Add environment options for building against Boost libraries.
          Apply the options and take into account any variant information
          in the environment.
      """
      if self.found_incs:
         if self.useCppPath or useCppPath:
            env.AppendUnique(CPPPATH = self.found_incs)
         else:
            env.AppendUnique(CXXFLAGS = self.found_incs_as_flags)
      env.AppendUnique(CPPDEFINES = self.found_defines,
                       LIBPATH = self.found_lib_paths)
      if not self.autoLink:
         full_libs = [self.getFullLibName(l,env, useDebug) for l in self.lib_names if 'python' != l]         
         env.AppendUnique(LIBS = full_libs)      


   def updatePythonEmbeddedEnv(self,env):
      self.applyPythonEmbeddedEnv(env)

   def applyPythonEmbeddedEnv(self,env, useDebug=None):
      """ Update the environment for building python embedded. 
          XXX: may need python_static_lib_path.
      """
      self.apply(env)
      if not self.autoLink or self.version_str < "1.34":
         #print "Full python lib name:", self.buildFullLibName('python')
         env.AppendUnique(LIBS = [self.getFullLibName('python',env, useDebug)])
      env.AppendUnique(CPPPATH = [self.python_inc_dir],
                       LINKFLAGS = self.python_embedded_link_flags + self.python_link_flags,
                       LIBPATH = self.python_lib_path,
                       LIBS = self.python_extra_libs)

   
   def updatePythonModEnv(self, env, useDebug=None):
      self.applyPythonModEnv(env, useDebug)
   
   def applyPythonModEnv(self, env, useDebug=None, **kwds):
      """ Update the environment for building python modules 
          kwds - Dictionary of customization flags
             use_visibility - if true, then try to use visibility flags on g++
      """
      if not "python" in self.lib_names:
         print "Tried to updatePythonModEnv with boost option object not configured with python library.\n"
         sys.exit(0)
         
      self.apply(env)

      if not self.autoLink or self.version_str < "1.34":
         env.AppendUnique(LIBS = [self.getFullLibName("python", env, useDebug)])    # Add on the boost python library
      env.AppendUnique(CPPPATH = [self.python_inc_dir],
                       LINKFLAGS = self.python_link_flags,
                       LIBPATH = self.python_lib_path,
                       LIBS = self.python_extra_libs)

      platform = sca_util.GetPlatform()
            
      env["SHLIBPREFIX"] = ""                    # Clear the library prefix settings
      if platform == "win32":
         env["SHLIBSUFFIX"] = ".pyd"
         
      if platform == "linux":
         env['CXXCOM'] += " && objcopy --set-section-flags .debug_str=contents,debug $TARGET"
         env['SHCXXCOM'] += " && objcopy -v --set-section-flags .debug_str=contents,debug $TARGET $TARGET"
      
      # Add visibility flags for gcc 4.0 and greater
      # XXX: It seems that this may not work on Mac OS X.
      if platform != "darwin" and "g++" in env["CXX"] and kwds.get("use_visibility",True):
         gcc_version = env["CXXVERSION"].split(".")
         if int(gcc_version[0]) >= 4:         
            env.AppendUnique(CXXFLAGS = ['-fvisibility=hidden', '-fvisibility-inlines-hidden'])

   def getSettings(self):
      return [(self.baseDirKey, self.baseDir), (self.incDirKey, self.incDir)]
   
   def dumpSettings(self,env=None):
      "Write out the settings"
      print "BoostBaseDir:", self.baseDir
      print "BoostIncludeDir:", self.incDir
      print "CPPPATH (as flags):", self.found_incs_as_flags
      print "CPPDEFINES:", self.found_defines
      print "LIBS:", self.lib_names
      print "LIBS: (real):", [self.getFullLibName(l,env) for l in self.lib_names]
      print "LIBPATH:", self.found_lib_paths      
      print "Python settings"
      print "               inc:", self.python_inc_dir
      print "     embedded link:", self.python_embedded_link_flags
      print "               lib:", self.python_extra_libs
      print "   static lib path:", self.python_static_lib_path
