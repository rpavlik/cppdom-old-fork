"""SConsAddons.Options.VRJuggler.JugglerCommon

Defines common options structure for VR Juggler project
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

import SCons.Environment;     # Get the environment stuff
import SCons;
import SConsAddons.Options;   # Get the modular options stuff
import SCons.Util
import sys, os, re, string

from SCons.Util import WhereIs
pj = os.path.join;


class JugglerCommon(SConsAddons.Options.LocalUpdateOption):
   """ 
   Options object for capturing common options and dependencies for VR Juggler projects
   """

   def __init__(self, name, requiredVersion, required, useCppPath, helpText,):
      """
         name - The name to use for this option
         requiredVersion - The version of project required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, put the include paths in cpppath else, put them in cxxflags.
         helpText - The help text to use for it all.
      """
      # Common settings needed for all tools (these must be setup in derived classes
      required_attrs = ['baseDirKey',
                        'optionName',
                        'configCmdName',
                        'filesToCheckRelBase']
      for a in required_attrs:
         assert hasattr(self,a), "JugglerCommon needs attribute [%s] set before calling it's init."%a
         
      #self.baseDirKey = "NoneBaseDir"
      #self.optionName = "No option name"
      #self.configCmdName = 'vrj-config'
      #self.filesToCheckRelBase = [pj('include','vpr','vprConfig.h'), ... ]
      
      SConsAddons.Options.LocalUpdateOption.__init__(self, name, self.baseDirKey, helpText);
            
      # Local options
      self.baseDir = None
      self.configCmdFullPath = None
      self.requiredVersion = requiredVersion
      self.required = required
      self.useCppPath = useCppPath      
      
      
   def checkRequired(self, msg):
      """ Called when there is config problem.  If required, then exit with error message """
      print msg;
      if self.required:
         sys.exit(1);

   def startUpdate(self):
      print "Checking for %s..."%self.optionName,
      
   def setInitial(self, optDict):
      " Set initial values from given dict "
      if self.verbose:
         print "   Setting initial %s settings."%self.optionName
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey];
         self.configCmdFullPath = pj(self.baseDir, 'bin', self.configCmdName)
         if self.verbose:
            print "   %s specified or cached. [%s]."% (self.baseDirKey,self.baseDir);
        
   def find(self, env):
      # Quick exit if nothing to find
      if self.baseDir != None:
         return;
      
      # Find cmd-config and call it to get the other arguments
      sys.stdout.write("searching...\n");
      self.configCmdFullPath = WhereIs(self.configCmdName);
      if None == self.configCmdFullPath:
         self.checkRequired("   could not find %s."%self.configCmdName);
      else:
         sys.stdout.write("   found %s.\n"%self.configCmdName);
         found_ver_str = os.popen(self.configCmdFullPath + " --version").read().strip();
         sys.stdout.write("   version:%s"%found_ver_str);
         
         # find base dir
         self.baseDir = os.popen(self.configCmdFullPath + " --prefix").read().strip();
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
      # Check that cmd-config exist
      # Check that an include file: include/vpr/vprConfig.h  exists
      # Update the temps for later usage
      passed = True;
      if not os.path.isdir(self.baseDir):
         passed = False;
         self.checkRequired("vrj base dir does not exist:%s"%self.baseDir);
      if not os.path.isfile(self.configCmdFullPath):
         passed = False;
         self.checkRequired("%s does not exist:%s"%(self.configCmdName,self.configCmdFullPath));
         
      # Check version requirement
      found_ver_str = os.popen(self.configCmdFullPath + " --version").read().strip();
      req_ver = [int(n) for n in self.requiredVersion.split(".")];
      found_ver = [int(n) for n in found_ver_str.split(".")];
      if found_ver < req_ver:
         passed = False;
         self.checkRequired("   found version is to old: required:%s found:%s"%(self.requiredVersion,found_ver_str));
         
      # Set of files to check for
      for f in self.filesToCheckRelBase:
         check_file = pj(self.baseDir, f);
         if not os.path.isfile(check_file):
            passed = False;
            self.checkRequired("%s not found:%s"%(f,check_file));
         
      self.found_incs = None;
      self.found_libs = None;
      self.found_lib_paths = None;
      self.found_link_from_libs = None;
         
      if not passed:
         # Clear everything and remove the key from the environment
         self.baseDir = None;
         self.configCmdFullPath = None;
         edict = env.Dictionary();
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey];
      else:
         # Get output from cmd-config
         # Res that when matched against vrj-config output should match the options we want
         # In future could try to use INCPREFIX and other platform neutral stuff
         inc_re = re.compile(r'-I(\S*)', re.MULTILINE);
         lib_re = re.compile(r'-l(\S*)', re.MULTILINE);
         lib_path_re = re.compile(r'-L(\S*)', re.MULTILINE);
         link_from_lib_re = re.compile(r'((?: |^)-[^lL]\S*)', re.MULTILINE);
         
         # Returns lists of the options we want
         self.found_incs = inc_re.findall(os.popen(self.configCmdFullPath + " --includes").read().strip());
         self.found_libs = lib_re.findall(os.popen(self.configCmdFullPath + " --libs --extra-libs").read().strip());
         self.found_lib_paths = lib_path_re.findall(os.popen(self.configCmdFullPath + " --libs --extra-libs").read().strip());
         self.found_link_from_libs = link_from_lib_re.findall(os.popen(self.configCmdFullPath + " --extra-libs").read().strip());

         # Create list of flags that may be needed later
         self.found_incs_as_flags = [env["INCPREFIX"] + p for p in self.found_incs];
         
         print "[OK]"
             
   def updateEnv(self, env, useCppPath=False):
      """ Add environment options for building against vapor"""
      if self.found_incs:
         if self.useCppPath or useCppPath:
            env.Append(CPPPATH = self.found_incs)
         else:
            env.Append(CXXFLAGS = self.found_incs_as_flags)
      if self.found_libs:
         env.Append(LIBS = self.found_libs)
      if self.found_lib_paths:
         env.Append(LIBPATH = self.found_lib_paths)
      if self.found_link_from_libs:
         env.Append(LINKFLAGS = " ".join(self.found_link_from_libs))
         
   def dumpSettings(self):
      "Write out the settings"
      print "%s: %s", (self.baseDirKey, self.baseDir)
      print "%s: %s"%(self.configCmdName,self.configCmdFullPath)
      print "CPPPATH:", self.found_incs
      print "LIBS:", self.found_libs
      print "LIBPATH:", self.found_lib_paths
      print "LINKFLAGS:", self.found_link_from_libs
