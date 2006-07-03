#!python
try:
   import wing.wingdbstub;       # stuff for debugging
   print "Loaded wingdb stub for debugging..."
except:
   pass

import os, string, sys, re, glob
#sys.path.insert(0,pj('tools','scons-addons','src'))
print "WARNING:"
print "WARNING: The build may not currently work.  Please use revision 1.56 for now."
print "WARNING:  'cvs update -r 1.56 SConstruct' "
print "WARNING:"

import SCons.Environment
import SCons
import SConsAddons.Util
import SConsAddons.Options as sca_opts
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
   return 0

def CreatePkgConfig(target, source, env):
   """ Pkg Config file builder.
      Creates the pkgconfig .pc file users use to compile against this library 
   """
   targets = map(lambda x: str(x), target)
   sources = map(lambda x: str(x), source)

   submap = env['submap']

   # Build each target from its source
   for i in range(len(targets)):
      print "Generating pkg-config file " + targets[i]
      contents = open(sources[i], 'r').read()

      # Go through the substitution dictionary and modify the contents read in
      # from the source file
      for key, value in submap.items():
         contents = re.sub(re.escape(key), value, contents)

      # Write out the target file with the new contents
      open(targets[0], 'w').write(contents)
      os.chmod(targets[0], 0644)
   return 0


# --- Platform specific environment factory methods --- #
def BuildIRIXEnvironment():
   "Builds a base environment for other modules to build on set up for IRIX"
   global optimize, profile, builders

   CXXFLAGS = ['-n32', '-mips3', '-LANG:std', '-w2']
   LINKFLAGS = CXXFLAGS

   # Enable profiling?
   if profile != 'no':
      CXXFLAGS.extend([])
      LINKFLAGS.extend([])

   # Debug or optimize build?
   if optimize != 'no':
      CXXFLAGS.extend(['-DNDEBUG', '-O2'])
   else:
      CXXFLAGS.extend(['-D_DEBUG', '-g', '-gslim'])

   return Environment(
      ENV         = os.environ,
      CXXFLAGS    = CXXFLAGS,
      LINKFLAGS   = LINKFLAGS
   )



#------------------------------------------------------------------------------
# Main build setup
#------------------------------------------------------------------------------
EnsureSConsVersion(0,94)
#SourceSignatures('MD5')
SourceSignatures('timestamp')
SConsignFile()

# Figure out what version of CppDom we're using
CPPDOM_VERSION = GetCppDomVersion()
Export('CPPDOM_VERSION')
print 'Building CppDom Version: %i.%i.%i' % CPPDOM_VERSION

option_env = Environment()
platform = SConsAddons.Util.GetPlatform()
unspecified_prefix = "use-instlinks"
default_libdir = 'lib'

# --- OPTIONS --- #
option_filename = "config.cache." + platform
opts = sca_opts.Options(files = [option_filename, 'options.custom'],
                                   args= ARGUMENTS)

cppunit_options = SConsAddons.Options.CppUnit.CppUnit("cppunit", "1.9.10", required=0)
boost_options = SConsAddons.Options.Boost.Boost("boost","1.31.0",required=0)
opts.AddOption( cppunit_options )
opts.AddOption( boost_options )
opts.Add('prefix', 'Installation prefix', unspecified_prefix)
opts.Add('libdir', 'Library installation directory under <prefix>', default_libdir)
opts.AddOption( sca_opts.BoolOption('optimize','If true, generate optimized code.',False))
opts.AddOption( sca_opts.BoolOption('profile','If true, generate profiled code.',False))
opts.Add('build_test', 'Build the test programs', 'yes')
opts.Add('static_only', 'If not "no" then build only static library', 'no')
opts.Add('versioning', 'If no then build only libraries and headers without versioning', 'yes')
opts.Add('MakeDist', 'If true, make the distribution packages as part of the build', 'no')
opts.Add('arch', 'CPU architecture (ia32, x86_64, or ppc)',
         SConsAddons.Util.GetArch())
