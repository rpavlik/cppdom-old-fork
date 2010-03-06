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
import sys, os, re, string

from SCons.Util import WhereIs
pj = os.path.join;

import SCons.SConf
Configure = SCons.SConf.SConf     # Use same alias as SConsctruct sees


class CppDom(SConsAddons.Options.PackageOption):
   def __init__(self, name, requiredVersion, required=True, 
                autoLink=True, preferDynamic=True):
      """
         name - The name to use for this option
         requiredVersion - The version of cppdom required (ex: "0.1.0")
         required - Is the dependency required?  (if so we exit on errors)
         autoLink - Attempt to use autolinking on windows (default: True)
         preferDynamic - If true, prefer to use shared libraries.
      """
      help_text = """Base directory for cppdom. bin, include, and lib should be under this directory""";
      self.baseDirKey = "CppDomBaseDir";
      self.requiredVersion = requiredVersion;
      self.required = required;
      self.available = False
      self.autoLink = autoLink
      self.preferDynamic = preferDynamic      
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey, help_text);
      
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

   def startProcess(self):
      print "Checking for cppdom...",

   def setInitial(self, optDict):
      " Set initial values from given dictionary. "
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
         assert self.baseDir
         return
      
      # Find cppdom-config and call it to get the other arguments
      sys.stdout.write("searching...\n");
      self.cppdomconfig_cmd = WhereIs('cppdom-config');
      if None == self.cppdomconfig_cmd:
         self.checkRequired("   could not find cppdom-config.");
      else:
         sys.stdout.write("   found cppdom-config: %s"%self.cppdomconfig_cmd);
         # find base dir
         self.baseDir = os.popen(self.cppdomconfig_cmd + " --prefix").read().strip();
         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"% self.baseDir);
            self.baseDir = None;
         else:
            print "   found at: ", self.baseDir;
   
   def validate(self, env):
      # Check that path exist
      # Check that vpr-config exist
      # Check version is correct
      # Check that an include file: include/vpr/vprConfig.h  exists
      # Update the temps for later usage
      passed = True
      if not os.path.isdir(self.baseDir):
         passed = False
         self.checkRequired("cppdom base dir does not exist:%s"%self.baseDir)
      
      # If cppdom-config exists and we are not on windows, use it.
      has_config_cmd = os.path.isfile(self.cppdomconfig_cmd) and \
         not SConsAddons.Util.GetPlatform() == "win32"

      if not has_config_cmd:
         print "Can not find %s. Limping along without it."%self.cppdomconfig_cmd
      else:
         cfg_cmd_parser = SConsAddons.Util.ConfigCmdParser(self.cppdomconfig_cmd)           
      
      # -- Find header directory -- #
      inc_dir = None
      header_file = pj('cppdom', 'cppdom.h')
      base_include = pj(self.baseDir,'include')
      # check standard directory first
      if os.path.isfile(pj(base_include, header_file)):
         inc_dir = base_include
      # check versioned directories by building a list and sorting them
      elif os.path.exists(base_include):
         pot_dirs = [pj(base_include,d) for d in os.listdir(base_include)\
                                              if d.count("cppdom")]
         pot_dirs.sort()
         pot_dirs.reverse()         
         for d in pot_dirs:
            if os.path.isfile(pj(d,header_file)):
               inc_dir = d
               break
         
      if not inc_dir:
         passed = False
         self.checkRequired("cppdom.h not found in any cppdom directories.")
      else:
         print "   Found cppdom include directory: ", inc_dir

      # --- Check version requirement --- #
      req_ver = [int(n) for n in self.requiredVersion.split(".")]
      version_header = pj(inc_dir,'cppdom','version.h')
      if not os.path.isfile(version_header):
         passed = False
         self.checkRequired("%s does not exist.  Can not determin gmtl version."%version_header)
      found_ver = GetCppDomVersion(version_header)
      found_ver_str = '.'.join([str(i) for i in found_ver])
      
      print "   Found cppdom version: ", found_ver_str
      if found_ver < req_ver:
         passed = False
         self.checkRequired("   CppDOM version is too old! Required %s but found %s"%(self.requiredVersion,found_ver_str))
      lib_ver_str = '_'.join([str(i) for i in found_ver])         
      
      # --- Build flag settings --- #         
      if has_config_cmd:
         # Returns lists of the options we want
         print "%s %s" % (inc_dir, cfg_cmd_parser.findIncludes(" --cxxflags"))
         self.found_incs = [inc_dir,] + cfg_cmd_parser.findIncludes(" --cxxflags")
         self.found_libs = cfg_cmd_parser.findLibs()
         self.found_lib_paths = cfg_cmd_parser.findLibPaths()
      else:
         self.found_incs = [inc_dir,]         
         self.found_lib_paths = [pj(self.baseDir,'lib'),]
         # On win32 we go with autolinking
         if SConsAddons.Util.GetPlatform() == "win32" and self.autoLink:
            self.found_libs = []
         else:
            #assert False, "cppdom options doesn't currently know how to get correct version without cppdom-config"
            self.found_libs = ['cppdom-%s'%lib_ver_str]         
      
      # Try to build against the library
      conf_env = env.Clone();                     # Make a copy of the env
      self.apply(conf_env);                  # Update it with the guessed values
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
      
             
   def apply(self, env):
      """ Add environment options for building against cppdom"""
      if self.found_incs:
         env.AppendUnique(CPPPATH = self.found_incs);
      if self.found_libs:
         env.AppendUnique(LIBS = self.found_libs);
      if self.found_lib_paths:
         env.AppendUnique(LIBPATH = self.found_lib_paths);
      if self.autoLink and SConsAddons.Util.GetPlatform() == "win32":
         env.AppendUnique(CPPDEFINES = ['CPPDOM_AUTO_LINK'])
         if self.preferDynamic:
            env.AppendUnique(CPPDEFINES = ['CPPDOM_DYN_LINK'])
   
   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),]

   def dumpSettings(self):
      "Write out the settings"
      print "CppDomBaseDir:", self.baseDir
      print "cppdom-config:", self.cppdomconfig_cmd
      print "CPPPATH:", self.found_incs
      print "LIBS:", self.found_libs
      print "LIBPATH:", self.found_lib_paths
      
def GetCppDomVersion(versionHeader):
   """Gets the CppDom version from cppdom/version.h.
      Returns version as tuple (major,minor,patch)
   """
   contents = open(versionHeader, 'r').read()
   major = re.compile('.*(#define *CPPDOM_VERSION_MAJOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   minor = re.compile('.*(#define *CPPDOM_VERSION_MINOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   patch = re.compile('.*(#define *CPPDOM_VERSION_PATCH *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   return (int(major), int(minor), int(patch))
      
