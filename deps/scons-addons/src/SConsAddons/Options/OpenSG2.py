"""SConsAddons.Options.OpenSG2

Defines options for OpenSG project
"""


__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

import SCons
import SConsAddons.Options
import SCons.Util
import sys, os, re

from SCons.Util import WhereIs
pj = os.path.join

class OpenSG2(SConsAddons.Options.PackageOption):
   """
   Options object for capturing OpenSG2 options and dependencies
   """
   def __init__(self, name, requiredVersion, required=True):
      """
         name - The name to use for this option
         requiredVersion - the version of OpenSG required (ex: "0.3.3")
         required - Is the dependency required? (if so we exit on errors)
      """
      self.baseDirKey = "OpenSG2BaseDir"
      help_text = """Base directory for OpenSG. bin and include should be under this directory."""
      self.requiredVersion = requiredVersion
      self.required = required
      self.available = False
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey,
                                                 help_text)

      self.baseDir = None
      self.config_script = None

   def isAvailable(self):
      return self.available

   def checkRequired(self, msg):
      """ Called when there is a config error.  If required, then exits with
      error message """
      print msg
      if self.required:
         sys.exit(1)

   def setInitial(self, optDict):
      " Set initial values from given dictionary. "
      if self.verbose:
         print "   Loading initial OpenSG settings."
      if optDict.has_key(self.baseDirKey):
         self.baseDir = os.path.abspath(optDict[self.baseDirKey])
         if self.verbose:
            print "   %s specified or cached. [%s]."% (self.baseDirKey, self.baseDir);
         self.config_script = pj(self.baseDir, 'bin', 'osg2-config')

   def find(self, env):
      # Quick exit if nothing to find because it is already specified
      if self.baseDir != None:
         assert self.baseDir
         return

      # Find osg2-config and call it to get the other arguments
      sys.stdout.write("searching...\n")
      self.config_script = WhereIs('osg2-config', pathext='')
      if None == self.config_script:
         self.checkRequired("   could not find osg2-config")
      else:
         sys.stdout.write("   found osg2-config.\n")
         full_command = sys.executable + ' ' + self.config_script
         # find base dir
         self.baseDir = os.popen(full_command + " --prefix").read().strip()
         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"%self.baseDir)
            self.baseDir = None
         else:
            print "   found at: ", self.baseDir

   def validate(self, env):
      # check path existance
      # check osg2-config existance
      # check include/OpenSG/OpenSG.h existance
      # check version correctness
      # XXX: Check that an include file: include/OpenSG/OpenSG.h  exists
      # update the temps for later usage
      passed = True
      if self.baseDir is None:
         passed = False
         self.checkRequired("OpenSG2 base dir not specified")
         return

      if not os.path.isdir(self.baseDir):
         passed = False
         self.checkRequired("OpenSG2 base dir does not exist:%s"%self.baseDir)
      elif self.verbose:
         print "   %s is valid directory."% (self.baseDir)

      # If OpenSG2-config exists and we are not on windows, use it.
      if not os.path.isfile(self.config_script):
         passed = False
         self.checkRequired("Can not find %s." % self.config_script)
      elif self.verbose:
         print "   found osg2-config %s."% (self.config_script)

      cfg_cmd_parser = SConsAddons.Util.PythonScriptParser(self.config_script)

      # -- Find header directory -- #
      base_include = pj(self.baseDir,'include')
      header_file = pj('OpenSG', 'OSGConfig.h')
      # check standard directory first
      if not os.path.isfile(pj(base_include, header_file)):
         passed = False
         self.checkRequired("Could not find OSGConfig.h. [%s]" % (pj(base_include, header_file)))
      elif self.verbose:
         print "   Found OpenSG include directory: ", base_include

      # Regular expresion to grab version out of version string. (ex 2.0.0-pre1)
      versionRe = re.compile("(\d*)\.(\d*)\.(\d*)")

      # Get version string and grab version from result.
      found_ver_str = cfg_cmd_parser.getVersion()
      version_match = versionRe.match(found_ver_str)
      if version_match is None:
         passed = False;
         self.checkRequired("   could not determine OpenSG version from [%s]"%found_ver_str)
      found_ver = list(version_match.groups())
      req_ver = self.requiredVersion.split(".")

      # Pad both version lists with 0s. This addesses the case of [2] < [2,0]
      found_ver = found_ver + [0]*(3-len(found_ver))
      req_ver = req_ver + [0]*(3-len(req_ver))
      if found_ver < req_ver:
         passed = False;
         self.checkRequired("   OpenSG version is too old! Required %s but found %s"%(self.requiredVersion,found_ver_str))
      elif self.verbose:
         print "   Found OpenSG version [%s] required [%s]" % (found_ver_str, self.requiredVersion)

      # --- Build flag settings --- #         
      # Returns lists of the options we want
      # XXX: Should we really need to pass 'Base' here?
      self.found_incs = cfg_cmd_parser.findIncludes("--cflags Base")
      if self.verbose:
         print "   --cflags =", self.found_incs

      if not passed:
         self.available = False 
         self.baseDir = None
         self.config_script = None
         self.found_incs = None

         # Remove base directory from environment dictonary.
         edict = env.Dictionary()
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey]
      else:
         self.available = True

   def apply(self, env, libs = ['System'], buildType = None):
      """ Add environment options for building against OpenSG"""

      # A build to be made with debugging symbols and linked against the MSVC
      # debug runtime. This really should apply only to Windows.
      if 'dbgrt' == buildType or 'debugrt' == buildType:
         opt_option = " --dbgrt"
      # A build to be made without debugging symbols. This is linked against
      # the regular C/C++ runtime implementation.
      elif 'opt' == buildType or 'optimized' == buildType:
         opt_option = " --opt"
      # A build to be made with debugging symbols. This is linked against the
      # regular C/C++ runtime implementation.
      elif "dbg" == buildType or "debug" == buildType:
         opt_option = " --dbg"
      else:
         # Default to debug            
         opt_option = " --dbg"
         if env.has_key("variant") and env["variant"].has_key("type"):
            var_type = env["variant"]["type"]

            if "debugrt" == var_type:
               opt_option = " --dbgrt"
            elif "debug" != var_type:
               opt_option = " --opt"
            else:
               opt_option = " --dbg"

      # Ensure that libs is a list.
      if not isinstance(libs, list):
         libs = [libs,]

      cfg_cmd_parser = SConsAddons.Util.PythonScriptParser(self.config_script)

      lib_names_str = " ".join(libs)
      extra_params = opt_option + ' ' + lib_names_str
      found_libs = cfg_cmd_parser.findLibs("--libs " + extra_params)
      found_frameworks = cfg_cmd_parser.findFrameworks("--libs " + extra_params)
      found_lib_paths = cfg_cmd_parser.findLibPaths("--llibs %s" % extra_params)
      found_includes = cfg_cmd_parser.findIncludes("--cflags %s" % extra_params)
      # NOTE: findCXXFlags seems to parse for defines.
      found_defines = cfg_cmd_parser.findCXXFlags("--cflags %s"%extra_params)

      if self.verbose:
         print "   found_libs       =", found_libs
         print "   found_frameworks =", found_frameworks
         print "   found_lib_paths  =", found_lib_paths
         print "   found_includes   =", found_includes
         print "   found_defines    =", found_defines

      if len(found_includes):
         env.AppendUnique(CPPPATH = found_includes);

      if len(found_libs):
         env.AppendUnique(LIBS = found_libs)
      else:
         print "ERROR: Could not find OpenSG libs for libs:%s lib_names_str:%s" % (libs, lib_names_str)

      if len(found_frameworks):
         env.AppendUnique(FRAMEWORKS = found_frameworks)

      if len(found_lib_paths):
         env.AppendUnique(LIBPATH = found_lib_paths);
      else:
         print "ERROR: Could not find OpenSG lib paths for libs: %s lib_names_str:%s" % (libs, lib_names_str)
            
      if len(found_defines):
         env.AppendUnique(CPPDEFINES = found_defines)

   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),]

   def dumpSettings(self):
      "Write out the settings"
      print "OpenSGBaseDir:", self.baseDir
      print "osg2-config:", self.config_script
