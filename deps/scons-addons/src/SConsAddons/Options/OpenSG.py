"""SConsAddons.Options.OpenSG

Defines options for OpenSG project
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
import SConsAddons.Util as sca_util
import SCons.Util
import sys
import os
import re
import string
import types

from SCons.Util import WhereIs
pj = os.path.join


class OpenSG(SConsAddons.Options.PackageOption):
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
      help_text = ["Base directory for OpenSG. (bin should be under this directory for osg-config).",
                   "Optional directory that contains OpenSG dependencies. include and lib will be added to paths."]
      self.baseDirKey = "OpenSGBaseDir"
      self.depDirKey = "OpenSGDepDir"
      self.requiredVersion = requiredVersion
      self.required = required
      self.useCppPath = useCppPath
      SConsAddons.Options.PackageOption.__init__(self, name, 
                              [self.baseDirKey,self.depDirKey], 
                              help_text)
      
      # configurable options
      self.baseDir = None
      self.depDir = None
      self.osgconfig_cmd = None

      # Settings to use
      self.found_libs = None
      self.found_cflags = None
      self.found_lib_paths = None
      self.found_defines = None
      
   def checkRequired(self, msg):
      """ Called when there is config problem.  If required, then exit with error message """
      print msg;
      if self.required:
         sys.exit(1);

   def startProcess(self):
      print "Checking for OpenSG...",
      
   def setInitial(self, optDict):
      " Set initial values from given dict "
      if self.verbose:
         print "   Setting initial OpenSG settings."
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         self.osgconfig_cmd = pj(self.baseDir, 'bin', 'osg-config')
         if self.verbose:
            print "   %s specified or cached. [%s]."% (self.baseDirKey, self.baseDir)
      if optDict.has_key(self.depDirKey):          
         self.depDir = optDict[self.depDirKey]
         if self.verbose:
            print "   %s specified or cached. [%s]."% (self.depDirKey, self.depDir)
      else:
         print "   Did not find key:", self.depDirKey
        
   def find(self, env):
      # Quick exit if nothing to find
      if self.baseDir != None:
         return
      
      # Find osg-config and call it to get the other arguments
      sys.stdout.write("searching for osg-config...\n")
      self.osgconfig_cmd = WhereIs('osg-config')
      if not self.osgconfig_cmd:
         self.checkRequired("   could not find osg-config.")
         self.osgconfig_cmd = None
         return
      else:
         try:
            sys.stdout.write("   found osg-config.\n");
            found_ver_str = os.popen(self.osgconfig_cmd + " --version").read().strip();
            sys.stdout.write("   version:%s"%found_ver_str);
            
            # find base dir
            self.baseDir = os.popen(self.osgconfig_cmd + " --prefix").read().strip();
            if not os.path.isdir(self.baseDir):
               self.checkRequired("   returned directory does not exist:%s"% self.baseDir);
               self.baseDir = None;
            else:
               print "   found at: ", self.baseDir;
         except Exception, ex:
            print "using osg-config failed."
            self.osgconfig_cmd = None
   
   def validate(self, env):
      # Check that path exist
      # Check that osg-config exist
      # Check that an include file: include/OpenSG/OSGConfig.h  exists
      # Update the temps for later usage
      passed = True
      if (None == self.baseDir) or (not os.path.isdir(self.baseDir)):
         self.checkRequired("OpenSG base dir does not exist:%s"%self.baseDir)
         return
      
      # Check if osg-config is found and if it can be called
      has_config_cmd = os.path.isfile(self.osgconfig_cmd)
      try:
         found_ver_str = os.popen(self.osgconfig_cmd + " --version").read().strip()
         if "" == found_ver_str:
            has_config_cmd = False               # Set to false because the command is failing.
      except Exception, ex:         
         has_config_cmd = False

      if not has_config_cmd:
         print "    Can not find or use osg-config.  Limping along without it."
         self.osgconfig_cmd = None         
      
      if has_config_cmd:      
         req_ver = self.requiredVersion.split(".")
         found_ver = found_ver_str.split(".");
         if found_ver < req_ver:
            passed = False;
            self.checkRequired("   OpenSG version is too old! Required %s but found %s"%(self.requiredVersion,found_ver_str))
         
      osgconfig_file = pj(self.baseDir, 'include', 'OpenSG', 'OSGConfig.h');
      if not os.path.isfile(osgconfig_file):
         self.checkRequired("OSGConfig.h not found:%s"%osgconfig_file);
         return
         
      # If not pass, then clear everything
      # Else we pass, set up the real data structures to use (initialized in constructor)      
      if not passed:
         # Clear everything
         self.baseDir = None;
         self.osgconfig_cmd = None;
         edict = env.Dictionary();
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey];
      else:
         self.available = True         
         print "[OK]"

   def apply(self, env, libs = ['system'], optimize = None, useCppPath = False,
             buildType = None):
      """ Add environment options for building against vapor.
          lib: One of: base, system, glut, x, qt.
          optimize: If true use --opt option, if None, then try to autoset based on
                    env["variant"].  It will default to False
          useCppPath: If true, then put the include paths into the CPPPATH variable.
      """
      if buildType is not None:
         optimize = buildType == 'optimized'

      # Auto-set optimize
      if not optimize:         
         if env.has_key("variant") and env["variant"].has_key("type"):
            var_type = env["variant"]["type"]
            optimize = ("debug" != "var_type")            
         else:            
            optimize = False   # Default to debug            
      #if not (type(libs) in (types.TupleType, types.ListType)):
      #   libs = (libs,)      
      if not isinstance(libs, list):
         libs = [libs,]
      
      # Ensure we are using standardized naming
      naming_map = { ("base","Base"):"Base",
                     ("system","System"):"System",
                     ("GLUT","glut","Glut"):"GLUT",
                     ("X","x"):"X",
                     ("QT","qt"):"QT",
                     ("contrib","Contrib"):"Contrib" }      
      for i in range(len(libs)):
         for (name_list,norm_name) in naming_map.iteritems():
            if libs[i] in name_list:
               libs[i] = norm_name
      lib_names_str = " ".join(libs)      # List of library names as a string for osg-config
      #print "lib_names_str: ", lib_names_str
      
      if self.osgconfig_cmd:
         # Returns lists of the options we want
         opt_option = " --dbg"
         if optimize:
            opt_option = " --opt"
   
         # Call script for output
         cflags_stripped = os.popen(self.osgconfig_cmd + opt_option + " --cflags " + lib_names_str).read().strip()      
         libs_stripped = os.popen(self.osgconfig_cmd + " --libs " + lib_names_str).read().strip()
   
         # Get output from osg-config
         # Res that when matched against osg-config output should match the options we want
         # In future could try to use INCPREFIX and other platform neutral stuff
         inc_re = re.compile(r'(?: |^)-I(\S*)', re.MULTILINE);
         lib_re = re.compile(r'(?: |^)-l(\S*)', re.MULTILINE);
         lib_path_re = re.compile(r'(?: |^)-L(\S*)', re.MULTILINE);
         link_from_lib_re = re.compile(r'((?: |^)-[^lL]\S*)', re.MULTILINE);
         defines_re = re.compile(r'(?: |^)-D(\S*)', re.MULTILINE)
         optimization_opts_re = re.compile(r'^-(g|O\d)$')
            
         # Extract the flags and options from the script output
         found_cflags = cflags_stripped.split(" ")
         found_cflags = [s for s in found_cflags if not optimization_opts_re.match(s)]
         found_cflags = [s for s in found_cflags if not inc_re.match(s)]
         found_cflags = [s for s in found_cflags if not defines_re.match(s)]
   
         found_libs = lib_re.findall(libs_stripped)
         found_lib_paths = lib_path_re.findall(libs_stripped)
         found_defines = defines_re.findall(cflags_stripped)
         found_incs = inc_re.findall(cflags_stripped)
         found_incs_as_flags = [env["INCPREFIX"] + p for p in found_incs]
         
         #print "cflags_stripped: [%s]"%cflags_stripped
         #print "found cflags:", found_cflags
         #print "Found: ", found_defines
         #print "found_incs_as_flags: ", found_incs_as_flags
         #print "Found incs:", found_incs
          
         if len(found_cflags):
            env.Append(CXXFLAGS = found_cflags);
         if len(found_libs):
            env.Append(LIBS = found_libs);
         else:
            print "ERROR: Could not find OpenSG libs for libs:%s lib_names_str:%s" % (libs, lib_names_str)
         if len(found_lib_paths):
            env.Append(LIBPATH = found_lib_paths);
         else:
            print "ERROR: Could not find OpenSG lib paths for libs: %s lib_names_str:%s" % (libs, lib_names_str)
            
         if len(found_incs):
            if self.useCppPath or useCppPath:
               env.Append(CPPPATH = found_incs)
            else:
               env.Append(CXXFLAGS = found_incs_as_flags)
         if len(found_defines):
            env.Append(CPPDEFINES = found_defines)

      # If on Windows, just make some very lame assumptions and hope they are correct
      elif sca_util.GetPlatform() == "win32":
         lib_map = {"Base":["OSGBase",],
                    "GLUT":["OSGWindowGLUT","OSGSystem","OSGBase"],
                    "FileIO":["OSGFileIO",],
                    "Drawable":["OSGDrawable",],
                    "Group":["OSGGroup",],
                    "ImageFileIO":["OSGImageFileIO",],
                    "RenderTraversal":["OSGRenderTraversal",],
                    "State":["OSGState",],
                    "System":["OSGSystem","OSGBase"],
                    "Text":["OSGText",],
                    "Util":["OSGUtil",],
                    "WIN32":["OSGWindowWIN32",],
                    "Window":["OSGWindow",],
                    "Contrib":["OSGContrib","OSGSystem","OSGBase"]}
         
         found_libs = []
         lib_suffix = ""
         if not optimize:
            lib_suffix = "D"
         for l in libs:
            #print "Checking: ", l
            #print "has_key: ", lib_map.has_key(l)
            if lib_map.has_key(l):
               found_libs.extend([lib+lib_suffix for lib in lib_map[l]])
               
         lib_path = pj(self.baseDir,'lib')
         inc_path = pj(self.baseDir,'include')
         if not os.path.exists(lib_path):
            print "ERROR: Could not find OpenSG lib path.  tried: ", lib_path
         if not os.path.exists(inc_path):
            print "ERROR: Could not find OpenSG include path.  tried: ", inc_path
         
         env.AppendUnique(LIBS = found_libs,
                    LIBPATH = [lib_path,],
                    CPPPATH = [inc_path,])
         
         # XXX: Hack to apply extra info that I would like to get from osg-config
         # if it only worked without cygwin
         common_cppdefines = ["WIN32","_WINDOWS","WINVER=0x0400",("_WIN32_WINDOWS","0x0410"),
                       ("_WIN32_WINNT","0x0400"), "_OSG_HAVE_CONFIGURED_H_",
		                  "OSG_BUILD_DLL",  "OSG_WITH_TIF", "OSG_WITH_JPG",
		                  "OSG_WITH_PNG", "OSG_WITH_GIF"]
         debug_cppdefines = ["OSG_DEBUG"]   
         glut_cppdefines = ["OSG_WITH_GLUT",]
         common_libs = ["tif32","libjpeg","libpng","opengl32","glu32",
                        "gdi32","user32","kernel32","winmm","wsock32"]
               
         env.Append(CPPDEFINES=common_cppdefines,
                    LIBS=common_libs)
         if not optimize:
            env.Append(CPPDEFINES=debug_cppdefines)         
         if "GLUT" in libs:
            env.Append(CPPDEFINES=glut_cppdefines)
              
         #print "---------------------\nApplying OpenSG:\n-----------------"
         #print "lib_map: ", lib_map
         #print "libs: ", libs
         #print "found libs: ", found_libs
         #print "----------------------------------"
      
      # Apply dep dir settings if we have them
      #print "apply: depDir: ", self.depDir
      if self.depDir:
         #print "Apply: Have depdir:", self.depDir
         env.Append(CPPPATH=[pj(self.depDir,'include'),],
                    LIBPATH=[pj(self.depDir,'lib'),])
         #print "   LIBPATH:", env["LIBPATH"]
         

   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),(self.depDirKey,self.depDir)]

   def dumpSettings(self):
      "Write out the settings"
      print "OpenSGBaseDir:", self.baseDir;
      print "osg-config:", self.osgconfig_cmd;      
      #print "CXXFLAGS:", self.found_cflags;
      #if self.found_libs:
      #   for lib_name in self.found_libs.keys():
      #      print "LIBS (%s):"%lib_name, self.found_libs[lib_name]
      #      print "LIBPATH (%s):"%lib_name, self.found_lib_paths[lib_name]
      #print "DEFINES:", self.found_defines

