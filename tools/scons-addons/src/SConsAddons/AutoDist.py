"""
AutoDist

Automatic distribution builder and packager for SCons.

"""
############################################################## autodist-cpr beg
#
# AutoDist - Automatic distribution builder and packager for
#            SCons-based builds
# AutoDist is (C) Copyright 2002 by Ben Scott
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# -----------------------------------------------------------------
# File:          $RCSfile$
# Date modified: $Date$
# Version:       $Revision$
# -----------------------------------------------------------------
############################################################## autodist-cpr end

__version__    = '0.2.1'
__author__     = 'Ben Scott and Allen Bierbaum'


import os
from os import path
import stat
import re

import SCons.Defaults
import SCons.Environment
import SCons.Node.FS
import SCons.Util
import types
import re
import time
import glob
import shutil

pj = os.path.join


# SCons shorthand mappings
Action          = SCons.Action.Action
Builder         = SCons.Builder.Builder
Environment     = SCons.Environment.Environment
File            = SCons.Node.FS.default_fs.File
Value           = SCons.Node.Python.Value

config_script_contents = ""

class InstallableFile:
   """ Class to wrap any installable file.  ex. progs, libs, headers, docs, etc """
   def __init__(self, fileNode, prefix=""):
      """ fileNode - The scons node for the file.
          prefix - The prefix to use for the file (munus the package/bundle prefix)
      """
      assert isinstance(fileNode, SCons.Node.Node), "Installable file called with non file node: [%s]"%str(fileNode)
      self.fileNode = fileNode
      self.prefix = prefix

   def __str__(self):
      """ an installable file's string representation is its prefix/name."""
      if prefix:
         return path.join(prefix, str(fileNode))
      else:
         return str(fileNode)

   def getFileNode(self):
      return self.fileNode

   def getPrefix(self):
      return self.prefix

   
class Header(InstallableFile):
   """ This class is meant to wrap a header file to install. """
   def __init__(self, fileNode, prefix=""):
      InstallableFile.__init__(self, fileNode, prefix)
      

class FileBundle:
   """ Wrapper class for a group of files to bundle together to install, archive, etc. 

       This class provides a method to hold a group of files together with their destination 
       structure information.  Then it can be used to call install builders to setup an 
       install where needed.
       
       One area of note is prefix handling.  To allow maximum flexibility there are
       several prefixes used.
       
       When built, a prefix is passed as a base prefix (build_prefix)
       The file bundle has a prefix (bundle_prefix).
       Each bundled file has a prefix (file.prefix added when files is added).
       If path to the added file has a prefix it is added on as well.
       
       Final file location: build.prefix/bundle.prefix/passed_prefix/file.prefix/file.name
   """

   def __init__(self, bundlePrefix=""):
      """
      Construct file bundle object.
      """
      self.files = []                      # Installable files of class InstallableFile
      self.bundlePrefix = bundlePrefix
      self.built = False

   def addFiles(self, files, prefix = "", useRelPath=True):
      """
      Add these files to the list of installable files.
      The list can either be string file names or File() nodes.
      files - List of files to add to bundle.
      prefix - A common prefix to use for all installed files (beyond the bundle prefix)
      useRelPath - If true, append the relative path in the file to
                   the prefix to get the real full install path.
      """
      if not SCons.Util.is_List(files):
         files = [files]
         
      for f in files:                              # For all files
         local_dir_prefix = ""
         # If we are a string filename create file object
         if not isinstance(f, SCons.Node.FS.Base):
            f = File(f)
         if useRelPath:
            local_dir_prefix = str(f.dir)
         install_file = InstallableFile(f, pj(prefix, local_dir_prefix))
         self.files.append(install_file)                       # Append it on

   def getFiles(self):
      return self.files

   def buildInstall(self, env=None, installPrefix="", ignoreBuilt=False):
      """
      Calls install builder to setup the installation of the packaged files.
      Installs all files using the env environment under prefix.
      NOTE: file is installed into: installPrefix/bundle.prefix/passed_prefix/file.prefix/file.name
      
      NOTE: Whatever the current directly is when this is called is the directory
      used for the builder command associated with this assembly.
      
      Returns list of the Install() targets.
      ifgnoreBuilt - If true, just rebuild for the given environment and don't test/set the built flag.
      """
      if not ignoreBuilt:
         assert not self.built
         self.built = True
      
      # Clone the base environment if we have one
      if env:
         env = env.Copy()
      else:
         env = Environment()
      
      ret_targets = []
      
      # Install the files from the bundle
      common_prefix = pj(installPrefix, self.bundlePrefix)
      name = common_prefix
      if hasattr(self,"name"):
         name = self.name         
      #print "FileBundle-[%s]: buildInstall: "%name
      
      for f in self.files:
         fnode = f.getFileNode()
         target_dir = path.join(installPrefix, self.bundlePrefix, f.getPrefix())
         #print "   file:[%s] --> target dir: [%s]"%(str(fnode),target_dir)
         inst_tgt = env.Install(target_dir, fnode)
         ret_targets.append(inst_tgt)
         
      return ret_targets


