"""
General Util methods and classes for SConsAddons.

This should NOT be a final resting point for code, but more a place
to put things while refactoring and deciding where they should end
up long term.
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
from __future__ import generators

import os
import sys
import re
import string
import SCons.Environment
import SCons

pj = os.path.join


def hasHelpFlag():
   """ Return true if the help flag was passed to scons. """
   try: 
      has_help_flag = SCons.Script.Main.options.help_msg
   except AttributeError: 
      has_help_flag = SCons.Script.options.help_msg
      
   return has_help_flag


# ----------------------------------
# File extension retrieval methods
# -----------------------------------
def fileExtensionMatches(f, fexts):
   """ Returns true if f has an extension in the list fexts """
   for ext in fexts:
      if f.endswith(ext):
         return True
   return False
      
def getPredFilesRecursive(tree_root, predicateMethod):
   """ Return list of sources recursively.  Returned paths are related to the tree_root """
   def dir_method(f_list, dirpath, namelist):
      for f in namelist:
         if predicateMethod(f):
            f_list.append(os.path.normpath(pj(dirpath, f)))
   cur_dir = os.getcwd()
   os.chdir(tree_root)
   f_list = [];
   os.path.walk('.', dir_method, f_list);
   os.chdir(cur_dir)
   return f_list


def getHeaders():
   """ get the headers in the current directory """
   exts = ['.h','.hpp']
   files = os.listdir('.')
   ret = []
   for x in files:
      for e in exts:
         if x.endswith(e):
            ret.append(x)
   return ret
 
def getSources():
   """ get the source files in the current directory"""
   exts = ['.cpp','.c']
   files = os.listdir('.')
   ret = []
   for x in files:
      print "checking if"+x+"is a header"
      for e in exts:
         print "wither header suffix: "+e
         if x.endswith(e):
            ret.append(x)
   return ret

 
         
def getFilesRecursiveByExt(tree_root, fexts):
   def hasExtension(f):
      return fileExtensionMatches(f, fexts)
   return getPredFilesRecursive(tree_root, hasExtension)

def getSourcesRecursive(tree_root):
   """ Return list of sources recursively """
   return getFilesRecursiveByExt(tree_root, ['.cpp','.C'])

def getHeadersRecursive(tree_root):
   """ Return list of headers recursively """
   return getFilesRecursiveByExt(tree_root, ['.h','.hpp'])


class ConfigCmdParser:
   """  
   Helper class for calling a given *-config command and extracting
   various paths and other information from it.
   """
   
   def __init__(self, config_cmd):
      " config_cmd: The config command to call "
      self.config_cmd = config_cmd
      self.valid = True
      if not os.path.isfile(config_cmd):
         self.valid = False

      # Initialize regular expressions
      # Res that when matched against config output should match the options we want
      # In future could try to use INCPREFIX and other platform neutral stuff
      self.inc_re = re.compile(r'-I(\S*)', re.MULTILINE);
      self.lib_re = re.compile(r'-l(\S*)', re.MULTILINE);
      self.lib_path_re = re.compile(r'-L(\S*)', re.MULTILINE);
      
   def findLibs(self, arg="--libs"):
      if not self.valid:
         return ""
      return self.lib_re.findall(os.popen(self.config_cmd + " " + arg).read().strip())
   
   def findLibPaths(self, arg="--libs"):
      if not self.valid:
         return ""
      return self.lib_path_re.findall(os.popen(self.config_cmd + " " + arg).read().strip())

   def findIncludes(self, arg="--cflags"):
      if not self.valid:
         return ""
      return self.inc_re.findall(os.popen(self.config_cmd + " " + arg).read().strip())

   def getVersion(self, arg="--version"):
      if not self.valid:
         return ""
      return os.popen(self.config_cmd + " " + arg).read().strip()
   
def GetPlatform():
   "Get a platform string"
   if string.find(sys.platform, 'irix') != -1:
      return 'irix'
   elif string.find(sys.platform, 'linux') != -1:
      return 'linux'
   elif string.find(sys.platform, 'freebsd') != -1:
      return 'freebsd'
   elif string.find(sys.platform, 'cygwin') != -1:
      return 'win32'
   elif string.find(sys.platform, 'sun') != -1:
      return 'sun'
   elif string.find(sys.platform, 'darwin' ) != -1:
      return 'mac'
   else:
      return sys.platform   
   
   
# -------------------- #
# Path utils
# -------------------- #
def getFullSrcPath(env=SCons.Environment.Environment()):
   """ Return the full path to the local source directory we are in 
       (taking into account BuildDir) """
   # Get the local directory using Dir(.)
   # Then return the string rep of it's src node
   ldir_node = env.Dir('.')                                   # .
   ldir_srcnode = ldir_node.srcnode()                     # /home/.../XXX/src/plx
   return str(ldir_srcnode)
   

def getRelativeSourcePath(env=SCons.Environment.Environment()):
   """ Return the local source path relative to the base build directory
       ie. Dir('#') """
   ldir_node = env.Dir('.')                                   # .
   ldir_srcnode = ldir_node.srcnode()                     # /home/.../XXX/src/plx
   root_dir_node = env.Dir('#')                               # /home/.../XXX
   ldir_src_rpath = ldir_srcnode.get_path(root_dir_node)  # src/plx
   return ldir_src_rpath

def getFullRootPath(env=SCons.Environment.Environment()):
   " Return the full path of the root build dir "
   return str(env.Dir('#'))

def createRelativePath(target, base=os.curdir):
   " Return the given path as a path relative to the given base. "

   if not os.path.exists(target):
      raise OSError, 'Target does not exist: '+target

   if not os.path.isdir(base):
      raise OSError, 'Base is not a directory or does not exist: '+base

   base_list = (os.path.abspath(base)).split(os.sep)
   target_list = (os.path.abspath(target)).split(os.sep)

   # On the windows platform the target may be on a completely different drive from the base.
   if os.name in ['nt','dos','os2'] and base_list[0] != target_list[0]:
      raise OSError, 'Target is on a different drive to base. Target: '+target_list[0].upper()+', base: '+base_list[0].upper()

   # Starting from the filepath root, work out how much of the filepath is
   # shared by base and target.
   for i in range(min(len(base_list), len(target_list))):
      if base_list[i] != target_list[i]: 
         break
   else:  # Loop finished normally (no break)
      # If we broke out of the loop, i is pointing to the first differing path elements.
      # If we didn't break out of the loop, i is pointing to identical path elements.
      # Increment i so that in all cases it points to the first differing path elements.
      i+=1

   rel_list = [os.pardir] * (len(base_list)-i) + target_list[i:]
   return os.path.join(*rel_list)





# ---------- Inprogress ---------- #
import SCons.Node.FS

import fnmatch
import glob

def Glob(match, env=SCons.Environment.Environment()):
    """Similar to glob.glob, except globs SCons nodes, and thus sees
    generated files and files from build directories.  Basically, it sees
    anything SCons knows about."""
    def fn_filter(node):
        fn = str(node)
        return fnmatch.fnmatch(os.path.basename(fn), match)

    from SCons.Scanner.Dir import DirScanner
    scanner = DirScanner()

    children = []
    def add(node):
        print "Adding: ", str(node)
        children.extend(node.all_children())
        children.extend(scanner(node, node.get_build_env()))

    here = env.Dir('.')
    add(here)
    while here.srcnode() != here:
        here = here.srcnode()
        add(here)
	
    print "children: ", [str(c) for c in children]

    nodes = map(env.File, filter(fn_filter, children))

    # Remove duplicates.  O(n2) :(
    rv = []
    for n in nodes:
        if n not in rv:
            rv.append(n)
    return rv 

def Globber( pattern = '*.*', dir = '.', env=SCons.Environment.Environment() ):
    import os, fnmatch
    files = []
    srcdir_abs_path = env.Dir(dir).srcnode().abspath
    #print "srcdir_abs: ", srcdir_abs_path
    for file in os.listdir( srcdir_abs_path ):
        if fnmatch.fnmatch(file, pattern) :
            files.append( os.path.join( dir, file ) )
    return files
    
def WalkBuildFromSourceOld(dir='.', env=SCons.Environment.Environment() ):
    """ Something similar to os.walk() but it is called
        in the build directory and walks over the stuff in the source
	but makes it look like it is relative to the build directory.
    """
    srcdir_abs_path = env.Dir(dir).srcnode().abspath
    #print "src dir abs: ", srcdir_abs_path
    
    # Each dirpath is going to start with the top of the walk
    for dirpath, dirs, filenames in os.walk(srcdir_abs_path):
       if dirpath.startswith(srcdir_abs_path):
          dirpath = dirpath[len(srcdir_abs_path)+1:]   
       bdirpath = os.path.join(dir, dirpath)
       bfiles = filenames
       bdirs = dirs
       yield (bdirpath, bdirs, bfiles)

def WalkBuildFromSource(dir='.', env=SCons.Environment.Environment() ):
    """ Something similar to os.walk() but it is called
        in the build directory and walks over the stuff in the source
	but makes it look like it is relative to the build directory.
    """
    srcdir_abs_path = env.Dir(dir).srcnode().abspath
    #print "src dir abs: ", srcdir_abs_path
    ret_args = []
    
    def collect_args(junk, dir_path, name_list):
        dirs = []
        files = []
        for n in name_list:
            if os.path.isfile(os.path.join(dir_path, n)):
                files.append(n)
            else:
                dirs.append(n)
        ret_args.append( [dir_path, dirs, files] )

    os.path.walk(srcdir_abs_path, collect_args, None)

    # Each dirpath is going to start with the top of the walk
    for dirpath, dirs, filenames in ret_args:
       if dirpath.startswith(srcdir_abs_path):
          dirpath = dirpath[len(srcdir_abs_path)+1:]   
       bdirpath = os.path.join(dir, dirpath)
       bfiles = filenames
       bdirs = dirs
       yield (bdirpath, bdirs, bfiles)

def GlobA(pathname):
    """Return a list of paths matching a pathname pattern.

    The pattern may contain simple shell-style wildcards a la fnmatch.

    """
    glob_magic_check = re.compile('[*?[]')
    fs = SCons.Node.FS.default_fs

    def glob_has_magic(s):
       " Does the glob string contain magic characters."
       return magic_check.search(s) is not None
    
    def find_node(node_name):
       """ Returns node if the given node names exists in the default file system.
           Otherwise returns 0.
       """
       try:
          node = fs.Entry(node_name, create=0)
       except SCons.Errors.UserError:
          return 0
       return node
    
    # If no magic, then just check for the specified file
    if not has_magic(pathname):
       node = find_node(pathname)
       if node != 0:
          return [node]
       else:
          return []

    dirname, basename = os.path.split(pathname)

    if not dirname:
        return Glob_FilesInDir(os.curdir, basename)
    elif has_magic(dirname):                 # If there is magic in dir name then recurse
        list = glob(dirname)
    else:
        list = [dirname]
    
    # Assert: list contains full list of directories to search for file   
    if not has_magic(basename):
        result = []
        for dirname in list:
            if basename or os.path.isdir(dirname):
                name = os.path.join(dirname, basename)
                if os.path.exists(name):
                    result.append(name)
    else:
        result = []
        for dirname in list:
            sublist = Glob_FilesInDir(dirname, basename)
            for name in sublist:
                result.append(os.path.join(dirname, name))
    return result

def Glob_FilesInDir(dirnode, pattern):
   """ Return list of files in directory dirname that match the pattern. """
   children = dirnode.all_children()
        
   #if pattern[0]!='.':
   #   names=filter(lambda x: x[0]!='.',names)
   def fn_filter(node):
      filename = str(node)
      return fnmatch.fnmatch(filename, pattern)
   
   ret_list = [child for child in children if fn_filter(child)]
   return ret_list


def GlobB(match):
    """Similar to glob.glob, except globs SCons nodes, and thus sees
    generated files and files from build directories.  Basically, it sees
    anything SCons knows about."""
    def fn_filter(node):
        fn = str(node)
        return fnmatch.fnmatch(os.path.basename(fn), match)
    
    here = Dir('.')
    
    children = here.all_children()
    nodes = map(File, filter(fn_filter, children))
    node_srcs = [n.srcnode() for n in nodes]

    src = here.srcnode()
    if src is not here:
        src_children = map(File, filter(fn_filter, src.all_children()))
        for s in src_children:
            if s not in node_srcs:
                nodes.append(File(os.path.basename(str(s))))

    return nodes

