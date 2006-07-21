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
import SConsAddons.Util
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
   def __init__(self, name, requiredVersion, useDebug=False, useMt=True, libs=[], required=True, useCppPath=False, toolset="gcc"):
      """
         name - The name to use for this option
         requiredVersion - The version of Boost required (ex: "1.30.0")
         useDebug - Should we use debug boost libraries [default: False]
         useMt - Should we use multi-threaded boost libraries [default: True]
         libs - Boost libraries needed that are actually compiled (base library names. ex: python)
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, then include path is added to CPPPATH if not, then added to CPPFLAGS directly
	 toolset - The toolset name to use (ex: "gcc", "il")
      """
      help_text = ["Base directory for Boost. include, and lib should be under this directory.",
                   "Include directory for boost (if not under base)."]
      self.baseDirKey = "BoostBaseDir"
      self.incDirKey = "BoostIncludeDir"
      self.requiredVersion = requiredVersion
      self.lib_names = libs
      self.required = required
      self.useCppPath = useCppPath
      self.toolset = toolset
      SConsAddons.Options.PackageOption.__init__(self, name, [self.baseDirKey, self.incDirKey], help_text)
      self.available = False            # Track availability
      
      self.found_incs = []
      self.found_incs_as_flags = ""     # The includes as flags to add on command line
      self.found_lib_paths = []

      # configurable options
      self.baseDir = None
      self.incDir = None
      self.setupLibrarySettings()
      
      # Options for which libraries to use
      self.use_mt = useMt
      self.use_debug = useDebug 
      print "Use debug set to:", self.use_debug

   def setToolset(self, toolset):
      self.toolset = toolset
   
   def setUseMt(self, bval=True):
      self.use_mt = bval
      
   def setUseDebug(self, bval=True):
      self.use_debug = bval

   def setupLibrarySettings(self):
      # Map from library name to header to check for       
      self.headerMap = { 'program_options':'boost/program_options.hpp',
                         'python':'boost/python.hpp',
                         'thread':'boost/thread.hpp',
                         'filesystem':'boost/filesystem/path.hpp' }
      
      # Map for extra libs needed for config test
      self.extraEnvOptions = {}

      # --- Build up settings using distutils.sysconfig to get Python build options --- #
      # distutils.sysconfig.get_config_vars()
      #self.python_version = distutils.sysconfig.get_python_version()    # ex: '2.3'
      self.python_version = distutils.sysconfig.get_config_var("VERSION")    # ex: '2.3'
      self.python_inc_dir = distutils.sysconfig.get_python_inc()
      #python_link_share_flags = distutils.sysconfig.get_config_var('LINKFORSHARED')
      self.python_link_share_flags = "-Wl,-export-dynamic"
      self.python_lib_path = distutils.sysconfig.get_python_lib(standard_lib=True) + "/config"
      self.python_extra_libs = ["python"+self.python_version, "util", "pthread", "dl"]  # See SHLIBS
      
      
   def buildFullLibName(self, libname):
      """ Returns the full name of the boost library"""
      fullname = "boost_" + libname + "-" + self.toolset
      if self.use_mt:
         fullname += "-mt"
      if self.use_debug:
         fullname += "-d"
         
      return fullname
      
   def checkRequired(self, msg):
      """ Called when there is config problem.  If required, then exit with error message """
      print msg
      if self.required:
         sys.exit(0)

   def isAvailable(self):
      " If true, then validation passed and we should be able to use boost. "
      return self.available

   def setInitial(self, optDict):
      " Set initial values from given dict "
      print "Loading initial settings for boost"
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         print "   %s specified or cached. [%s]."% (self.baseDirKey, self.baseDir)
      if optDict.has_key(self.incDirKey):
         self.incDir = optDict[self.incDirKey]
         print "   %s specified or cached. [%s]."% (self.incDirKey, self.incDir)

   def find(self, env):
      # Quick exit if nothing to find
      if self.baseDir != None:
         return
      
      ver_header = None

      # Find boost/version.hpp
      print "   searching for boost..."
      if env.Dictionary().has_key('CPPPATH'):
         print "      Searching CPPPATH..."
         ver_header = env.FindFile(pj('boost','version.hpp'), env['CPPPATH'])

      if (None == ver_header) and env.Dictionary().has_key('CPLUS_INCLUDE_PATH'):
         print "      Searching CPLUS_INCLUDE_PATH..."
         ver_header = SCons.Script.SConscript.FindFile(pj('boost', 'version.hpp'),
                                    string.split(env['ENV']['CPLUS_INCLUDE_PATH'], os.pathsep))
         
      if None == ver_header:
         self.checkRequired("   could not find boost/version.hpp.")
      else:
         ver_header = str(ver_header)
         print "   found boost/version.hpp.\n"

         # find base dir
         self.baseDir = os.path.dirname(os.path.dirname(os.path.dirname(ver_header)))
         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"% self.baseDir)
            self.baseDir = None
         else:
            print "   found at: ", self.baseDir

   def convert(self):
      pass

   def set(self, env):
      if self.baseDir:
         env[self.baseDirKey] = self.baseDir
      if self.incDir:
         env[self.incDirKey] = self.incDir

   def validate(self, env):
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

      # --- Find include path --- #
      # If inc dir not set, try to find it
      # - Try just include, if not, then try subdirs in reverse sorted order
      if not self.incDir:
         self.incDir = pj(self.baseDir, 'include')
         if not os.path.isfile(pj(self.incDir, 'boost', 'version.hpp')):
            print "Searching for correct boost include dir...",
            potential_dirs = os.listdir(self.incDir)
            potential_dirs = [d for d in potential_dirs if os.path.isfile(pj(self.incDir, d, 'boost', 'version.hpp'))]
            potential_dirs.sort()
            if 0 == len(potential_dirs):
               print "none found."
            else:
               self.incDir = pj(self.incDir, potential_dirs[-1])
               print "found: ", self.incDir
         
      if self.incDir and (not os.path.isdir(pj(self.incDir,'boost'))):
         self.checkRequired("Boost inc dir is not a valid directory: %s" % pj(self.incDir,'boost'))
         return
      
      # Check the version header is there         
      version_header = pj(self.incDir, 'boost', 'version.hpp')         
      if not os.path.isfile(version_header):
         self.checkRequired("Boost version.hpp header does not exist:%s"%version_header)
         return
         
      print "   boost header path: ", self.incDir
      
      # --- Check version requirement --- #
      ver_file = file(version_header)
      ver_match = re.search("define\s+?BOOST_VERSION\s+?(\d*)", ver_file.read())
      if not ver_match:
         self.checkRequired("   could not find BOOST_VERSION in file: %s"%version_header)
         return
      found_ver_str = int(ver_match.group(1))
      found_ver_str = str(found_ver_str / 100000) + '.' + str(found_ver_str / 100 % 1000) + '.' + str(found_ver_str % 100)
      req_ver = [int(n) for n in self.requiredVersion.split('.')]
      found_ver = [int(n) for n in found_ver_str.split('.')]
      print "   boost version:", ".".join([str(x) for x in found_ver])
      if found_ver < req_ver:
         self.checkRequired("   found version is to old: required:%s found:%s"%(self.requiredVersion,found_ver_str))
         return

      # Set lists of the options we want
      self.found_incs = [self.incDir]
      self.found_incs_as_flags = [env["INCPREFIX"] + p for p in self.found_incs];
      self.found_lib_paths = [pj(self.baseDir, 'lib')]

      ######## BUILD CHECKS ###########  
      # --- Check building against libraries --- #   

      # For each library, find cononical lib name and associated header to check
      # default to checking lib with config.hpp
      for libname in self.lib_names:
         full_libname = self.buildFullLibName(libname)
         header_to_check = 'boost/config.hpp'
         if self.headerMap.has_key(libname):
            header_to_check = self.headerMap[libname]

         # Create config environment
         conf_env = env.Copy()
         conf_env.Append(CXXFLAGS= self.found_incs_as_flags, LIBPATH = self.found_lib_paths)
         if "python" == libname:
            conf_env.Append(CPPPATH = self.python_inc_dir,
                            LIBPATH = self.python_lib_path,
                            LIBS = [full_libname,] + self.python_extra_libs + ["dl",])
         if "thread" == libname:
            conf_env.Append(LIBS = [full_libname,] + ["pthread",] + ["dl",])
            conf_ctxt = Configure(conf_env)