# ############################################################################ #
#           ASSEMBLIES  
# ############################################################################ #      
class _Assembly:
   """ Base class for all assembly types that can be managed by a package. 
       This is an abstract type that provides common functionality and interface for all assemblies.
   """
   def __init__(self, pkg, baseEnv, prefix=""):
      """
      Constructs a new Assembly object with the given name within the given
      environment.
      """
      self.package = pkg                   # The package object we belong to
      self.built = False;                  # Flag set once we have been built
      self.installPrefix = prefix;         # Default install prefix

      # Clone the base environment if we have one
      if baseEnv:
         self.env = baseEnv.Copy()
      else:
         self.env = Environment()

   def setInstallPrefix(self, prefix):
      self.installPrefix = prefix

   def getInstallPrefix(self):
      return self.installPrefix
   
   def isBuilt(self):
      return self.built
   
   def build(self):
      """
      Sets up the build targets for this assembly.
      May only be called once.
      NOTE: Whatever the current directly is when this is called is the directory
      used for the builder command associated with this assembly.
      """      

      # Now actually do the build
      self._buildImpl()
      self.built = True;



class FileBundleAssembly(_Assembly):
   """ Wrapper class for a group of files to manage in an assembly for packaging. 
       
       One area of note is prefix handling.  To allow maximum flexibility there 
       are several used prefixes.
       
       The file bundle has a prefix (this is relative to the package prefix).
       Each bundled file has a prefix (added when files is added).
       If path to the added file has a prefix it is added on as well.
       
       Final file location: package.prefix/filebundle.prefix/added.prefix/file.prefix/file_name
   """

   def __init__(self, pkg, baseEnv, prefix=""):
      """
      Construct file bundle object.
      prefix - Prefix to install the files (relative to package)
      pkg - The package this bundle is a part of
      baseEnv - The environment that this bundle should be included in
      """
      print "WARNING: Usage of FileBundleAssembly is deprecated."
      _Assembly.__init__(self, pkg, baseEnv, prefix)

      self.files = []                      # Installable files of class InstallableFile
      self.built = False                   # Flag set once we have been built      

   def addFiles(self, files, prefix = None):
      """
      Add these files to the list of installable files.  They will be installed as:
      package.prefix/self.prefix/prefix/file_prefix. The list must come
      in as strings as they are processed through File().
      files - List of filenames (file nodes should also work)
      """
      if not SCons.Util.is_List(files):
         files = [files]
      
      for fn in files:                                # For all filenames
         local_dir_prefix = ""
         if not isinstance(fn, SCons.Node.FS.Base):        # If we are a string filename, then get our local prefix
            local_dir_prefix = os.path.dirname(fn)
         f = InstallableFile( File(fn), pj(prefix, local_dir_prefix))   # Create new installable file
         self.files.append(f)                       # Append it on
         
   def getInstallableFiles(self):
      return self.files

   def isBuilt(self):
      return self.built;
   
   def build(self):
      """
      Sets up the build and install targets for this file bundle.
      May only be called once.
      NOTE: Whatever the current directly is when this is called is the directory
      used for the builder command associated with this assembly.
      """
      # Add files to the package file bundle
      fb = self.package.createFileBundle()

      for f in self.files:
         fnode = f.getFileNode()
         fb.addFiles(fnode, f.getPrefix())        

      self.built = True;         



