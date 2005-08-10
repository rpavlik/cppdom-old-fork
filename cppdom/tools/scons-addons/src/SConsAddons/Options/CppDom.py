"""SConsAddons.Options.CppDom

Defines options for the CppDom project
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

import SCons.Environment;   # Get the environment crap
import SCons;
import SCons.Util
import SConsAddons.Options;   # Get the modular options stuff
import SConsAddons.Util
import sys;
import os;
import re;
import string;

from SCons.Util import WhereIs
pj = os.path.join;

import SCons.SConf
Configure = SCons.SConf.SConf     # Use same alias as SConsctruct sees


class CppDom(SConsAddons.Options.PackageOption):
   def __init__(self, name, requiredVersion, required=True):
      """
         name - The name to use for this option
         requiredVersion - The version of cppdom required (ex: "0.1.0")
         required - Is the dependency required?  (if so we exit on errors)
      """
      help_text = """Base directory for cppdom. bin, include, and lib should be under this directory""";
      self.baseDirKey = "CppDomBaseDir";
      self.requiredVersion = requiredVersion;
      self.required = required;
      self.available = False
      SConsAddons.Options.LocalUpdateOption.__init__(self, name, self.baseDirKey, help_text);
      
      # configurable options
      self.baseDir = None;
      self.cppdomconfig_cmd = None;
      
   def checkRequired(self, msg):
      """ Called when there is config problem.  If required, then exit with error message """
      print msg;
      if self.required:
         sys.exit(0);
         
   def isAvailable(self):
      return self.available

   def startUpdate(self):
      print "Checking for cppdom...",

   def setInitial(self, optDict):
      " Set initial values from given dict "
      if self.verbose:
         print "   Loading initial cppdom settings."
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey];
         self.cppdomconfig_cmd = pj(self.baseDir, 'bin', 'cppdom-config')
         if self.verbose:
            print "   %s specified or cached. [%s]."% (self.baseDirKey, self.baseDir);
        
   def find(self, env):
      # Quick exit if nothing to find because it is already specified
      if self.baseDir != None:
         assert self.baseDir;
         assert self.cppdomconfig_cmd;
         return;
      
      # Find cppdom-config and call it to get the other arguments
      sys.stdout.write("searching...\n");
      self.cppdomconfig_cmd = WhereIs('cppdom-config');
      if None == self.cppdomconfig_cmd:
         self.checkRequired("   could not find cppdom-config.");
      else:
         sys.stdout.write("   found cppdom-config.\n");
         found_ver_str = os.popen(self.cppdomconfig_cmd + " --version").read().strip();
         sys.stdout.write("   version:%s"%found_ver_str);
         
         # find base dir
         self.baseDir = os.popen(self.cppdomconfig_cmd + " --prefix").read().strip();
         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"% self.baseDir);
            self.baseDir = None;
         else:
            print "   found at: ", self.baseDir;
   
   def convert(self):
      pass;
   
   def set(self, env):
      if self.baseDir:
         env[self.baseDirKey] = self.baseDir;
   
   def validate(self, env):
      # Check that path exist
      # Check that vpr-config exist
      # Check version is correct
      # Check that an include file: include/vpr/vprConfig.h  exists
      # Update the temps for later usage
      passed = True;
      if not os.path.isdir(self.baseDir):
         passed = False;
         self.checkRequired("cppdom base dir does not exist:%s"%self.baseDir);
      if not os.path.isfile(self.cppdomconfig_cmd):
         passed = False;
         self.checkRequired("cppdom-config does not exist:%s"%self.cppdomconfig_cmd);

      cfg_cmd_parser = SConsAddons.Util.ConfigCmdParser(self.cppdomconfig_cmd)
      
      # Check version requirement
      found_ver_str = cfg_cmd_parser.getVersion()
      req_ver = [int(n) for n in self.requiredVersion.split(".")];
      found_ver = [int(n) for n in found_ver_str.split(".")];
      if found_ver < req_ver:
         passed = False;
         self.checkRequired("   found version is to old: required:%s found:%s"%(self.requiredVersion,found_ver_str));             
      
      # Check header file
      cppdom_header_file = pj(self.baseDir, 'include', 'cppdom', 'cppdom.h');
      if not os.path.isfile(cppdom_header_file):
         passed = False;
         self.checkRequired("cppdom.h not found:%s"%cppdom_header_file);
         
      # Returns lists of the options we want
      self.found_incs = cfg_cmd_parser.findIncludes(" --cxxflags")
      self.found_libs = cfg_cmd_parser.findLibs()
      self.found_lib_paths = cfg_cmd_parser.findLibPaths()
      
      # Try to build against the library
      conf_env = env.Copy();                     # Make a copy of the env
      self.updateEnv(conf_env);                  # Update it with the guessed values
      conf_ctxt = Configure(conf_env);
      if not conf_ctxt.CheckCXXHeader(pj("cppdom", "cppdom.h")):
         passed = False;
         self.checkRequired("Can't compile with cppdom.h");
#      if not conf_ctxt.CheckLibWithHeader(library=None, header="cppdom/cppdom.h", language="c++",
#                                          call = "cppdom::ContextPtr ctx( new cppdom::Context );", autoadd=0):
#         passed = False;
#         self.checkRequired("Can't compile with cppdom.");
         
      conf_ctxt.Finish();
     
      # If we don't pass, then clear everything out
      if not passed:
         self.baseDir = None;
         self.cppdom_cfg_cmd = None;
         edict = env.Dictionary();
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey];
         self.found_incs = None;
         self.found_libs = None;
         self.found_lib_paths = None;
      else:
         self.available = True
         print "[OK]"
      
             
   def updateEnv(self, env):
      """ Add environment options for building against vapor"""
      if self.found_incs:
         env.Append(CPPPATH = self.found_incs);
      if self.found_libs:
         env.Append(LIBS = self.found_libs);
      if self.found_lib_paths:
         env.Append(LIBPATH = self.found_lib_paths);
         
   def dumpSettings(self):
      "Write out the settings"
      print "CppDomBaseDir:", self.baseDir;
      print "cppdom-config:", self.cppdomconfig_cmd;
      print "CPPPATH:", self.found_incs;
      print "LIBS:", self.found_libs;
      print "LIBPATH:", self.found_lib_paths;
