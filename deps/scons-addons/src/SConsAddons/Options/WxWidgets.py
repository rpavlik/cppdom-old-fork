"""SConsAddons.Options.WxWidgets

Defines options for the WxWidgets project
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

import SCons.Environment   # Get the environment interface
import SCons
import SConsAddons.Options   # Get the modular options stuff
import SConsAddons.Util
import sys, os, re, string

from SCons.Util import WhereIs
pj = os.path.join

import SCons.SConf
Configure = SCons.SConf.SConf     # Use same alias as SConsctruct sees


class WxWidgets(SConsAddons.Options.PackageOption):
   def __init__(self, name, requiredVersion, required=True, useCppPath=False):
      """
         name - The name to use for this option
         requiredVersion - The version of cppunit required (ex: "0.1.0")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, put the include paths in cpppath else, put them in cxxflags.
      """
      help_text = """Base directory for cppunit. bin, include, and lib should be under this directory""";
      self.baseDirKey = "WxWidgetsBaseDir"
      self.requiredVersion = requiredVersion
      self.required = required
      self.available = False
      self.useCppPath = useCppPath
      self.found_cxxflags = None
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey, help_text)
      
      # configurable options
      self.baseDir = None
      self.wxwidgetsconfig_cmd = None
      self.variant_libs = {}
      
   def checkRequired(self, msg):
      """ Called when there is config problem.  If required, then exit with error message """
      print msg;
      if self.required:
         sys.exit(0)
      else:
         sys.stdout.write("WxWidgets not available. Skipping it...\n")
         
   def isAvailable(self):
      return self.available
   
   def startProcess(self):
      print "Checking for WxWidgets...",
      
   def setInitial(self, optDict):
      " Set initial values from given dict "
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey];
         self.wxwidgetsconfig_cmd = pj(self.baseDir, 'bin', 'wx-config')
         print "   %s specified or cached. [%s]." % (self.baseDirKey,self.baseDir);
        
   def find(self, env):
      # Quick exit if nothing to find because it is already specified
      if self.baseDir != None:
         assert self.baseDir
         assert self.wxwidgetsconfig_cmd
         return
      
      # Find cppunit-config and call it to get the other arguments
      sys.stdout.write("searching...\n");
      self.wxwidgetsconfig_cmd = WhereIs('wx-config');
      if None == self.wxwidgetsconfig_cmd:
         self.checkRequired("   could not find wx-config. Use %s to specify it: Ex: %s=/usr/local"% 
                           (self.baseDirKey, self.baseDirKey) );
      else:
         sys.stdout.write("   found wx-config.\n");
         found_ver_str = os.popen(self.wxwidgetsconfig_cmd + " --version").read().strip();
         sys.stdout.write("   version:%s"%found_ver_str);
         
         # find base dir
         self.baseDir = os.popen(self.wxwidgetsconfig_cmd + " --prefix").read().strip();
         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"% self.baseDir);
            self.baseDir = None;
         else:
            print "   found at: ", self.baseDir;
   
   def validate(self, env):
      # Check that path exist
      # Check that wx-config exist
      # Check version is correct
      # Check that an include file exists
      # Update the temps for later usage
      passed = True
      if not self.baseDir:
         self.checkRequired("wxwidgets base dir not found");
         return
      if not os.path.isdir(self.baseDir):
         passed = False
         self.checkRequired("wxwidgets base dir does not exist:%s"%self.baseDir);
         return
      has_config_cmd = os.path.isfile(self.wxwidgetsconfig_cmd)
      
      if not has_config_cmd:
         self.checkRequired("Can not find %s. "%self.wxwidgetsconfig_cmd);
      else:
         cfg_cmd_parser = SConsAddons.Util.ConfigCmdParser(self.wxwidgetsconfig_cmd)

      # Check version requirement
      if has_config_cmd:
         found_ver_str = cfg_cmd_parser.getVersion()
         req_ver = [int(n) for n in self.requiredVersion.split(".")];
         found_ver = [int(n) for n in found_ver_str.split(".")];
         if found_ver < req_ver:
            passed = False;
            self.checkRequired("   wxWidgets version is too old! Required %s but found %s"%(self.requiredVersion,found_ver_str))

      # Returns lists of the options we want
      self.found_incs = cfg_cmd_parser.findIncludes(" --cxxflags")
      self.found_cxxflags = cfg_cmd_parser.findCXXFlags()
      self.found_libs = cfg_cmd_parser.findLibs()
      self.found_lib_paths = cfg_cmd_parser.findLibPaths()

      # Create list of flags that may be needed later
      self.found_incs_as_flags = [env["INCPREFIX"] + p for p in self.found_incs];
            
      # Try to build against the library
      conf_env = env.Clone()                # Make a copy of the env
      self.apply(conf_env)                  # Update it with the guessed values
      conf_ctxt = Configure(conf_env);
      if not conf_ctxt.CheckCXXHeader(pj("wx", "setup.h")):
         passed = False;
         self.checkRequired("Can't compile with wx/wx.h");
      if len(self.found_libs):
         if not conf_ctxt.CheckLib(self.found_libs[0], autoadd=0):
            passed = False
            self.checkRequired("Can't compile with wx.")
         
      conf_ctxt.Finish()
     
      # If we don't pass, then clear everything out
      if not passed:
         self.baseDir = None;
         self.cppunitconfig_cmd = None
         edict = env.Dictionary()
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey]
         self.found_incs = None
         self.found_libs = None
         self.found_lib_paths = None
      else:
         self.available = True

             
   def apply(self, env):
      """ Add environment options for building against us"""
      if self.found_incs:
         if self.useCppPath:
            env.Append(CPPPATH = self.found_incs)
         else:
            env.Append(CXXFLAGS = self.found_incs_as_flags)
      if self.found_lib_paths:
         env.Append(LIBPATH = self.found_lib_paths);

      if self.found_libs:
         env.Append(LIBS = self.found_libs)

      if self.found_cxxflags:
         env.Append(CPPDEFINES = self.found_cxxflags)

   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),]

   def dumpSettings(self):
      "Write out the settings"
      print "WxWidgetsBaseDir:", self.baseDir;
      print "wx-config:", self.wxwidgetsconfig_cmd;
      print "CPPPATH:", self.found_incs;
      print "LIBS:", self.found_libs;
      print "LIBPATH:", self.found_lib_paths;