class _CodeAssembly(_Assembly):
   """
   Base type for assemblys that are based on "code".

   This "abstract" class provides common functionality for Program,
   StaticLibrary, and SharedLibrary. You don't want to instantiate this class
   directly. It is meant to be private.
   """

   def __init__(self, filename, pkg, baseEnv, prefix=""):
      """
      Constructs a new Assembly object with the given name within the given
      environment.
      """
      _Assembly.__init__(self, pkg, baseEnv, prefix)
      
      assert isinstance(filename, str), "Passed a filename that is not a string. %s"%filename
      self.fileNode      = File(filename)
      self.sources       = []
      self.includes      = []
      self.libs          = []
      self.libpaths      = []
      self.headers       = []

   def addSources(self, sources):
      """
      Adds the given list of source files into this assembly. The list must come
      in as strings as they are processed through File().
      """
      # Use File() to figure out the absolute path to the file
      srcs = map(File, sources)
      # Add these sources into the mix
      self.sources.extend(srcs)

   def addHeaders(self, headers, prefix = None):
      """
      Adds the given list of distribution header files into this assembly. These
      headers will be installed to self.package.prefix/include/prefix/file_prefix. The list must come
      in as strings as they are processed through File().
      """
      for fn in headers:                              # For all filenames in headers
         hdr = Header( File(fn), prefix)              # Create new header rep
         self.headers.append(hdr)                     # Append it on

   def addIncludes(self, includes):
      """
      Adds in the given list of include directories that this assembly will use
      while compiling.
      """
      self.includes.extend(includes)

   def addLibs(self, libs):
      """
      Adds in the given list of libraries directories that this assembly will
      link with.
      """
      self.libs.extend(libs)

   def addLibPaths(self, libpaths):
      """
      Adds in the given list of library directories that this assembly will use
      to find libraries while linking.
      """
      self.libpaths.extend(libpaths)

   def getHeaders(self):
      return self.headers

   def getSources(self):
      return self.sources

   def isBuilt(self):
      return self.built;
   
   def getFilename(self):
      return str(self.fileNode)
   
   def getAbsFilePath(self):
      return self.fileNode.get_abspath()

   def build(self):
      """
      Sets up the build and install targets for this assembly.
      May only be called once.
      NOTE: Whatever the current directly is when this is called is the directory
      used for the builder command associated with this assembly.
      """
      # Setup the environment for the build
      self.env.Append(CPPPATH = self.includes,
                      LIBPATH = self.libpaths,
                      LIBS    = self.libs)

      # Now actually do the build
      self._buildImpl()
      self.built = True;


class Library(_CodeAssembly):
   """
   This "abstract" class provides common functionality for StaticLibrary and
   SharedLibrary. You don't want to instantiate this class directly. It is
   meant to be private.
   """

   def __init__(self, libname, pkg, baseEnv, builderNames, installPrefix):
      """
      Creates a new library builder for a library of the given name.
      pkg - The package we are a part of
      baseEnv - The base environemnt to use
      builderNames - The names of the builders to use for building the libary: ex. 'SharedLibrary'
      installPrefix - Prefix (relative to the standard install path) to install this library
      """
      _CodeAssembly.__init__(self, libname, pkg, baseEnv, installPrefix)
      
      if type(builderNames) is types.StringType:
         self.builder_names = [ builderNames ]
      else:
         self.builder_names = builderNames      

   def _buildImpl(self):
      """
      Sets up the build dependencies and the install.
      """

      fb = self.package.createFileBundle()

      # Setup build and install for each built library
      # Use get_abspath() with fileNode so we get the path into the build_dir and not src dir
      # Only build libraries if we have sources
      if len(self.sources) > 0:
         for lib_builder in self.builder_names:
            lib_filepath = self.fileNode.get_abspath()
            lib = self.env.__dict__[lib_builder](lib_filepath, self.sources)

            # Lib to file bundle
            fb.addFiles(lib, self.installPrefix, False)

      # Install the headers in the source list
      for h in self.headers:
         headerNode = h.getFileNode()
         fb.addFiles(headerNode, pj('include', h.getPrefix()) )


class SharedLibrary(Library):
   """ Sets up Library assembly with 'SharedLibrary' builder."""
   def __init__(self, libname, pkg, baseEnv = None, installPrefix='lib'):
      Library.__init__(self, libname, pkg, baseEnv, 'SharedLibrary', installPrefix)

class StaticLibrary(Library):
   """ Sets up Library assembly with 'StaticLibrary' builder """
   def __init__(self, libname, pkg, baseEnv = None, installPrefix='lib'):
      Library.__init__(self, libname, pkg, baseEnv, 'StaticLibrary', installPrefix)

