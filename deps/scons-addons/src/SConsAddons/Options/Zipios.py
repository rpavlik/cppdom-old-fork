"""SConsAddons.Options.Zipios

Defines options for zipios project
"""

#
# __COPYRIGHT__
#
# This file is part of scons-addons.
#
# Scons-addons is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Scons-addons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with scons-addons; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

import SCons.Environment   # Get the environment crap
import SCons
import SConsAddons.Options   # Get the modular options stuff
import SCons.Util
import sys
import os
import re
import string
import types

from SCons.Util import WhereIs
pj = os.path.join


class Zipios(SConsAddons.Options.PackageOption):
   """ 
   Options object for capturing vapor options and dependencies.
   """

   def __init__(self, name, requiredVersion, required=True, useCppPath = False):
      """
         name - The name to use for this option
         requiredVersion - The version of vapor required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - Should includes be put in cpp path environment
      """
      help_text = """Base directory for zipios."""
      self.baseDirKey = "ZipiosBaseDir";
      self.requiredVersion = requiredVersion;
      self.required = required;
      self.useCppPath = useCppPath
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey, help_text);
      
      # configurable options
      self.baseDir = None;

      # Settings to use
      self.found_libs = None;
      self.found_cflags = None;
      self.found_lib_paths = None;
      self.found_defines = None;
      
   def checkRequired(self, msg):
      """ Called when there is config problem.  If required, then exit with error message """
      print msg;
      if self.required:
         sys.exit(1);

   def startProcess(self):
      print "Checking for zipios...",
      
   def setInitial(self, optDict):
      " Set initial values from given dict "
      if self.verbose:
         print "   Setting initial zipios settings."
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey];
         if self.verbose:
            print "   %s specified or cached. [%s]."% (self.baseDirKey, self.baseDir);
        
   def find(self, env):
      # Quick exit if nothing to find
      if self.baseDir != None:
         return;

      ver_header = None

      # Find zipios++/zipios-config.h
      print "   searching for zipios..."
      if env.Dictionary().has_key('CPPPATH'):
         print "      Searching CPPPATH..."
         ver_header = env.FindFile(pj('zipios++','zipios-config.h'), env['CPPPATH'])

      if (None == ver_header) and env.Dictionary().has_key('CPLUS_INCLUDE_PATH'):
         print "      Searching CPLUS_INCLUDE_PATH..."
         ver_header = SCons.Script.SConscript.FindFile(pj('zipios++', 'zipios-config.h'),
                                    string.split(env['ENV']['CPLUS_INCLUDE_PATH'], os.pathsep))
         
      if None == ver_header:
         self.checkRequired("   could not find zipios-config.h.")
      else:
         ver_header = str(ver_header)
         print "   found zipios++/zipios-config.h.\n"

         # find base dir
         self.baseDir = os.path.dirname(os.path.dirname(os.path.dirname(ver_header)))
         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"% self.baseDir)
            self.baseDir = None
         else:
            print "   found at: ", self.baseDir
   
   def validate(self, env):
      # Check that path exist
      # Update the temps for later usage
      passed = True;

      if not self.baseDir:
         self.checkRequired("zipios base dir not set")
         return
      if not os.path.isdir(self.baseDir):
         self.checkRequired("zipios base dir does not exist:%s"%self.baseDir);
         return

      # Check the version header is there         
      version_header = pj(self.baseDir, 'include', 'zipios++', 'zipios-config.h')
      if not os.path.isfile(version_header):
         self.checkRequired("zipios-config.h header does not exist:%s"%version_header)
         return

      # --- Check version requirement --- #
      ver_file = file(version_header)
      ver_match = re.search('#define\s*VERSION\s*\"(.*)\"', ver_file.read())
      if not ver_match:
         self.checkRequired("   could not find VERSION in file: %s"%version_header)
         return


      found_ver_str = ver_match.group(1)
      req_ver = [int(n) for n in self.requiredVersion.split('.')]
      found_ver = [int(n) for n in found_ver_str.split('.')]
      if found_ver < req_ver:
         self.checkRequired("   Zipios version is too old! Required %s but found %s"%(self.requiredVersion,found_ver_str))
         return

         
      # If not pass, then clear everything
      # Else we pass, set up the real data structures to use (initialized in constructor)      
      if not passed:
         # Clear everything
         self.baseDir = None;
         edict = env.Dictionary();
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey];
      else:
         self.available = True         
         print "[OK]"
         
   def apply(self, env):
      """ Update the passed environment with full settings for the option """

      include_path = pj(self.baseDir, 'include')
      lib_path = pj(self.baseDir, 'lib')
      include_path_as_flags = env["INCPREFIX"] + include_path

      if self.useCppPath:
         env.Append(CPPPATH = include_path)
      else:
         env.Append(CXXFLAGS = include_path_as_flags)
      env.Append(LIBPATH = lib_path)
      env.Append(LIBS = ['zipios'])


   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),]


   def dumpSettings(self):
      "Write out the settings"
      print "ZipiosBaseDir:", self.baseDir;
