"""
Small custom builders useful for scons-addons
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

import os, sys, re, types, string, random
import SConsAddons.Util as sca_util
import SConsAddons.EnvironmentBuilder as sca_envbldr
import SCons.Defaults
import SCons.Environment
import SCons.Node.FS
import SCons.Util

def CreateSubst(target, source, env):
   """ Custom builder helpful for creating *-config scripts and just about anything
       else that can be based on substitutability from a map.
      
      The builder works by pulling the variable 'submap' out of the environment
      and then any place in the source where key from the map exists,
      that content is replaced with the value of the key in the dictionary.
      
      Example:
         submap = {
         '@prefix@'                    : my_prefix,
         '@version@'                   : version_str
      }

         my_file = env.SubstBuilder('file.out','file.in', submap=submap)
         env.AddPostAction (my_file, Chmod('$TARGET', 0644))
         env.Depends(my_file, 'version.h')
   """
   targets = map(lambda x: str(x), target)
   sources = map(lambda x: str(x), source)

   submap = env['submap']

   # Build each target from its source
   for i in range(len(targets)):
      #print "Generating file " + targets[i]
      contents = open(sources[i], 'r').read()

      # Go through the substitution dictionary and modify the contents read in
      # from the source file
      for key, value in submap.items():
         try:
            contents = contents.replace(key, value);
         except TypeError:
            print "Type error for value in key: %s" % key

      # Write out the target file with the new contents
      open(targets[i], 'w').write(contents)
      os.chmod(targets[i], 0755)

def generate_builder_str(target, source, env):
   builderStr = "generating: %s"%[str(t) for t in target]
   return builderStr

def registerSubstBuilder(env):
   env["BUILDERS"]["SubstBuilder"] = \
            SCons.Builder.Builder(action=SCons.Action.Action(CreateSubst, 
                                                             generate_builder_str,
                                                             varlist=['submap',]))


def CreateStringFormatBuilder(target, source, env):
   """ Custom builder helpful for creating just about anything that can use
       standard string substitutation.

       The builder works by replacing standard string replacement variables
       ( %(example)s ) within a string with values from a map.
      
      Example:
         submap = {
         'prefix'    : my_prefix,
         'version'   : version_str
      }

         my_file = env.StringFormatBuilder('file.out','file.in', submap=submap)
         env.AddPostAction (my_file, Chmod('$TARGET', 0644))
         env.Depends(my_file, 'version.h')
   """
   targets = map(lambda x: str(x), target)
   sources = map(lambda x: str(x), source)

   submap = env['submap']

   # Build each target from its source
   for i in range(len(targets)):
      #print "Generating file " + targets[i]
      contents = open(sources[i], 'r').read()

      # Perform string subsittution
      new_contents = contents % submap       

      # Write out the target file with the new contents
      open(targets[i], 'w').write(new_contents)
      os.chmod(targets[i], 0755)

def registerStringFormatBuilder(env):
   env["BUILDERS"]["StringFormatBuilder"] = \
            SCons.Builder.Builder(action=SCons.Action.Action(CreateStringFormatBuilder,
                                                             generate_builder_str,
                                                             varlist=['submap',]))


def randomHeaderGuard(length):
   chars = string.letters
   random_str = ""
   for i in range(length):
      random_str += random.choice(chars)
   return random_str
   
def CreateDefineBuilder(target, source, env):
   """ Custom builder for creating a define file.

      The builder works by creating "#define" values in a file.      
      If value is a list, then first entry is the value and second is help text.
      There is a special case when the value is 'False' as a bool.  In this case
      the variable is not even defined.
      
      Example:
      definemap = {
         'USES_OPTION1'   : True,
         'USES_OPTION2'   : (5,"Value to set something")
      }
      
      my_file = env.StringFormatBuilder('file.out','file.in', definemap=definemap, headerguard="MY_FILE_H")
      env.AddPostAction (my_file, Chmod('$TARGET', 0644))
      env.Depends(my_file, 'version.h')
   """
   targets = map(lambda x: str(x), target)
   sources = map(lambda x: str(x), source)
   definemap = env['definemap']

   # Build each target from its source
   for i in range(len(targets)):
      #print "Generating file " + targets[i]
      content = ""
      for (define,value) in definemap.iteritems():
         help_text = None
         if isinstance(value, (types.ListType,types.TupleType)):
            help_text = value[1]
            value = value[0]
         if help_text:
            content += "/* %s */\n"%help_text
         if isinstance(value, types.BooleanType):     # Convert from bool to int
            value = int(value)
            if not value:
               content += "/* #define %s %s */\n\n"%(define,str(value))
               continue            
         content += "#define %s %s\n\n"%(define,str(value))
      #guard = "_GUARD_%s_"%randomHeaderGuard(8) 
      # Compute a guard that will remain stable when file contents don't change
      import md5
      guard = "_GUARD_%s_"%md5.new(content).hexdigest()      
      if env.has_key("headerguard"):
         guard = env["headerguard"]
      content = "#ifndef %(guard)s\n#define %(guard)s\n\n%(content)s\n\n#endif\n"%vars()
      
      # Write out the target file with the new contents
      open(targets[i], 'w').write(content)      

def registerDefineBuilder(env):
   env["BUILDERS"]["DefineBuilder"] = \
            SCons.Builder.Builder(action=SCons.Action.Action(CreateDefineBuilder,
                                                             generate_builder_str,
                                                             varlist=['definemap','headerguard']))