class StaticAndSharedLibrary(Library):
   """ Sets up Library assembly with both 'StaticLibrary' and 'SharedLibrary' builders. """
   def __init__(self, libname, pkg, baseEnv = None, installPrefix='lib'):
      Library.__init__(self, libname, pkg, baseEnv, ['StaticLibrary', 'SharedLibrary'], installPrefix)


class Program(_CodeAssembly):
   """
   This object knows how to build (and install) an executable program from a
   given set of sources.
   """
   def __init__(self, progname, pkg, baseEnv = None, installPrefix='bin'):
      """
      Creates a new program builder for a program of the given name.
      """
      _CodeAssembly.__init__(self, progname, pkg, baseEnv, installPrefix)      

   def _buildImpl(self):
      """
      Sets up the build dependencies and the install.
      """
      # Build rule
      prog = self.env.Program(self.fileNode, source = self.sources)

      # Add executable to file bundle
      fb = self.package.createFileBundle()
      fb.addFiles(prog, self.installPrefix, False)
      
      # Install the binary
      #inst_prefix = self.package.prefix
      #if self.installPrefix:
      #   inst_prefix = pj(inst_prefix, self.installPrefix)
      #self.env.Install(inst_prefix, prog)

      
class Package:
   """
   A package defines a collection of distributables including programs and
   libraries. The Package class provides the ability to build, install, and
   package up distributions of your project.
   
   A package object provides an interface to add libraries, programs, headers,
   and support files to a single unit that can be installed.  It also shares
   an environment across all these different units to build.
   
   The package contains assemblies (objects that encapsulate things to build)
   and a FileBundle (which holds all the files that could be installed or handled).
   
   When assemblies are built, they should add any installable files to the package file bundle.
   """

   def __init__(self, name, version, prefix='/usr/local', baseEnv = None, description= None):
      """
      Creates a new package with the given name and version, where version is in
      the form of <major>.<minor>.<patch> (e.g 1.12.0)
      """
      self.name = name
      self.prefix = prefix
      self.assemblies = []
      self.fileBundles = [FileBundle(),]     # File bundle for all files
      self.extra_dist = []
      self.description = description
      self.packagers = []
      self.distDir = "dist"                  # Path to the dist directory to use
      
      if not self.description:
         self.description = self.name + " Package"
      
      if baseEnv:
         self.env = baseEnv.Copy()
      else:
         self.env = Environment()
         
      if type(version) is types.TupleType:
         (self.version_major, self.version_minor, self.version_patch) = version;
      else:
         re_matches = re.match(r'^(\d+)\.(\d+)\.(\d+)$', version)
         self.version_major = int(re_matches.group(1))
         self.version_minor = int(re_matches.group(2))
         self.version_patch = int(re_matches.group(3))
         
   def getName(self):
      return self.name

   def getVersionMajor(self):
      return self.version_major

   def getVersionMinor(self):
      return self.version_minor

   def getVersionPatch(self):
      return self.version_patch
   
   def getFullVersion(self):
      return ".".join( (str(self.version_major), str(self.version_minor), str(self.version_patch)) )

   def getAssemblies(self):
      return self.assemblies

   def getExtraDist(self):
      return self.extra_dist
   
   def getFileBundles(self):
      return self.fileBundles  
   
   def getEnv(self):
      " Get the common pakcage environment. "
      return self.env
   
   def setDistDir(self, path):
      " Set the prefix for distribution/packaged files. "
      self.distDir = path
      
   def getDistDir(self):
      return self.distDir
      
   def addPackager(self, packager):
      " Add a new packager.  Sets the packager to point to this package. "
      packager.setPackage(self)
      self.packagers.append(packager)

   # ###### Assembly factory methods ####### #
   def createSharedLibrary(self, name, baseEnv = None, installPrefix='lib'):
      """
      Creates a new shared library of the given name as a part of this package.
      The library will be built within the given environment.
      """
      if not baseEnv:
         baseEnv = self.env
      lib = SharedLibrary(name, self, baseEnv, installPrefix)
      self.assemblies.append(lib)
      return lib

   def createStaticLibrary(self, name, baseEnv = None, installPrefix='lib'):
      """
      Creates a new static library of the given name as a part of this package.
      The library will be built within the given environment.
      """
      if not baseEnv:
         baseEnv = self.env
      lib = StaticLibrary(name, self, baseEnv, installPrefix)
      self.assemblies.append(lib)
      return lib
   
   def createStaticAndSharedLibrary(self, name, baseEnv = None, installPrefix='lib'):
      """
      Creates new static and shared library of the given name as a part of this package.
      The library will be built within the given environment.
      """
      if not baseEnv:
         baseEnv = self.env
      lib = StaticAndSharedLibrary(name, self, baseEnv, installPrefix)
      self.assemblies.append(lib)
      return lib

   def createProgram(self, name, baseEnv = None, installPrefix='bin'):
      """
      Creates a new executable program of the given name as a part of this
      package. The program will be built within the given environment.
      """
      if not baseEnv:
         baseEnv = self.env
      prog = Program(name, self, baseEnv, installPrefix)
      self.assemblies.append(prog)
      return prog
   
   def createFileBundleAssembly(self, prefix, baseEnv = None): 
      """ Creates a new FileBundle object as part of this package. """
      bundle = FileBundleAssembly(pkg = self, baseEnv = baseEnv, prefix=prefix)
      self.assemblies.append(bundle)
      return bundle
   
   def createFileBundle(self, bundlePrefix=""):
      """ Creates a new file bundle to use with this package. """
      fb = FileBundle(bundlePrefix)
      self.fileBundles.append(fb)
      return fb   
   
   def createConfigAction(self, target, source, env):
      """ Called as action of config script builder """
      global config_script_contents
      
      new_contents = config_script_contents
      value_dict = source[0].value                  # Get dictionary from the value node
      
      all_lib_names = [os.path.basename(l.getAbsFilePath()) for l in self.assemblies if isinstance(l,Library)]
      lib_names = []
      for l in all_lib_names:
         if not lib_names.count(l):
            lib_names.append(l)
      inc_paths = [pj(self.prefix,'include'),]
      cflags = " ".join(["-I"+p for p in inc_paths])
      if value_dict["extraCflags"] != None:
         cflags = cflags + " " + value_dict["extraCflags"]
      lib_paths = [pj(self.prefix,'lib'),]
      if value_dict["extraIncPaths"] != None:
         cflags = cflags + " ".join([" -I"+l for l in value_dict["extraIncPaths"]])
      
      # Extend varDict with local settings
      varDict = {}
      if value_dict["varDict"] != None:
         varDict = value_dict["varDict"]
         
      varDict["Libs"] = " ".join(["-L"+l for l in lib_paths])
      if value_dict["extraLibPath"] != None:
         varDict["Libs"] += " " + " ".join(["-L"+l for l in value_dict["extraLibPath"]])
      if len(lib_names):
         varDict["Libs"] += " " + " ".join(["-l"+l for l in lib_names])
      if value_dict["extraLibs"]!=None:
         varDict["Libs"] = varDict["Libs"] + " " + " ".join(["-l"+l for l in value_dict["extraLibs"]])
      varDict["Cflags"] = cflags
      varDict["Version"] = self.getFullVersion()
      varDict["Name"] = self.name
      varDict["Description"] = self.description
      varDict["Prefix"] = self.prefix
      
      # Create the new content
      txt = "# config script generated for %s at %s\n" % (self.name, time.asctime())
      txt = txt + '# values: %s\n'%(source[0].get_contents(),)
      
      for k,v in value_dict["varDict"].items():
         if SCons.Util.is_String(v):
            txt = txt + 'vars["%s"] = "%s"\n' % (k,v)
         else:
            txt = txt + 'vars["%s"] = %s\n' % (k,v)            
            
      # Find and replace the replacable content
      cut_re = re.compile(r'(?<=# -- BEGIN CUT --\n).*?(?=# -- END CUT --)', re.DOTALL)
      new_contents = cut_re.sub(txt,config_script_contents)
      
      # Create and write out the new file (setting it executable)
      fname = str(target[0])
      f = open(str(target[0]), 'w')
      f.write(new_contents)
      f.close()
      os.chmod(fname, stat.S_IREAD|stat.S_IEXEC|stat.S_IWUSR)    # Set the file options
      
      return 0   # Successful build
      
   def createConfigScript(self, name, installDir='bin', varDict = None,
                          extraIncPaths=None, extraLibs=None, extraLibPaths=None, extraCflags=None):
      """ Adds a config script to the given package installation.
          varDict - Dictionary of extra variables to define.
      """

      if not self.env['BUILDERS'].has_key("PackageConfigScript"):
         cfg_builder = Builder(action = Action(self.createConfigAction,
                                        lambda t,s,e: "Create config script for %s package: %s"%(self.name, t[0])) )
         self.env.Append(BUILDERS = {"PackageConfigScript" : cfg_builder})
      
      value_dict = {}
      value_dict["prefix"] = self.prefix
      value_dict["extraIncPaths"] = extraIncPaths
      value_dict["extraLibs"] = extraLibs
      value_dict["extraLibPath"] = extraLibPaths
      value_dict["extraCflags"] = extraCflags
                 
      value_dict["varDict"] = varDict
      self.env.PackageConfigScript(target = pj(installDir, name), source = Value(value_dict))
      
      # May need to delay doing this until a later build stage so that all libs, headers, etc are 
      # setup and ready to go in this package.
      

   def addExtraDist(self, files, exclude=[]):
      """
      Adds in the given files to the distribution of this package. If a
      directory is encountered in the file list, it is recursively added. Files
      whose names are in the exclude list are not added to the list.
      """
      for file in files:
         assert isinstance(file, str), "file was not of correct type."
         # Make sure to skip files in the exclude list
         if not file in exclude:
            # If the file is a directory, recurse on it
            if os.path.isdir(str(file)):
               self.addExtraDist(os.listdir(str(file)), exclude)
            # If the file is not a directory, add in the extra dist list
            else:
               self.extra_dist.append(file)


   def build(self):
      """
      Sets up the build and install for this package. This will build all
      assemblies contained therein that have not already been built and set 
      up the standard filebundle install.
      """
      for assembly in self.assemblies:
         if not assembly.isBuilt():
            assembly.build()

      # Build the file bundles for this package
      for fb in self.fileBundles:
         fb.buildInstall(self.env, self.prefix)
      #self.env.Alias('install', self.prefix)            
      
      # Setup all packagers
      for p in self.packagers:
         p.build()


