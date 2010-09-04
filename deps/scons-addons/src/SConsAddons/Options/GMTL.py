"""SConsAddons.Options.GMTL

Defines options for GMTL project
"""


__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

import sys, os, re, string
pj = os.path.join
import SCons
import SCons.Environment
import SCons.Util
from SCons.Util import WhereIs
import SCons.SConf
import SConsAddons.Options
import SConsAddons.Options.FlagPollBasedOption as FlagPollBasedOption

Configure = SCons.SConf.SConf    # Use same alias as SConstruct sees

class GMTL(FlagPollBasedOption.FlagPollBasedOption):
   """ 
   Options object for capturing GMTL options and dependencies.
   """

   def __init__(self, name, requiredVersion, required=True, useCppPath=True):
      """
         name - The name to use for this option
         requiredVersion - The version of vapor required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, put the include paths in cpppath else, put them in cxxflags.
      """
      FlagPollBasedOption.FlagPollBasedOption.__init__(self, name, 'gmtl', requiredVersion, required, useCppPath)

class GMTL_config(SConsAddons.Options.PackageOption):
   """
   Options object for capturing gmtl options and dependencies
   """
   def __init__(self, name, requiredVersion, required=True):
      """
         name - The name to use for this option
         requiredVersion - the version of gmtl required (ex: "0.3.3")
         required - Is the dependency required? (if so we exit on errors)
      """
      help_text = """Base directory for gmtl. bin and include should be under this directory."""
      self.baseDirKey = "GMTLBaseDir"
      self.requiredVersion = requiredVersion
      self.required = required
      self.available = False
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey, help_text)

      self.baseDir = None;
      self.gmtlconfig_cmd = None;

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
         print "   Loading initial GMTL settings."
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey];
         self.gmtlconfig_cmd = pj(self.baseDir, 'bin', 'gmtl-config')
         if self.verbose:
            print "   %s specified or cached. [%s]."% (self.baseDirKey, self.baseDir);


   def find(self, env):
      # Quick exit if nothing to find because it is already specified
      if self.baseDir !=None:
         assert self.baseDir
         return

      # Find gmtl-config and call it to get the other arguments
      sys.stdout.write("searching...\n")
      self.gmtlconfig_cmd = WhereIs('gmtl-config')
      if None == self.gmtlconfig_cmd:
         self.checkRequired("   could not find gmtl-config")
      else:
         sys.stdout.write("   found gmtl-config.\n")
         # find base dir
         self.baseDir = os.popen(self.gmtlconfig_cmd + " --prefix").read().strip()
         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"%self.baseDir)
            self.baseDir = None
         else:
            print "   found at: ", self.baseDir

   def validate(self, env):
      # check path existance
      # check gmtl-config existance
      # check include/gmtl/gmtl.h existance
      # check version correctness
      # XXX: Check that an include file: include/gmtl/gmtl.h  exists
      # update the temps for later usage
      passed = True
      if not os.path.isdir(self.baseDir):
         passed = False
         self.checkRequired("gmtl base dir does not exist:%s"%self.baseDir)

      # If gmtl-config exists and we are not on windows, use it.
      has_config_cmd = os.path.isfile(self.gmtlconfig_cmd) and \
         not SConsAddons.Util.GetPlatform() == "win32"

      if not has_config_cmd:
         print "Can not find %s. Limping along without it." % self.gmtlconfig_cmd
      else:
         cfg_cmd_parser = SConsAddons.Util.ConfigCmdParser(self.gmtlconfig_cmd)           

      # -- Find header directory -- #
      inc_dir = None
      header_file = pj('gmtl', 'gmtl.h')
      base_include = pj(self.baseDir,'include')
      # check standard directory first
      if os.path.isfile(pj(base_include, header_file)):
         inc_dir = base_include
      # check versioned directories by building a list and sorting them
      elif os.path.exists(base_include):
         pot_dirs = [pj(base_include,d) for d in os.listdir(base_include)\
                                              if d.count("gmtl")]
         pot_dirs.sort()
         pot_dirs.reverse()         
         for d in pot_dirs:
            if os.path.isfile(pj(d,header_file)):
               inc_dir = d
               break

      if not inc_dir:
         passed = False
         self.checkRequired("gmtl.h not found in any gmtl directories.")
      else:
         print "   Found gmtl include directory: ", inc_dir


      # --- Check version requirement --- #
      req_ver = [int(n) for n in self.requiredVersion.split(".")]
      version_header = pj(inc_dir,'gmtl','Version.h')
      if not os.path.isfile(version_header):
         passed = False
         self.checkRequired("%s does not exist.  Can not determine gmtl version."%version_header)
   
      found_ver = GetGMTLVersion(version_header)
      found_ver_str = '.'.join([str(i) for i in found_ver])

      print "   Found GMTL version: ", found_ver_str
      if found_ver < req_ver:
         passed = False
         self.checkRequired("   GMTL version is too old! Required %s but found %s"%(self.requiredVersion, found_ver_str))

      # --- Build flag settings --- #         
      if has_config_cmd:
         # Returns lists of the options we want
         self.found_incs = [inc_dir,] + cfg_cmd_parser.findIncludes(" --cxxflags")
      else:
         self.found_incs = [inc_dir,]         

      if not passed:
         self.baseDir = None
         self.gmtlconfig_cmd = None
         edict = env.Dictionary()
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey]
         self.found_incs = None
         self.found_libs = None
         self.found_lib_paths = None
      else:
         self.available = True

   def apply(self, env):
      """ Add environment options for building against gmtl"""
      if self.found_incs:
         env.AppendUnique(CPPPATH = self.found_incs);

   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),]

   def dumpSettings(self):
      "Write out the settings"
      print "GMTLBaseDir:", self.baseDir
      print "gmtl-config:", self.gmtlconfig_cmd
      print "CPPPATH:", self.found_incs

def GetGMTLVersion(versionHeader):
   """Gets the GMTL version from gmtl/Version.h.
      Returns version as tuple (major,minor,patch)
   """
   contents = open(versionHeader, 'r').read()
   major = re.compile('.*(#define *GMTL_VERSION_MAJOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   minor = re.compile('.*(#define *GMTL_VERSION_MINOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   patch = re.compile('.*(#define *GMTL_VERSION_PATCH *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   return (int(major), int(minor), int(patch))
