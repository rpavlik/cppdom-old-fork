"""SConsAddons.Options.GMTL

Defines options for GMTL project
"""


__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

import SCons.Environment
import SCons
import SConsAddons.Options
import SCons.Util
import sys, os, re, string

from SCons.Util import WhereIs
pj = os.path.join

import SCons.SConf
Configure = SCons.SConf.SConf    # Use same alias as SConstruct sees

class GMTL(SConsAddons.Options.PackageOption):
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
      SConsAddons.Options.LocalUpdateOption.__init__(self, name,
      self.baseDirKey, help_text)

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
      " Set initial values from given dictionary "
      sys.stdout.write("checking for gmtl...")
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         self.gmtlconfig_cmd = pj(self.baseDir, 'bin', 'gmtl-config')
         sys.stdout.write("specified or cached. [%s].\n"% self.baseDir)

   def find(self, env):
      # quick exit if nothing to find
      if self.baseDir !=None:
         return

      sys.stdout.write("searching...\n")
      self.plxconfig_cmd = WhereIs('gmtl-config')
      if None == self.gmtlconfig_cmd:
         self.checkRequired("   could not find gmtl-config")
      else:
         sys.stdout.write("   found gmtl-config.\n")
         found_ver_str = os.popen(self.gmtlconfig_cmd + " --version").read().strip()
         sys.stdout.write("   version:%s"%found_ver_str)

         self.baseDir = os.popen(self.gmtlconfig_cmd + " --prefix").read().strip()

         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"%self.baseDir)
            self.baseDir = None;
         else:
            print "   found at: ", self.baseDir

   def convert(self):
      pass

   def set(self, env):
      if self.baseDir:
         env[self.baseDirKey] = self.baseDir

   def validate(self, env):
      # check path existance
      # check gmtl-config existance
      # check include/gmtl/gmtl.h existance
      # check version correctness
      # update the temps for later usage
      passed = True
      if not os.path.isdir(self.baseDir):
         passed = False
         self.checkRequired("gmtl base dir does not exist:%s"%self.baseDir)
      if not os.path.isfile(self.gmtlconfig_cmd):
         passed = False;
         self.checkRequired("gmtl-config does not exist:%s"%self.gmtlconfig_cmd)

      cfg_cmd_parser = SConsAddons.Util.ConfigCmdParser(self.gmtlconfig_cmd)

      # check version requirement

      found_ver_str = cfg_cmd_parser.getVersion()
      req_ver = [int(n) for n in self.requiredVersion.split(".")]
      found_ver = [int(n) for n in found_ver_str.split(".")]
      if found_ver < req_ver:
         passed = False
         self.checkRequired("   found version is to old: required:%s found:%s"%(self.requiredVersion, found_ver_str))

      gmtl_header_file = pj(self.baseDir, 'include', 'gmtl', 'gmtl.h')
      if not os.path.isfile(gmtl_header_file):
         passed = False
         self.checkRequired("gmtl.h not found:%s"%gmtl_header_file)

      self.found_incs = cfg_cmd_parser.findIncludes(" --cxxflags")
      #self.found_libs = cfg_cmd_parser.findLibs()
      #self.found_lib_paths = cfg_cmd_parser.findLibPaths()

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

   def updateEnv(self, env):
      """ Add environment options for building against plexus"""
      if self.found_incs:
         env.Append(CPPPATH = self.found_incs)
      #if self.found_libs:
      #   env.Append(LIBS = self.found_libs)
      #if self.found_lib_paths:
      #   env.Append(LIBPATH = self.found_lib_paths)
         
   def dumpSettings(self):
      "Write out the settings"
      print "GMTLBaseDir:", self.baseDir
      print "gmtl-config:", self.gmtlconfig_cmd
      print "CPPPATH:", self.found_incs
      #print "LIBS:", self.found_libs
      #print "LIBPATH:", self.found_lib_paths