# ############################################# #
#        PACKAGERS
# ############################################# #

class Packager:
   """ Base class for all packagers in the system.
       A packager is responsible for packaging a package into a distributable form (rpm, etc).
       The packager is attached to the package and then it is the package's responsibility
       to add the Packager to the build.
   """
   def __init__(self):
      self.package = None
   
   def setPackage(self, pkg):
      " Set the package that we are packaging for. "
      self.package = pkg     # The package we are responsible for
      
   def build(self):
      " Template method that is called by the package to build the packaging mechanism. "
      pass

   
class TarGzPackager(Packager):
   """ Packager class that uses a .tar.gz file to compress the package for distribution. """
   def __init__(self):
      """ Initialize the packager.
      
      """
      Packager.__init__(self)      

   def makeDistTarGz(self, target, distDir, env = None):
      """ Builder for targz file.  
          target is file name.
          source is a list of directories for files to add.
      """
      import os
   
      # Make the tar.gz
      # Execute in the directory above the file or directory we are adding
      # and tar up the contents of the source directory
      exec_dir = os.path.dirname(os.path.abspath(distDir))
      src_dir = os.path.basename(distDir)

      targz = Action('tar -C %s -czf $TARGET %s' % (exec_dir, src_dir ))
      targz(target, None, env)
 
      return None

   def makeDistTarGz_print(self, target, source, env):
      print "Building .tar.gz dist: %s"% (target[0],)
   
   def build(self):
      env = self.package.getEnv().Copy()
      dist_dir = self.package.getDistDir()
      dist_name = "%s-%s"% (self.package.getName(), self.package.getFullVersion())
      work_dir = pj(dist_dir, dist_name)

      # Explicitly install the dist into a directory to use
      # Then create command to build dist from that directory with the install files and dependencies
      inst_targets = []
      for fb in self.package.getFileBundles():
         inst_targets += fb.buildInstall(env, work_dir, ignoreBuilt=True)
      env.Command(pj(dist_dir, dist_name+'.tar.gz'), inst_targets,
                  Action( lambda target, source, env:  self.makeDistTarGz(target, work_dir, env),
                          self.makeDistTarGz_print) 
                 )

