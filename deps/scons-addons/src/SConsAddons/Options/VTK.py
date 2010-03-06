"""SConsAddons.Options.VTK

Defines options for VTK project
"""

#
# __COPYRIGHT__
#
# This file is part of scons-addons.
#
# Scons-addons is free software you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation either version 2 of the License, or
# (at your option) any later version.
#
# Scons-addons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with scons-addons if not, write to the Free Software
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
import distutils.util

from SCons.Util import WhereIs
pj = os.path.join


class VTK(SConsAddons.Options.PackageOption):
   """
   Options object for capturing vapor options and dependencies.
   """

   def __init__(self, name, requiredVersion, required=True,
                useCppPath = False, libList=None):
      """
         name - The name to use for this option
         requiredVersion - The version of vapor required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - Should includes be put in cpp path environment
      """
      help_text = "Base directory for Visualization ToolKit (VTK)."
      self.baseDirKey = "VtkBaseDir"
      self.vtkVersion = "vtk"
      self.requiredVersion = requiredVersion
      self.required = required
      self.useCppPath = useCppPath
      SConsAddons.Options.PackageOption.__init__(self, name,
                                                     self.baseDirKey,
                                                     help_text)

      # configurable options
      self.baseDir = None
      self.libList = libList
      # Settings to use
      self.found_libs = None
      self.found_cflags = None
      self.found_lib_paths = None
      self.found_defines = None

   def checkRequired(self, msg):
      """
      Called when there is config problem.  If required, then exit with
      error message.
      """
      print msg
      if self.required:
         sys.exit(1)

   def startProcess(self):
      print "Checking for VTK...",

   def setInitial(self, optDict):
      " Set initial values from given dict "
      if self.verbose:
         print "   Applying initial VTK settings."
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         if self.verbose:
            print "   %s specified or cached. [%s]."% \
                     (self.baseDirKey, self.baseDir)
                     
      if optDict.has_key('VtkVersion'):
         self.vtkVersion = 'vtk-%s' %(optDict['VtkVersion'])
      else:
         self.vtkVersion = 'vtk'

   def find(self, env):
      # Quick exit if nothing to find
      if self.baseDir != None:
         return

   def validate(self, env):
      # Check that path exists
      # Check that an include file (include/osg/Version) exists
      # Update the temps for later usage
      passed = False
      self.available = False

      if self.baseDir is None:
         self.checkRequired("VTK base dir (VtkBaseDir) was not specified")
         return

      if not os.path.isdir(self.baseDir):
         self.checkRequired("VTK base dir %s does not exist" % self.baseDir)
         return
         
      vtk_version_file = pj(self.baseDir, 'include', self.vtkVersion, 'vtkConfigure.h')
      if not os.path.isfile(vtk_version_file):
         self.checkRequired("%s not found" % vtk_version_file)
         return
      else:
         passed = True

      # TODO: Check version requirement

      # If not pass, then clear everything
      # Else we pass, set up the real data structures to use (initialized in
      # constructor)
      if not passed:
         # Clear everything
         self.baseDir = None
         edict = env.Dictionary()
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey]
      else:
         self.available = True
         print "[OK]"

   def apply(self, env, optimize = False, useCppPath = False):
      """
      Add environment options for building against vapor.
      optimize: If true use --opt option
      useCppPath: If true, then put the include paths into the CPPPATH
                  variable.
      """
      inc_dir = os.path.join(self.baseDir, 'include',self.vtkVersion )
      if self.useCppPath or useCppPath:
         env.Append(CPPPATH = [inc_dir])
      else:
         env.Append(CXXFLAGS = [inc_dir])

      #help improve the effeciency of the build by 
      #doing foward declarations in VTK
      env.Append(CPPDEFINES = ['VTK_STREAMS_FWD_ONLY'])
      env.Append(LIBPATH = [os.path.join(self.baseDir, 'lib')])

      env.Append(LIBS = self.libList)

   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),]

   def dumpSettings(self):
      "Write out the settings"
      print "VtkBaseDir:", self.baseDir
      #print "CXXFLAGS:", self.found_cflags
      #if self.found_libs:
      #   for lib_name in self.found_libs.keys():
      #      print "LIBS (%s):"%lib_name, self.found_libs[lib_name]
      #      print "LIBPATH (%s):"%lib_name, self.found_lib_paths[lib_name]
      #print "DEFINES:", self.found_defines

