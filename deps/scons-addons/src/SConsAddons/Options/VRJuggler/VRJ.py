"""SConsAddons.Options.VRJuggler.VRJ

Defines options for VR Juggler project
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
import SConsAddons.Options.FlagPollBasedOption as FlagPollBasedOption
import JugglerCommon
import SCons.Util
import sys, os, re, string

from SCons.Util import WhereIs
pj = os.path.join


class VRJ(FlagPollBasedOption.FlagPollBasedOption):
   """ 
   Options object for capturing vapor options and dependencies.
   """

   def __init__(self, name, requiredVersion, required=True, useCppPath=True, drawManagers=['GL']):
      """
         name - The name to use for this option
         requiredVersion - The version of VRJ required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, put the include paths in cpppath else, put them in cxxflags.
      """
      self.mDrawManagers = drawManagers

      FlagPollBasedOption.FlagPollBasedOption.__init__(self, name, 'vrjuggler',
                                                       requiredVersion,
                                                       required, useCppPath)

   def validate(self, env):
      passed = FlagPollBasedOption.FlagPollBasedOption.validate(self, env)
      
      self.found_draw_mgr_libs = []
      self.found_draw_mgr_fwks = []

      if passed:
         ogl_libs = self.flagpoll_parser.findLibs("--get-vrj-ogl-libs")
         ogl_fwks = self.flagpoll_parser.findFrameworks("--get-vrj-ogl-libs")
         pf_libs = self.flagpoll_parser.findLibs("--get-vrj-pf-libs")
         for man in self.mDrawManagers:
            if man.lower() == 'gl':
               self.found_draw_mgr_libs.extend(ogl_libs)
               self.found_draw_mgr_fwks.extend(ogl_fwks)
            if man.lower() == 'pf':
               self.found_draw_mgr_libs.extend(pf_libs)

         #print "-----------------------"
         #print "self.found_draw_mgr_libs:", self.found_draw_mgr_libs

   def apply(self, env, useCppPath=False):
      """ Add environment options for building against vrj-based library"""
      passed = FlagPollBasedOption.FlagPollBasedOption.apply(self, env, useCppPath)
      if self.found_draw_mgr_libs:
         env.AppendUnique(LIBS = self.found_draw_mgr_libs)
         env.AppendUnique(FRAMEWORKS = self.found_draw_mgr_fwks)

class VRJ_config(JugglerCommon.JugglerCommon):
   """ 
   Options object for capturing vapor options and dependencies.
   """

   def __init__(self, name, requiredVersion, required=True, useCppPath=True):
      """
         name - The name to use for this option
         requiredVersion - The version of VRJ required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, put the include paths in cpppath else, put them in cxxflags.
      """
      help_text = """Base directory for VRJ. bin, include, and lib should be under this directory""";
      self.baseDirKey = "VrjBaseDir"
      self.optionName = "VR Juggler"
      self.configCmdName = 'vrjuggler-config'
      self.filesToCheckRelBase = [pj('include','vpr','vprConfig.h'),
                                  pj('include','jccl','jcclConfig.h'),
                                  pj('include','gadget','gadgetConfig.h'),
                                  pj('include','vrj','vrjConfig.h')]

      JugglerCommon.JugglerCommon.__init__(self, name, requiredVersion,required, useCppPath, help_text);
      