class RpmPackager(Packager):
   """ Very simple stupid packager for working with rpm spec files.
   
   """
   def __init__(self, specFile):
      """ Initialize the packager.
      
      """
      Packager.__init__(self)
      self.specFile = specFile

   def makeDistRpm(self, target, source, distdir, buildroot, topdir, env = None):
      """ Builder for rpm files.
          target is rpm target file.
          source is the .spec file to use
          distdir - The distribution dir to install into
          buildroot is the root where the files are installed to package
          topdir is the destination direct for the files
      """
      #rpmbuild -bb -v --define='_topdir /var/tmp/cppdom' --define='_rpmdir /var/tmp/cppdom' --buildroot=/..../cppdom/build.linux/dist/cppdom-0.5.3 cppdom.spec
      rpmbuild_cmd = Action("rpmbuild -bb -v --define='_topdir %s' --define='_rpmdir %s' --buildroot=%s %s"%(topdir,topdir,buildroot,source[0]))
      rpmbuild_cmd(target, source, env)
      
      # - copy to the target location
      #   - Attempt to find the file and then copy it
      #   - If not, then big warning
      # Find rpm by looking in the target dir and searching for any matching rpm names in arch-based subdirs
      matching_rpm = glob.glob(pj(topdir, '*', os.path.basename(str(target[0]))))
      if len(matching_rpm) == 0:
         print "ERROR: Can't find dist rpm for target:", str(target[0])
      else:
         print "Move: %s --> %s"%(matching_rpm[0], str(target[0]))
         os.rename(matching_rpm[0], str(target[0]))
      
      return None

   def makeDistRpm_print(self, target, source, env):
      print "Building rpm dist: %s"% (target[0],)
   
   def build(self):
      env = self.package.getEnv().Copy()
      dist_dir = self.package.getDistDir()
      
      package_arch = 'i386'    # Hardcode for now
      package_version = self.package.getFullVersion()
      package_release = '1'
      package_name = self.package.getName()
      
      def specFileBuilder(target, source, env):
         """ Quick builder for updating the spec file with the info about this package. """
         spec_contents = source[0].get_contents()
         spec_contents = spec_contents.replace('_SCONS_PACKAGE_NAME_',package_name)
         spec_contents = spec_contents.replace('_SCONS_PACKAGE_VERSION_',package_version)
         spec_contents = spec_contents.replace('_SCONS_PACKAGE_RELEASE_',package_release)
         spec_filename_out = pj(target_rpm_dir, os.path.basename(self.specFile))
         open(str(target[0]), 'wb').write(spec_contents)
      
      # Setup the rpm file name (note this can change due to local user settings in rpmrc)
      rpm_fn_base = "%s-%s-%s.%s.rpm"%(package_name, package_version, package_release, package_arch)      
      rootname = "%s-%s-root"% (self.package.getName(), self.package.getFullVersion())       # Name of the build root to use with rpmbuild
      dist_name = "rpm"                                                     # Name of the directory under dist to use
      build_root_dir = os.path.abspath(pj(dist_dir, dist_name, rootname))   # Path of build root to create the RPM.  (abspath so rpmbuild will like it)
      target_rpm_dir = pj(dist_dir, dist_name)                              # Path to throw the rpm output
      
      # Setup the spec file
      spec_filename_out = pj(target_rpm_dir, os.path.basename(self.specFile) + ".out")
      spec_out_target = env.Command(spec_filename_out, self.specFile, specFileBuilder)

      # Explicitly install to the directory
      # Then create command to build dist from that directory with the install files and dependencies
      inst_targets = []
      for fb in self.package.getFileBundles():
         inst_targets += fb.buildInstall(env, build_root_dir, ignoreBuilt=True)
      env.Command(pj(dist_dir, rpm_fn_base), [spec_filename_out] + inst_targets,
                  Action( lambda target, source, env:  self.makeDistRpm(target, source, dist_dir, build_root_dir, target_rpm_dir, env),
                          self.makeDistRpm_print) 
                 )

