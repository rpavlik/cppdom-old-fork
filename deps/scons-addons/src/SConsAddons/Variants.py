"""
Variants module.  Currently this is just a place to dump
common code used for variant handling.
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

import os, sys, re, types
import SConsAddons.Util as sca_util
from SConsAddons.EnvironmentBuilder import EnvironmentBuilder, detectValidArchs
import SCons.Defaults
import SCons.Environment
import SCons.Node.FS
import SCons.Util
from SConsAddons.Util import GetPlatform, GetArch


class VariantsHelper(object):
   """ Helper class for managing builds using variants and some standard conventions.
       Note: This class may not be fully general as it is setup to meet some conventions
       that I have found helpful but not necessarily what everyone else may want.
       Also note that the class may end up feeling rather monolithic.  Once again,
       this is simply because it is trying to reuse code across multiple builds.
       
       variantKeys - List of default variant keys to use.  Valid values include:
          type - runtime type (debug,optimized,debugrt)
          libtype - shared,static
          arch - ia32, x64, ppc, ppc64, etc
   """

   def __init__(self, variantKeys=["type","libtype","arch"]):
      
      # List of variants that we are using.
      # - variants[key] - [[option_list,], is alternative]
      self.variants = {}
      self.fillDefaultVariants(variantKeys)
      
      
   def fillDefaultVariants(self, varKeys):
      """ Fill the variants variable with default allowable settings. """
      if "type" in varKeys:
         self.variants["type"] = [["debug","optimized"], True]
         if sca_util.GetPlatform() == "win32":
            self.variants["type"][0].append("debugrt")
      
      if "libtype" in varKeys:
         libtype_is_alternative = False

         # On Windows, SCons does not use the object file extension to
         # distinguish between object files compiled for static and dynamic
         # libraries. We help out by distinguishing the two by directory.
         if sca_util.GetPlatform() == "win32":
            libtype_is_alternative = True

         self.variants["libtype"] = [["shared","static"], 
                                     libtype_is_alternative]

      if "arch" in varKeys:
         valid_archs = detectValidArchs()
         if len(valid_archs) == 0:
            valid_archs = ["default"]
         print "Valid archs: ", valid_archs
         self.variants["arch"] = [valid_archs[:], True]
      else:
         self.variants["arch"] = [["default"], True]

   def iterate(self, vars, baseEnvBuilder, baseEnv=None):
      """
         vars: locals() to use
         baseEnvBuilder: Environment builder to start with
         baseEnv: baseEnvironment to start with, if none, don't build environment
         
         Local variables exported:
            variant_pass: Iterates from 0 to number of combos            
            combo_dir: build dire parts made from combo strings
            static_lib_suffix,shared_lib_suffix: Suffix values to use
            env_builder: Created and modified environment builder
            build_env: Created build environment to use (based on baseEnv) with:
                   - "variant" - contains combo
            
      """
      variant_pass = -1
      var_combos = zipVariants(self.variants)
      
      # Main iteration
      for combo in var_combos:      
         variant_pass += 1
         # -- Setup Environment builder --- #
         env_bldr = baseEnvBuilder.clone()
            
         # Process modifications for variant combo            
         if combo["type"] == "debugrt":
            env_bldr.enableDebug()
            env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DBG_DLL_RT)
         elif combo["type"] == "optimized":
            env_bldr.enableOpt()
            env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DLL_RT)
         elif combo["type"] == "debug":
            env_bldr.enableDebug()
            env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DLL_RT)
         
         if "ia32" == combo["arch"]:
            env_bldr.setCpuArch(EnvironmentBuilder.IA32_ARCH)
         elif "x64" == combo["arch"]:
            env_bldr.setCpuArch(EnvironmentBuilder.X64_ARCH)
         elif "ia64" == combo["arch"]:
            env_bld.setCpuArch(EnvironmentBuilder.IA64_ARCH)
         elif "ppc" == combo["arch"]:
            env_bldr.setCpuArch(EnvironmentBuilder.PPC_ARCH)
         elif "ppc64" == combo["arch"]:
            env_bldr.setCpuArch(EnvironmentBuilder.PPC64_ARCH)
         elif "universal" == combo["arch"]:
            env_bldr.setCpuArch(EnvironmentBuilder.UNIVERSAL_ARCH)

         # Build up library name and paths to use
         # xxx: common
         (static_lib_suffix,shared_lib_suffix) = ("","")
         if GetPlatform() == "win32":   
            if combo["type"] == "debug" or  combo["type"] == "optimized":
               (static_lib_suffix,shared_lib_suffix) = ("_s","")
            elif combo["type"] == "debugrt":
               (static_lib_suffix,shared_lib_suffix) = ("_d_s","_d")

         # Set the directory to install libraries into.
         if combo["type"] == "debug":
            lib_subdir = "debug"
         else:
            lib_subdir = ""

         # Suffix to add to the end of apps.
         runtime_suffix = ""
         if combo["type"] == "debug":
            runtime_suffix = "_d"
         elif combo["type"] == "debugrt":
            runtime_suffix = "_drt"

         # Determine the build dir for this variant
         combo_dir = "--".join(['%s-%s'%(i[0],i[1]) for i in combo.iteritems() if not isinstance(i[1],(types.ListType))])
      
         # --- Build environment if needed--- #   
         build_env = None
         if baseEnv:
            build_env = env_bldr.applyToEnvironment(baseEnv.Clone(), variant=combo)      
         
         # export the locals
         vars["variant_pass"] = variant_pass
         vars["combo_dir"] = combo_dir
         vars["static_lib_suffix"] = static_lib_suffix
         vars["shared_lib_suffix"] = shared_lib_suffix
         vars["env_builder"] = env_bldr
         vars["build_env"] = build_env
         vars["lib_subdir"] = lib_subdir
         vars["runtime_suffix"] = runtime_suffix
         
         yield combo
         # Yield the combo



   # ---- Command-line option processing ---- #
   def addOptions(self, opts):
      """ The VariantHelper has support for adding command line options to an
          option processing object.  This object has to be an instance
          of SConsAddons.Options.   
          The key for the options is the variant 'var_' + key
      """
      import SConsAddons.Options as sca_opts      
      assert isinstance(opts, sca_opts.Options)
      
      known_help_text = {"type":"Types of run-times to build.",
                         "libtype":"Library types to build.",
                         "arch":"Target processor architectures to build."}

      opts.AddOption(sca_opts.SeparatorOption("\nVariant Helper Options"))      
      
      var_keys = self.variants.keys()
      var_keys.sort()
      for key in var_keys:
         option_key_name = 'var_' + key
         option_help = known_help_text.get(key,"Variant option")
         option_allowed = self.variants[key][0][:]
         opts.Add(sca_opts.ListOption(option_key_name,option_help,
                                      option_allowed,option_allowed))      
   
   def readOptions(self, optEnv):
      """ Read the processed options from the given environment. """
      # For each key, if found in environment, copy the list over to the variant
      var_keys = self.variants.keys()
      for key in var_keys:         
         opt_key_name = "var_" + key
         if optEnv.has_key(opt_key_name):            
            self.variants[key][0] = optEnv[opt_key_name][:]



def zipVariants(variantMap):
   """ This method takes a map of variants and items within each variant and returns
       a list of all combinations of ways that the variants can be combined.

       The input format is:
       { key : ([option_list,], is_alternative), }
       - option_list is a list of all items for this variant.
       - is_alternative is a flag saying wether we just need to choose one item or if all
         items can be in the same variant combination

       The return format is:         
       [ {"var":"option", "var2":["op1","op2"]}, .. }
       
       Each entry in the list is a dictionary that fully specfies a combination of
       variant keys and associated items.
       
       Usage:
         # Define the variants to use   
         # - variant[key] - ([option_list,], is alternative)
         variants = {}
         variants["type"]    = (common_env["types"], True)
         variants["libtype"] = (common_env["libtypes"], False)
         variants["arch"]    = (common_env["archs"], True)
   
         # Return list of combos
         # [ {"var":"option", "var2":["op1","op2"], .. }
         var_combos = zipVariants(variants)
    """
          
   # List of (key,[varlist,])
   alt_items = [ (i[0],i[1][0]) for i in variantMap.iteritems() if i[1][1] == True]
   always_items = [ (i[0],i[1][0]) for i in variantMap.iteritems() if i[1][1] == False]
   assert len(alt_items) + len(always_items) == len(variantMap)
   
   alt_item_sizes = [len(i[1]) for i in alt_items]    # Length of the alt lists
   
   # Build up list of
   # [ (key,'option"), (key2,"option"), ...]
   cur_combos=[[]]
   for variant in alt_items:
      new_combos = []
      variant_key = variant[0]
      option_list = variant[1]
      for option in option_list:
         for i in cur_combos:
            new_combos.append(i+[(variant_key,option)])
      cur_combos = new_combos
   
   #print cur_combos
   
   # Now turn the list of combo lists into a list of
   # combo dictionaries
   ret_combos = []
   for c in cur_combos:
      combo = {}
      for i in c:
         combo[i[0]] = i[1]
      for i in always_items:
         combo[i[0]] = i[1]
      ret_combos.append(combo)

   #import pprint
   #pprint.pprint(ret_combos)
   
   return ret_combos

   