#            result = conf_ctxt.CheckLib(full_libname, "join", header_to_check, "c++")
            result = True

         else:
            conf_ctxt =Configure(conf_env)
            result = conf_ctxt.CheckLibWithHeader(full_libname, header_to_check, "c++")

           
         if not result:
            passed = False
            self.checkRequired("Can't compile test program: lib: %s full_lib: %s header:%s"%(libname,full_libname,header_to_check))
            
         conf_ctxt.Finish()

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
         self.found_lib_paths = None
      else:
         self.available = True

         
   def updateEnv(self, env, libs=None, useCppPath=False):
      """ Add environment options for building against Boost libraries """
      if self.found_incs:
         if self.useCppPath or useCppPath:
            env.Append(CPPPATH = self.found_incs)
         else:
            env.Append(CXXFLAGS = self.found_incs_as_flags)
      if self.found_lib_paths:
         env.Append(LIBPATH = self.found_lib_paths)
      for l in self.lib_names:
         if 'python' != l:               # Don't add python by default
            env.Append(LIBS = [self.buildFullLibName(l)])

   def updatePythonEmbeddedEnv(self,env):
      """ Update the environment for building python embedded """
      self.updateEnv(env)
      #print "Full python lib name:", self.buildFullLibName('python')
      env.Append(LIBS = [self.buildFullLibName('python')])
      env.Append(CPPPATH = [self.python_inc_dir,],
                 LINKFLAGS = self.python_link_share_flags,
                 LIBPATH = self.python_lib_path,
                 LIBS = self.python_extra_libs)

                  
   def updatePythonModEnv(self, env):
      """ Update the environment for building python modules """
      if not "python" in self.lib_names:
         print "Tried to updatePythonModEnv with boost option object not configured with python library.\n"
         sys.exit(0)
         
      self.updateEnv(env)
      env.Append(LIBS = self.buildFullLibName("python") )    # Add on the boost python library
      env.Append(CPPPATH = [self.python_inc_dir,],
                 LIBPATH = self.python_lib_path,
                 LIBS = self.python_extra_libs)

            
      env["SHLIBPREFIX"] = ""                    # Clear the library prefix settings
      if(SConsAddons.Util.GetPlatform() == "linux"):
         env['CXXCOM'] += " ; objcopy --set-section-flags .debug_str=contents,debug $TARGET"
         env['SHCXXCOM'] += " ; objcopy -v --set-section-flags .debug_str=contents,debug $TARGET $TARGET"


   def dumpSettings(self):
      "Write out the settings"
      print "BoostBaseDir:", self.baseDir
      print "BoostIncludeDir:", self.incDir
      print "CPPPATH (as flags):", self.found_incs_as_flags
      print "LIBS:", self.lib_names
      print "LIBS: (full):", [self.buildFullLibName(l) for l in self.lib_names]
      print "LIBPATH:", self.found_lib_paths
      print "Python settings"
      print "   inc:", self.python_inc_dir
      print "   link:", self.python_link_share_flags
      print "   lib:", self.python_extra_libs
      print "   lib path:", self.python_lib_path