def MakeSourceDist(package, baseEnv = None):
   """
   Sets up the distribution build of a source tar.gz for the given package.
   This will put all the required source and extra distribution files into a
   compressed tarball. If an environment is specified, it is copied and the
   copy is used.
   """
   # Clone the base environment if we have one
   if baseEnv:
      env = baseEnv.Copy()
   else:
      env = Environment()


   # Create the distribution filename
   dist_filename = '%s-%d.%d.%d.tar.gz' % (package.getName(),
                                           package.getVersionMajor(),
                                           package.getVersionMinor(),
                                           package.getVersionPatch())

   # Add the tar.gz builder to the environment
   _CreateSourceTarGzBuilder(env)

   # Get a list of the sources that will be included in the distribution
   dist_files = []
   for a in package.getAssemblies():
      dist_files.extend(map(lambda n: n.getFileNode(), a.getHeaders()))
      dist_files.extend(a.getSources())
   dist_files.extend(package.getExtraDist())

   # Setup the build of the distribution
   env.SourceTarGz(dist_filename, dist_files)

   # Mark implicit dependencies on all the files that will be distributed
   env.Depends(dist_filename, dist_files)

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

def _CreateSourceTarGzBuilder(env):
   import tempfile

   def makeSourceTarGz(target = None, source = None, env = None):
      import os, shutil, tempfile

      # Create the temporary directory
      dist_name = str(target[0])[:-len('.tar.gz')]
      temp_dir = tempfile.mktemp()
      target_dir = path.join(temp_dir, dist_name)

      os.makedirs(target_dir)

      # Copy the sources to the target directory
      for s in source:
         src_file = str(s)
         dest_dir = path.join(target_dir, path.dirname(src_file))
         dest_file = path.basename(src_file)

         # Make sure the target directory exists
         if not os.path.isdir(dest_dir):
            os.makedirs(dest_dir)

         # Copy the file over
         shutil.copy2(src_file, path.join(dest_dir, dest_file))

      # Make the tar.gz
      targz = Action('tar cf - -C '+temp_dir+' $SOURCES | gzip -f > $TARGET')
      targz(target, [File(dist_name)], env)

      # Remove the temporary directory
      shutil.rmtree(temp_dir)

      return None

   # Create the builder and add it to the environment
   source_targz_builder = Builder(action = makeSourceTarGz,
                                  suffix = '.tar.gz')
   env['BUILDERS']['SourceTarGz'] = source_targz_builder
   
   
   
   
   
