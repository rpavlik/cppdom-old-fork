"""SConsAddons.Options.PyJuggler

Defines options for PyJuggler project
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

class PyJuggler(SConsAddons.Options.PackageOption):
   """
   Options object for capturing PyJuggler options and dependencies
   """
   def __init__(self, name, requiredVersion, required=True):
      """
         name - The name to use for this option
         requiredVersion - the version of PyJuggler required (ex: "0.3.3")
         required - Is the dependency required? (if so we exit on errors)
      """
      help_text = """Base directory for PyJuggler. lib and include should be under this directory."""
      self.baseDirKey = "PyJugglerBaseDir"
      self.requiredVersion = requiredVersion
      self.required = required
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey, help_text)

      self.baseDir = None;

   def checkRequired(self, msg):
      """ Called when there is a config error.  If required, then exits with
      error message """
      print msg
      if self.required:
         sys.exit(1)

   def setInitial(self, optDict):
      " Set initial values from given dictionary "
      sys.stdout.write("checking for PyJuggler...")
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         #sys.stdout.write("specified or cached. [%s].\n"% self.baseDir)

   def find(self, env):
      # quick exit if nothing to find
      if self.baseDir !=None:
         return

      sys.stdout.write("Find not supported for PyJuggler.  Must specify manually.\n")

      #if not os.path.isdir(self.baseDir):
      #   self.checkRequired("   returned directory does not exist:%s"%self.baseDir)
      #   self.baseDir = None;
      #else:
      #   print "   found at: ", self.baseDir
      
   
   def validate(self, env):
      # check path existance
      # check include/pyjutil/InterpreterGuard.h existance
      # check version correctness
      # update the temps for later usage
      passed = True
      if self.baseDir == None or not os.path.isdir(self.baseDir):
         passed = False
         self.checkRequired("PyJuggler base dir does not exist:%s"%self.baseDir)

      pyjuggler_header_file = pj(self.baseDir, 'include', 'pyjutil', 'InterpreterGuard.h')
      if not os.path.isfile(pyjuggler_header_file):
         passed = False
         self.checkRequired("InterpreterGuard.h not found:%s"%pyjuggler_header_file)

      self.found_incs = pj(self.baseDir, 'include')
      self.found_libs = ['pyjutil']
      self.found_lib_paths = pj(self.baseDir, 'lib')

      if not passed:
         self.baseDir = None
         edict = env.Dictionary()
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey]
         self.found_incs = None
         self.found_libs = None
         self.found_lib_paths = None
      else:
         self.available = True
         print "[OK]"

   def apply(self, env):
      """ Add environment options for building against pyjuggler"""
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
      print "PyJugglerBaseDir:", self.baseDir
      print "CPPPATH:", self.found_incs
      print "LIBS:", self.found_libs
      print "LIBPATH:", self.found_lib_paths