opts.Add('universal', 'Build universal binaries (Mac OS X only)',
         'yes')
opts.Add('sdk', 'Platform SDK (Mac OS X only)', '')
if option_env.has_key("MSVS"):
   opts.Add('MSVS_VERSION', 'Set to specific version of MSVS to use. %s'%str(option_env['MSVS']['VERSIONS']), 
            option_env['MSVS']['VERSION'])
  
help_text = """--- CppDom Build system ---
Targets:
   install - Install this puppy
      ex: 'scons install prefix=$HOME/software' to install in your account
   Type 'scons' to just build it
 
"""
if SConsAddons.Util.hasHelpFlag():
   opts.Update(option_env)
   help_text = help_text + opts.GenerateHelpText(option_env)
   Help(help_text)

# --- MAIN BUILD STEPS ---- #
# If we are running the build
if not SConsAddons.Util.hasHelpFlag():   
   if cpu_arch == 'x86_64':
      default_libdir = 'lib64'
   
   if GetPlatform() == 'mac' and universal == 'yes':
      buildDir = "build.%s-universal" % platform
   elif cpu_arch != cpu_arch_default:
      buildDir = "build.%s-%s" % (platform, cpu_arch)
   else:
      buildDir = "build." + platform
   
   def_prefix = pj( Dir('.').get_abspath(), buildDir, 'instlinks')   
   
   # Create the extra builders
   # Define a builder for the cppdom-config script
   # Define a builder for the cppdom.pc file
   builders = {
      'ConfigBuilder'   : Builder(action = CreateConfig),
      'PkgConfigBuilder'   : Builder(action = CreatePkgConfig)
   }
   
   
   # Create and export the base environment
   env_bldr = EnvironmentBuilder()
   #env_bldr.enableWarnings(EnvironmentBuilder.MAXIMUM)
   env_bldr.enableWarnings(EnvironmentBuilder.MINIMAL)
   
   if optimize:
      env_bldr.enableOpt(EnvironmentBuilder.STANDARD)
      env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DLL_RT)
   else:
      env_bldr.enableDebug()
      env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DBG_DLL_RT)
   
   if profile:
      env_bldr.enableProfiling()
   
   #baseEnv = env_bldr.buildEnvironment(ENV = os.environ)  
   #baseEnv = env_bldr.buildEnvironment(MSVS_VERSION="7.0")
   baseEnv = env_bldr.buildEnvironment()
   
   if GetPlatform() == 'irix':
      baseEnv = BuildIRIXEnvironment()
      opts.Update(baseEnv)
   elif GetPlatform() == 'win32':
      baseEnv = env_bldr.buildEnvironment(options=opts)
   else:
      baseEnv = env_bldr.buildEnvironment(ENV = os.environ,
                                          options=opts)   
   
   Export('baseEnv')

   # Finish up option processing and saving
   help_text = help_text + opts.GenerateHelpText(option_env)
   Help(help_text)
   
   try:                                   # Try to save the options if possible
      opts.Save(option_filename, baseEnv)
   except LookupError, le:
      pass

   

   # Update environment for boost options
   if boost_options.isAvailable():
      boost_options.updateEnv(baseEnv)

   # If defaulting to instlinks prefix:
   #  - Use symlinks
   #  - Manually set the used prefix to the instlinks of the build dir
   if baseEnv['prefix'] == unspecified_prefix:
      if hasattr(os,'symlink'):
         baseEnv['INSTALL'] = symlinkInstallFunc
      baseEnv['prefix'] = def_prefix

   # Setup installation paths
   inst_paths = {}
   inst_paths['base'] = os.path.abspath(baseEnv['prefix'])
   inst_paths['lib'] = pj(inst_paths['base'], baseEnv['libdir'])
   inst_paths['bin'] = pj(inst_paths['base'], 'bin')
   if baseEnv['versioning'] == 'yes':
      inst_paths['include'] = pj(inst_paths['base'], 'include', 
                                 "cppdom-%s.%s.%s" % CPPDOM_VERSION)
   else:
      inst_paths['include'] = pj(inst_paths['base'], 'include')
      

   print "using prefix: ", inst_paths['base']   
   
   dirs = ['cppdom']
   if baseEnv['build_test'] == 'yes':
      dirs.append('test')

   # Build up library name to use
   if GetPlatform() == "win32":   
      static_lib_suffix = "_s"
      shared_lib_suffix = ""
      if not optimize:
         static_lib_suffix += "_d"
         shared_lib_suffix += "_d"
   else:
      static_lib_suffix = ""
      shared_lib_suffix = ""      
   
   if baseEnv['versioning'] == 'yes':
      version_suffix = "-%s_%s_%s" % CPPDOM_VERSION
   else:
      version_suffix = ''
      
   cppdom_shared_libname = 'cppdom' + shared_lib_suffix + version_suffix
   cppdom_static_libname = 'cppdom' + static_lib_suffix + version_suffix

   Export('baseEnv','inst_paths','cpu_arch', 'universal', 'sdk', 'opts', 'optimize',
          'cppunit_options', 'boost_options', 
          'cppdom_shared_libname','cppdom_static_libname')
   
   variants = {}
   variants["performance"] = ["debug","optimized"]   
   variants["runtime"] = ["debug","standard"]
   variants["libtype"] = ["static","dynamic"]
   #variants["arch"] = ["32","64"]
   
   variant_pass = 0
     
   #for each variant:
   #   build environment 
   #   setup options 
   #   
   #   if "win32" == GetPlatform():
   #      
   #   else:
   #      if variant["runtime"] = "debug":
   #         inst_path['lib'] += 'debug'
   #   build_dir = platform + variant_desc 
   #   for d in dirs:
   #      SConscript(pj(d,'SConscript'), build_dir=pj(buildDir,d), duplicate=0)

   # Process subdirectories
   for d in dirs:
      SConscript(pj(d,'SConscript'), build_dir=pj(buildDir, d), duplicate=0)


   # ------------------ Build -config and .pc files ----------------- #
   # Build up substitution map
   submap = {
      '@prefix@'                    : inst_paths['base'],
      '@exec_prefix@'               : '${prefix}',
      '@cppdom_cxxflags@'           : '',
      '@includedir@'                : inst_paths['include'],
      '@cppdom_extra_cxxflags@'     : '',
      '@cppdom_extra_include_dirs@' : '',
      '@cppdom_libs@'               : "-l%s" % cppdom_shared_libname,
      '@libdir@'                    : inst_paths['lib'],
      '@lib_subdir@'                : baseEnv['libdir'],
      '@VERSION_MAJOR@'             : str(CPPDOM_VERSION[0]),
      '@VERSION_MINOR@'             : str(CPPDOM_VERSION[1]),
      '@VERSION_PATCH@'             : str(CPPDOM_VERSION[2]),
   }

   # Setup the builder for cppdom-config
   if GetPlatform() != 'win32':
      env = baseEnv.Copy(BUILDERS = builders)
      cppdom_config  = env.ConfigBuilder('cppdom-config', 'cppdom-config.in', submap=submap )

      env.Depends('cppdom-config', 'cppdom/version.h')
      env.Install(inst_paths['bin'], cppdom_config)

   # Setup the builder for cppdom.pc
   if GetPlatform() != 'win32':
      env = baseEnv.Copy(BUILDERS = builders)
      cppdom_pc  = env.PkgConfigBuilder("cppdom.pc", 'cppdom.pc.in', submap=submap)
      env.Install(pj(inst_paths['lib'],'pkgconfig'), cppdom_pc)

      env.Depends('cppdom.pc', 'cppdom/version.h')
      env.Alias('install', inst_paths['base'])

   # Close up with aliases and defaults   
   Default('.')