# ------------------ CONFIG SCRIPT ------------------- #
config_script_contents = '''#!/bin/env python
#
import getopt
import sys

vars = {}

# -- BEGIN CUT --
# config script generated for PACKAGE
vars["Libs"] = "-L/usr/lib -lmylib"
vars["Cflags"] = "-I/include/here -Dmy_VAR"
vars["Version"] = "4.5.0"
vars["Name"] = "My Module"
vars["Description"] = "This is my module over here"
vars["test_var"] = "Test value"
vars["Requires"] = "OtherPackage"
vars["Prefix"] = "/home/users/browner/projects/old-collab/collab/build.linux/modules/plexus"
# -- END CUT -- #

def usage():
   print """Usage:
   --libs            output all linker flags
   --cflags          output all compiler flags
   --cflags-only-I   output only the -I flags
   --modversion      get the version of the module
   --modname         get the name of the module
   --moddesc         get the description of the module
   --variable=VAR    get the value of a variable
   --prefix          get the base installation directory

   --help            display this help message
"""

def getVar(varName):
   if vars.has_key(varName):
      return vars[varName]
   else:
      return ""

def main():
   try:
      opts, args = getopt.getopt(sys.argv[1:], "",
                                 ["help", "libs", "cflags", "cflags-only-I",
                                  "modversion", "modname", "moddesc",
                                  "variable=", "prefix"])
   except getopt.GetoptError:
      usage()
      sys.exit(2)
      
   if not len(opts):
      usage()
      sys.exit()

   ret_val = ""
   
   predefined_opts = { "libs" : "Libs",
                       "cflags" : "Cflags",
                       "modversion" : "Version",
                       "modname" : "Name",
                       "moddesc" : "Description",
                       "prefix" : "Prefix"}
   for o,a in opts:
      if o == "--help":
         usage()
         sys.exit()
      if predefined_opts.has_key(o[2:]):      # Handle standard options
         print getVar(predefined_opts[o[2:]]),
      elif o == "--cflags-only-I":
         cflags = getVar("Cflags")
         cflags_i = [x for x in cflags.split() if x.startswith("-I")]
         print " ".join(cflags_i),
      elif o == "--variable":
         print getVar(a),
         
if __name__=='__main__':
    main()
'''
