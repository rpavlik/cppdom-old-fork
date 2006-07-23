#!python
try:
   import wing.wingdbstub;       # stuff for debugging
   print "Loaded wingdb stub for debugging..."
except:
   pass

import os, string, sys, re, glob, copy, types
#sys.path.insert(0,pj('tools','scons-addons','src'))
print "WARNING:"
print "WARNING: The build is currently in development.  It needs the svn version of scons-addons"
print "WARNING:"

import SCons.Environment
import SCons
import SConsAddons.Util
import SConsAddons.Options as sca_opts
import SConsAddons.Variants as sca_variants
import SConsAddons.Options.CppUnit
import SConsAddons.Options.Boost
from SConsAddons.EnvironmentBuilder import EnvironmentBuilder

# Aliases
GetPlatform = SConsAddons.Util.GetPlatform
Export('GetPlatform')
pj = os.path.join

# ------ HELPER METHODS -------- #
def GetCppDomVersion():
   """Gets the CppDom version from cppdom/version.h.
      Returns version as tuple (major,minor,patch)
   """
   contents = open('cppdom/version.h', 'r').read()
   major = re.compile('.*(#define *CPPDOM_VERSION_MAJOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   minor = re.compile('.*(#define *CPPDOM_VERSION_MINOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   patch = re.compile('.*(#define *CPPDOM_VERSION_PATCH *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   return (int(major), int(minor), int(patch))

def symlinkInstallFunc(dest, source, env):
   """Replacement function for install so it can install source
      to destination by sym linking it.
   """
   os.symlink(pj(os.getcwd(), source), dest)
   return 0

   
# ------ CUSTOM BUILDERS ------------- #
def CreateConfig(target, source, env):
   """ Config script builder 
      Creates the prefix-config file users use to compile against this library 
   """
   targets = map(lambda x: str(x), target)
   sources = map(lambda x: str(x), source)

   submap = env['submap']

   # Build each target from its source
   for i in range(len(targets)):
      print "Generating config file " + targets[i]
      contents = open(sources[i], 'r').read()

      # Go through the substitution dictionary and modify the contents read in
      # from the source file
      for key, value in submap.items():
         contents = re.sub(re.escape(key), value, contents)

      # Write out the target file with the new contents
      open(targets[0], 'w').write(contents)
      os.chmod(targets[0], 0755)

def registerConfigBuilder(env):
   env["BUILDERS"]["ConfigBuilder"] = Builder(action=Action(CreateConfig, varlist=['submap',]))


#------------------------------------------------------------------------------
# Main build setup
#------------------------------------------------------------------------------
EnsureSConsVersion(0,94)
#SourceSignatures('MD5')
#SourceSignatures('timestamp')
SConsignFile()

# Figure out what version of CppDom we're using
CPPDOM_VERSION = GetCppDomVersion()
cppdom_version_str = '%i.%i.%i' % CPPDOM_VERSION
Export('CPPDOM_VERSION')
print 'Building CppDom Version: %s' % cppdom_version_str

platform = SConsAddons.Util.GetPlatform()
unspecified_prefix = "use-instlinks"
buildDir = "build." + platform      
option_filename = "config.cache." + platform

if GetPlatform() == "win32":
   common_env = Environment()
else:
   common_env = Environment(ENV = os.environ)
registerConfigBuilder(common_env)

# Variant setup (get defaults)
default_types    = ["debug","optimized"]
if GetPlatform() == "win32":
   default_types.append("hybrid")
default_libtypes = ["static","shared"]
default_archs    = ["ia32","x64"]

base_bldr = EnvironmentBuilder()

# --------------- #
# --- OPTIONS --- #
# --------------- #
opts = sca_opts.Options(files = [option_filename, 'options.custom'],
                                   args= ARGUMENTS)

cppunit_options = SConsAddons.Options.CppUnit.CppUnit("cppunit", "1.9.10", required=0)
boost_options = SConsAddons.Options.Boost.Boost("boost","1.31.0",required=0)
opts.AddOption(sca_opts.SeparatorOption("\nPackage Options"))
opts.AddOption( cppunit_options )
opts.AddOption( boost_options )
# Variant options 
opts.AddOption(sca_opts.SeparatorOption("\nBuild Variants"))
opts.Add(sca_opts.ListOption('types','Types of run-times to build.(comma separated list)',default_types,default_types))
opts.Add(sca_opts.ListOption('libtypes','Library types to build.(comma separated list)',default_libtypes,default_libtypes))
opts.Add(sca_opts.ListOption('archs','Run-time architectures to build against.(comma separated list)',default_archs,default_archs))
opts.AddOption(sca_opts.SeparatorOption("\nOther settings"))
opts.Add('prefix', 'Installation prefix', unspecified_prefix)
opts.Add('build_test', 'Build the test programs', 'yes')
opts.Add(sca_opts.BoolOption('versioning', 
                             'If no then build only libraries and headers without versioning', True))
if common_env.has_key("MSVS"):
   opts.Add('MSVS_VERSION', 'Set to specific version of MSVS to use. %s'%str(common_env['MSVS']['VERSIONS']), 
            common_env['MSVS']['VERSION'])
base_bldr.addOptions(opts)

opts.Process(common_env)

help_text = """--- CppDom Build system ---
%s
Targets:
   install - Install this puppy
      ex: 'scons install prefix=$HOME/software' to install in your account
   Type 'scons' to just build it
 
"""%(opts.GenerateHelpText(common_env),)

#help_text = opts.GenerateHelpText(common_env) + help_text
Help(help_text)


# --- MAIN BUILD STEPS ---- #
# If we are running the build
if not SConsAddons.Util.hasHelpFlag():
   try:                                   # Try to save the options if possible
      opts.Save(option_filename, common_env)
   except LookupError, le:
      pass
   
   # -- Common builder settings
   base_bldr.readOptions(common_env)
   base_bldr.enableWarnings()   
   
   # If defaulting to instlinks prefix:
   #  - Use symlinks
   #  - Manually set the used prefix to the instlinks of the build dir
   if common_env['prefix'] == unspecified_prefix:
      if hasattr(os,'symlink'):
         common_env['INSTALL'] = symlinkInstallFunc
      common_env['prefix'] = pj( Dir('.').get_abspath(), buildDir, 'instlinks')
   
   # --- Setup installation paths --- #
   base_inst_paths = {}
   base_inst_paths['base'] = os.path.abspath(common_env['prefix'])
   base_inst_paths['lib'] = pj(base_inst_paths['base'], 'lib')
   base_inst_paths['pkgconfig'] = pj(base_inst_paths['lib'], 'pkgconfig')
   base_inst_paths['bin'] = pj(base_inst_paths['base'], 'bin')
   if common_env['versioning'] == True:
      version_suffix = "-%s_%s_%s" % CPPDOM_VERSION
      base_inst_paths['include'] = pj(base_inst_paths['base'], 'include', 
                                 "cppdom-%s.%s.%s" % CPPDOM_VERSION)
   else:
      version_suffix = ''
      base_inst_paths['include'] = pj(base_inst_paths['base'], 'include')
   print "using prefix: ", base_inst_paths['base']         
      
   # Define the variants to use   
   # - variant[key] - ([option_list,], is alternative)
   variants = {}
   variants["type"]    = (common_env["types"], True)
   variants["libtype"] = (common_env["libtypes"], False)
   variants["arch"]    = (common_env["archs"], True)

   # Return list of combos
   # [ {"var":"option", "var2":["op1","op2"], .. }
   var_combos = sca_variants.zipVariants(variants)
   
   print "types: ", common_env["types"] 
   print "libtypes: ", common_env["libtypes"] 
   print "archs: ", common_env["archs"] 
   
   # Apply any common package options
   # Update environment for boost options
   if boost_options.isAvailable():
      boost_options.apply(common_env)
  
   # ---- FOR EACH VARIANT ----- #
   variant_pass = -1                            # Id of the pass, useful for one-time things
   for combo in var_combos:
      variant_pass += 1                  # xxx: standard
      inst_paths = copy.copy(base_inst_paths)
      
      # -- Setup Environment builder --- #
      env_bldr = base_bldr.clone()
            
      # Process modifications for variant combo
      # xxx: standard
      if combo["type"] == "debug":
         env_bldr.enableDebug()
         env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DBG_DLL_RT)
      elif combo["type"] == "optimized":
         env_bldr.enableOpt()
         env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DLL_RT)
      elif combo["type"] == "hybrid":
         env_bldr.enableDebug()
         env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DLL_RT)
      
      if "ia32" == combo["arch"]:
         env_bldr.setCpuArch(EnvironmentBuilder.IA32_ARCH)
      elif "x64" == combo["arch"]:
         env_bldr.setCpuArch(EnvironmentBuilder.X64_ARCH)
         inst_paths['lib'] = inst_paths['lib'] + '64'
         
   
      # --- Build environment --- #   
      baseEnv = env_bldr.applyToEnvironment(common_env.Copy(), variant=combo,options=opts)      

      # Determine the build dir for this variant
      # xxx: common
      dir_parts = ['%s-%s'%(i[0],i[1]) for i in combo.iteritems() if not isinstance(i[1],(types.ListType))]      
      full_build_dir = pj(buildDir,"--".join(dir_parts))
      
      # Build up library name and paths to use
      # xxx: common
      (static_lib_suffix,shared_lib_suffix) = ("","")
      if GetPlatform() == "win32":   
         if combo["type"] == "debug":
            (static_lib_suffix,shared_lib_suffix) = ("_d_s","_d")
         elif combo["type"] == "optimized":
            (static_lib_suffix,shared_lib_suffix) = ("_s","")
         elif combo["type"] == "hybrid":
            (static_lib_suffix,shared_lib_suffix) = ("_h_s","_h")
      else:
         if combo["type"] == "debug":
            inst_paths["lib"] = pj(inst_paths["lib"],"debug")      
            
      cppdom_shared_libname = 'cppdom' + shared_lib_suffix + version_suffix
      cppdom_static_libname = 'cppdom' + static_lib_suffix + version_suffix
      
      # set a library name to use when linking test applications
      if "shared" in combo["libtype"]:
         cppdom_app_libname = cppdom_shared_libname
      elif "static" in combo["libtype"]:
         cppdom_app_libname = cppdom_static_libname
      
      Export('baseEnv','inst_paths','opts', 'variant_pass','combo',
             'cppunit_options', 'boost_options', 
             'cppdom_shared_libname','cppdom_static_libname', 'cppdom_app_libname')

      dirs = ['cppdom']
      if common_env['build_test'] == 'yes':
         dirs.append('test')

      # Process subdirectories
      for d in dirs:
         SConscript(pj(d,'SConscript'), build_dir=pj(full_build_dir, d), duplicate=0)

      # Build up the provides vars for the .fpc files
      inst_paths['pkgconfig'] = pj(inst_paths['lib'],'pkgconfig')
      provides = "cppdom"
      if combo["type"] != "optimized":
         provides += "_%s"%combo["type"]

      arch = "noarch"
      if "ia32" == combo["arch"]:
         arch = "i386"
      elif "x64" == combo["arch"]:
         arch = "x86_64"      

      # - Define a builder for the cppdom.pc file
      # ------------------ Build -config and .pc files ----------------- #
      # Build up substitution map
      submap = {
         '@provides@'                  : provides,
         '@prefix@'                    : base_inst_paths['base'],
         '@exec_prefix@'               : '${prefix}',
         '@cppdom_cxxflags@'           : '',
         '@includedir@'                : base_inst_paths['include'],
         '@cppdom_extra_cxxflags@'     : '',
         '@cppdom_extra_include_dirs@' : '',
         '@cppdom_libs@'               : "-l%s" % cppdom_shared_libname,
         '@libdir@'                    : base_inst_paths['lib'],
         '@arch@'                      : arch,
         '@version@'                   : cppdom_version_str
      }

      # Setup the builder for cppdom.pc
      if GetPlatform() != 'win32':
         name_parts = ['cppdom',cppdom_version_str,arch]
         if combo["type"] != "optimized":
            name_parts.append(combo["type"])
         pc_filename = "-".join(name_parts) + ".fpc"
         cppdom_pc  = baseEnv.ConfigBuilder(pj(inst_paths['pkgconfig'],pc_filename), 
                                        'cppdom.fpc.in', submap=submap)
         baseEnv.AddPostAction (cppdom_pc, Chmod('$TARGET', 0644))
         baseEnv.Depends(cppdom_pc, 'cppdom/version.h')

      ## Setup the builder for cppdom-config
      #if GetPlatform() != 'win32':
      #   env = common_env.Copy(BUILDERS = builders)
      #   cppdom_config  = env.ConfigBuilder(pj(inst_paths['bin'],'cppdom-config'), 
      #                                      'cppdom-config.in', submap=submap )
      #   env.AddPostAction (cppdom_config, Chmod('$TARGET', 0755))
      #   env.Depends(cppdom_config, 'cppdom/version.h')

   
   common_env.Alias('install', inst_paths['base'])

   # Close up with aliases and defaults   
   Default('.')

