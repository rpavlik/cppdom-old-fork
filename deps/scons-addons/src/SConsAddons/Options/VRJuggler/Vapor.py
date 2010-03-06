"""SConsAddons.Options.VRJuggler.Vapor

Defines options for Vapor project
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


class Vapor(FlagPollBasedOption.FlagPollBasedOption):
   """ 
   Options object for capturing vapor options and dependencies.
   """

   def __init__(self, name, requiredVersion, required=True, useCppPath=True):
      """
         name - The name to use for this option
         requiredVersion - The version of vapor required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, put the include paths in cpppath else, put them in cxxflags.
      """
      FlagPollBasedOption.FlagPollBasedOption.__init__(self, name, 'vpr', requiredVersion, required, useCppPath)


class Vapor_config(JugglerCommon.JugglerCommon):
   """ 
   Options object for capturing vapor options and dependencies.
   """

   def __init__(self, name, requiredVersion, required=True, useCppPath=True):
      """
         name - The name to use for this option
         requiredVersion - The version of vapor required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, put the include paths in cpppath else, put them in cxxflags.
      """
      help_text = """Base directory for vapor. bin, include, and lib should be under this directory""";
      self.baseDirKey = "VprBaseDir"
      self.optionName = "Vapor"
      self.configCmdName = 'vpr-config'
      self.filesToCheckRelBase = [pj('include','vpr','vprConfig.h'),]
      
      JugglerCommon.JugglerCommon.__init__(self, name, requiredVersion, required, useCppPath, help_text);
