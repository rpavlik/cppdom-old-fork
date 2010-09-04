"""SConsAddons.Options.Plexus

Defines options for Plexus project
"""


__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

import SCons.Environment
import SCons
import SConsAddons.Options
import SCons.Util
import sys
import os
import re
import string

from SCons.Util import WhereIs
pj = os.path.join

import SCons.SConf
Configure = SCons.SConf.SConf    # Use same alias as SConstruct sees

class Plexus(SConsAddons.Options.PackageOption):
   """
   Options object for capturing plexus options and dependencies
   """
   def __init__(self, name, requiredVersion, required=True):
      """
         name - THe name to use for this option
         requiredVersion - the version of plexus required (ex: "0.19.0")
         required - Is the dependency required? (if so we exit on errors)
      """
      help_text = """Base directory for plexus. bin, include, and lib should be under this directory."""
      self.baseDirKey = "PlxBaseDir"
      self.requiredVersion = requiredVersion
      self.required = required
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey, help_text)

      self.baseDir = None;
      self.plxconfig_cmd = None;

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
      sys.stdout.write("checking for plexus...")
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         self.plxconfig_cmd = pj(self.baseDir, 'bin', 'plexus-config')
         sys.stdout.write("specified or cached. [%s].\n"% self.baseDir)

   def find(self, env):
      # quick exit if nothing to find
      if self.baseDir !=None:
         return

      sys.stdout.write("searching...\n")
      self.plxconfig_cmd = WhereIs('plexus-config')
      if None == self.plxconfig_cmd:
         self.checkRequired("   could not find plexus-config")
      else:
         sys.stdout.write("   found plexus-config.\n")
         found_ver_str = os.popen(self.plxconfig_cmd + " --version").read().strip()
         sys.stdout.write("   version:%s"%found_ver_str)

         self.baseDir = os.popen(self.plxconfig_cmd + " --prefix").read().strip()

         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"%
            self.baseDir)

            self.baseDir = None;
         else:
            print "   found at: ", self.baseDir
   
   def validate(self, env):
      # check path existance
      # check plexus-config existance
      # check include/plx/plxConfig.h existance
      # check version correctness
      # update the temps for later usage
      passed = True
      if not os.path.isdir(self.baseDir):
         passed = False
         self.checkRequired("plexus base dir does not exist:%s"%self.baseDir)
      if not os.path.isfile(self.plxconfig_cmd):
         passed = False;
         self.checkRequired("plexus-config does not exist:%s"%self.plxconfig_cmd)

      cfg_cmd_parser = SConsAddons.Util.ConfigCmdParser(self.plxconfig_cmd)

      # check version requirement

      found_ver_str = cfg_cmd_parser.getVersion()
      req_ver = [int(n) for n in self.requiredVersion.split(".")]
      found_ver = [int(n) for n in found_ver_str.split(".")]
      if found_ver < req_ver:
         passed = False
         self.checkRequired("   Plexus version is too old! Required %s but found %s"%(self.requiredVersion, found_verStr))

      plx_header_file = pj(self.baseDir, 'include', 'plx', 'plxConfig.h')
      if not os.path.isfile(plx_header_file):
         passed = False
         self.checkRequired("plxConfig.h not found:%s"%plx_header_file)

      self.found_incs = cfg_cmd_parser.findIncludes(" --cxxflags")
      self.found_libs = cfg_cmd_parser.findLibs()
      self.found_lib_paths = cfg_cmd_parser.findLibPaths()

      
      if not passed:
         self.baseDir = None
         self.plx_cfg_cmd = None
         edict = env.Dictionary()
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey]
         self.found_incs = None
         self.found_libs = None
         self.found_lib_paths = None
      else:
         self.available = True

   def apply(self, env):
      """ Add environment options for building against plexus"""
      if self.found_incs:
         env.Append(CPPPATH = self.found_incs)
      if self.found_libs:
         env.Append(LIBS = self.found_libs)
      if self.found_lib_paths:
         env.Append(LIBPATH = self.found_lib_paths)

   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),]

   def dumpSettings(self):
      "Write out the settings"
      print "PlxBaseDir:", self.baseDir
      print "plexus-config:", self.plxconfig_cmd
      print "CPPPATH:", self.found_incs
      print "LIBS:", self.found_libs
      print "LIBPATH:", self.found_lib_paths
