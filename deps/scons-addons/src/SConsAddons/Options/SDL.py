"""SConsAddons.options.SDL

Defines options for SDL projects
"""

__revision__ = "__FILES__ __REVISION__ __DATE__ __DEVELOPER__"

import SCons.Environment
import SCons
import SConsAddons.Options
import SCons.Util
import SConsAddons.Util
import sys,os,re,string

if SConsAddons.Util.GetPlatform()=='darwin':
   True = 1
   False = 0

from SCons.Util import WhereIs
pj = os.path.join

import SCons.SConf
Configure = SCons.SConf.SConf

class SDL(SConsAddons.Options.PackageOption):
   """
   Options object for capturing SDL options and dependencies
   """

   def __init__(self, name, requiredVersion, required):
      """
         name - The name to use for this option
         requiredVersion - the version of SDL required
         required - Is the dependency required?
      """
      help_text = """Base directory for SDL. bin, include and lib should be uner this dir."""
      self.baseDirKey = "sdlBaseDir"
      self.requiredVersion = requiredVersion
      self.required = required
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey, help_text)

      self.baseDir = None;
      self.sdlconfig_cmd = None;

   def isAvailable(self):
      return self.available

   def checkRequired(self, msg):
      """ Called when there is a config error.  if required, then exits with error message """
      print msg
      if self.required:
         sys.exit(1)

   def setInitial(self, optDict):
      " set inital values from given dictionary "
      sys.stdout.write("checking for sdl...")
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         self.sdlconfig_cmd = pj(self.baseDir, 'bin', 'sdl-config')
         sys.stdout.write("specified or cdached. [%s].\n"% self.baseDir)

   def find(self, env):
      # quick exit if nothing to find
      if self.baseDir != None:
         return

      sys.stdout.write("searching...\n")
      self.sdlconfig_cmd = WhereIs('sdl-config')
      if None == self.sdlconfig_cmd:
         self.checkRequired("   Could not find sdl-config")
      else:
         sys.stdout.write("    found sdl-config.\n")
         found_ver_str = os.popen(self.sdlconfig_cmd + " --version").read().strip()
         sys.stdout.write("   version: %s"%found_ver_str)

         self.baseDir = os.popen(self.sdlconfig_cmd + "  --prefix").read().strip()

         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist: %s"%self.baseDir)

            self.baseDir = None;
         else:
            print "   found at: ", self.baseDir
   
   def validate(self, env):
      passed = True
      if not os.path.isdir(self.baseDir):
         passed = False
         self.checkRequired("sdl base dir does not exist: %s"%self.baseDir)
      if not os.path.isfile(self.sdlconfig_cmd):
         passed=False;
         self.checkRequired("sdl-config does not exist:%s"%self.sdlconfig_cmd)
      cfg_cmd_parser = SConsAddons.Util.ConfigCmdParser(self.sdlconfig_cmd)

      found_ver_str = cfg_cmd_parser.getVersion()
      req_ver = [int(n) for n in self.requiredVersion.split(".")]
      found_ver = [int(n) for n in found_ver_str.split(".")]
      if found_ver < req_ver:
         passed = False
         self.checkRequired("   SDL version is too old! Required %s but found %s"%(self.requiredVersion, found_ver_str))

      sdl_header_file = pj(self.baseDir, 'include', 'SDL', 'SDL.h')
      if not os.path.isfile(sdl_header_file):
         passed = False
         self.checkRequired("sdlConfig.h not found:%s"%sdl_header_file)
      self.found_incs = cfg_cmd_parser.findIncludes("--cflags")
      self.found_libs = cfg_cmd_parser.findLibs()
      self.found_lib_paths = cfg_cmd_parser.findLibPaths()
      

      if not passed:
         self.baseDir = None
         self.sdl_cfg_cmd = none
         edict = env.dictionary()
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey]
         self.found_incs = None
         self.found_libs = None
         self.found_lib_path = None
      else:
         self.available = True

   def apply(self, env):
      """ Add environment options for building against sdl"""
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
      print "SDLbaseDir:", self.baseDir
      print "sdl-config:", self.sdlconfig_cmd
      print "CPPPATH:", self.found_incs
      print "LIBS:", self.found_libs
      print "LIBPATH:", self.found_lib_paths

