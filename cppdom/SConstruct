#!python
try:
   import wing.wingdbstub;       # stuff for debugging
   print "Loaded wingdb stub for debugging..."
except:
   pass

import os, string, sys, re, glob
import distutils.util
pj = os.path.join

#sys.path.insert(0,pj('tools','scons-addons','src'))

Default('.')

import SCons.Environment
import SCons
import SConsAddons.Util
import SConsAddons.Options
import SConsAddons.Options.CppUnit
import SConsAddons.Options.Boost
from SConsAddons.EnvironmentBuilder import EnvironmentBuilder

#------------------------------------------------------------------------------
# Define some generally useful functions
#------------------------------------------------------------------------------
def GetCppDomVersion():
   """Gets the CppDom version from cppdom/version.h.
      Returns version as tuple (major,minor,patch)
   """
   contents = open('cppdom/version.h', 'r').read()
   major = re.compile('.*(#define *CPPDOM_VERSION_MAJOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   minor = re.compile('.*(#define *CPPDOM_VERSION_MINOR *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   patch = re.compile('.*(#define *CPPDOM_VERSION_PATCH *(\d+)).*', re.DOTALL).sub(r'\2', contents)
   return (int(major), int(minor), int(patch))

GetPlatform = SConsAddons.Util.GetPlatform
Export('GetPlatform')


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
   """ Config script builder 
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

def symlinkInstallFunc(dest, source, env):
    """Install a source file into a destination by sym linking it."""
    os.symlink(pj(os.getcwd(), source), dest)
    return 0

# --- Platform specific environment factory methods --- #
def BuildLinuxEnvironment():
   "Builds a base environment for other modules to build on set up for linux"
   global optimize, profile, builders, cpu_arch

   env = Environment(ENV = os.environ) 
   
   ## Arch specific flags
   #if cpu_arch == 'ia32':
   #   CXXFLAGS.extend(['-m32'])
   #   LINKFLAGS.extend(['-m32'])
   #elif cpu_arch == 'x86_64':
   #   CXXFLAGS.extend(['-m64'])
   #   LINKFLAGS.extend(['-m64'])
   #elif cpu_arch != '':
   #   print "WARNING: Unknown CPU archtecture", cpu_arch

   return env

def BuildDarwinEnvironment():
   "Builds a base environment for other modules to build on set up for Darwin"
   global optimize, profile, builders, cpu_arch_default, universal

   env = Environment(ENV = os.environ)

   if not os.environ.has_key('CXXFLAGS'):
      CXXFLAGS = ['-Wall']
      LINKFLAGS = ['']

      env.Append(CXXFLAGS = CXXFLAGS, LINKFLAGS = LINKFLAGS)

   # Enable profiling?
   if profile != 'no':
      env.Append(CXXFLAGS = ['-pg'])
      env.Append(LINKFLAGS = ['-pg'])

   # Debug or optimize build?
   if optimize != 'no':
      env.Append(CXXFLAGS = ['-DNDEBUG', '-O2'])
   else:
      env.Append(CXXFLAGS = ['-D_DEBUG', '-g'])

   if universal != 'yes':
      if cpu_arch != cpu_arch_default:
         if cpu_arch == 'ia32':
            env.Append(CXXFLAGS = ['-arch', 'i386'],
                       LINKFLAGS = ['-arch', 'i386'])
         elif cpu_arch == 'ppc':
            env.Append(CXXFLAGS = ['-arch', 'ppc'],
                       LINKFLAGS = ['-arch', 'ppc'])
   else:
      env.Append(CXXFLAGS = ['-arch', 'ppc', '-arch', 'i386'],
                 LINKFLAGS = ['-arch', 'ppc', '-arch', 'i386'])

   if sdk != '':
      env.Append(CXXFLAGS = ['-isysroot', sdk],
                 LINKFLAGS = ['-isysroot', sdk])

      sdk_re = re.compile('MacOSX(10\..*?)u?\.sdk')
      match = sdk_re.search(sdk)
      if match is not None:
         min_osx_ver = '-mmacosx-version-min=' + match.group(1)
         env.Append(CXXFLAGS = [min_osx_ver], LINKFLAGS = [min_osx_ver])

   return env

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

def BuildSunEnvironment():
   "Builds a base environment for other modules to build on set up for linux"
   global optimize, profile, builders

   CXXFLAGS = ['-Wall', '-fexceptions']
   LINKFLAGS = []

   # Enable profiling?
   if profile != 'no':
      CXXFLAGS.extend(['-pg'])
      LINKFLAGS.extend(['-pg'])

   # Debug or optimize build?
   if optimize != 'no':
      CXXFLAGS.extend(['-DNDEBUG', '-O2'])
   else:
      CXXFLAGS.extend(['-D_DEBUG', '-g'])

   ret_env = Environment( ENV = os.environ )
   
   # Override the tool chains used
   for t in ['gcc', 'gnulink', 'g++']:
      Tool(t)(ret_env)    
   
   ret_env['CXXFLAGS'] = CXXFLAGS;
   return ret_env;

def BuildWin32Environment():
   env = Environment(ENV = os.environ)
   for t in ['msvc', 'mslink']:
      Tool(t)(env)

   env['LINKFLAGS'] += ' /subsystem:console /incremental:no'
   env['CXXFLAGS'] += '/DWIN32 /D_WINDOWS /D_USRDLL /DCPPDOM_EXPORTS /Zm800 /GX /GR /Op /Zc:wchar_t,forScope'

   if optimize != 'no':
      env['CXXFLAGS'] += ' /Ogity /O2 /Gs /Ob2 /MD /DNDEBUG'
      env['LINKFLAGS'] += ' /RELEASE'
   else:   
      env['CXXFLAGS'] += ' /Z7 /Od /Ob0 /MDd /D_DEBUG'
      env['LINKFLAGS'] += ' /DEBUG'

   return env

def BuildCygwinEnvironment():
   "Builds a base environment for other modules to build on set up for Cygwin"
   global optimize, profile, builders

   CXX = 'g++'
   LINK = CXX
   CXXFLAGS = ['-ftemplate-depth-256', '-Wall', '-pipe']
   LINKFLAGS = []

   # Enable profiling?
   if profile != 'no':
      CXXFLAGS.extend(['-pg'])
      LINKFLAGS.extend(['-pg'])

   # Debug or optimize build?
   if optimize != 'no':
      CXXFLAGS.extend(['-DNDEBUG', '-g', '-O3'])
   else:
      CXXFLAGS.extend(['-D_DEBUG', '-g'])

   env = Environment(ENV=os.environ, CXX=CXX, LINK=LINK)
   env.Append(CXXFLAGS=CXXFLAGS, LINKFLAGS=LINKFLAGS)
   return env

#------------------------------------------------------------------------------
# Main build setup
#------------------------------------------------------------------------------
EnsureSConsVersion(0,94)
SourceSignatures('MD5')
#SourceSignatures('timestamp')
SConsignFile()

# Figure out what version of CppDom we're using
CPPDOM_VERSION = GetCppDomVersion()
Export('CPPDOM_VERSION')
print 'Building CppDom Version: %i.%i.%i' % CPPDOM_VERSION


# Get command-line arguments
optimize = ARGUMENTS.get('optimize', 'no')
profile = ARGUMENTS.get('profile', 'no')

platform = distutils.util.get_platform()

cpu_arch_default = SConsAddons.Util.GetArch()
universal_default = 'yes'
sdk_default = ''

cpu_arch = ARGUMENTS.get('arch', cpu_arch_default)
universal = ARGUMENTS.get('universal', universal_default)
sdk = ARGUMENTS.get('sdk', sdk_default)

default_libdir = 'lib'
if cpu_arch == 'x86_64':
   default_libdir = 'lib64'

if GetPlatform() == 'mac' and universal == 'yes':
   buildDir = "build.%s-universal" % platform
elif cpu_arch != cpu_arch_default:
   buildDir = "build.%s-%s" % (platform, cpu_arch)
else:
   buildDir = "build." + platform

def_prefix = pj( Dir('.').get_abspath(), buildDir, 'instlinks')
unspecified_prefix = "use-instlinks"
               
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

if optimize != 'no':
   env_bldr.enableOpt(EnvironmentBuilder.STANDARD)
   env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DLL_RT)
else:
   env_bldr.enableDebug()
   env_bldr.setMsvcRuntime(EnvironmentBuilder.MSVC_MT_DBG_DLL_RT)

if profile != 'no':
   env_bldr.enableProfiling()

#baseEnv = env_bldr.buildEnvironment(ENV = os.environ)  
#baseEnv = env_bldr.buildEnvironment(MSVS_VERSION="7.0")
baseEnv = env_bldr.buildEnvironment()

print "Version: ", baseEnv["MSVS"]["VERSION"]
print "Versions: ", baseEnv["MSVS"]["VERSIONS"]

#baseEnv["MSVS_VERSION"] = "7.0"

if GetPlatform() == 'irix':
   baseEnv = BuildIRIXEnvironment()
elif GetPlatform() == 'sun':
   baseEnv = BuildSunEnvironment()
#else:
#   print 'Unsupported build environment: ' + GetPlatform()
#   print 'Attempting to use standard SCons toolchains.'
#   baseEnv = Environment()

Export('baseEnv')

# --- OPTIONS --- #
option_filename = "config.cache." + platform
opts = SConsAddons.Options.Options(files = [option_filename, 'options.custom'],
                                   args= ARGUMENTS)

cppunit_options = SConsAddons.Options.CppUnit.CppUnit("cppunit", "1.9.10", required=0)
boost_options = SConsAddons.Options.Boost.Boost("boost","1.31.0",required=0)
opts.AddOption( cppunit_options )
opts.AddOption( boost_options )
opts.Add('prefix', 'Installation prefix', unspecified_prefix)
opts.Add('libdir', 'Library installation directory under <prefix>', default_libdir)
opts.Add('build_test', 'Build the test programs', 'yes')
opts.Add('StaticOnly', 'If not "no" then build only static library', 'no')
opts.Add('versioning', 'If no then build only libraries and headers without versioning', 'yes')
opts.Add('MakeDist', 'If true, make the distribution packages as part of the build', 'no')
opts.Add('arch', 'CPU architecture (ia32, x86_64, or ppc)',
         cpu_arch_default)
opts.Add('universal', 'Build universal binaries (Mac OS X only)',
         universal_default)
opts.Add('sdk', 'Platform SDK (Mac OS X only)', sdk_default)
if baseEnv.has_key("MSVS"):
   opts.Add('MSVS_VERSION', 'NOT WORKING: Set to specific version of MSVS to use. %s'%str(baseEnv['MSVS']['VERSIONS']), 
            baseEnv['MSVS']['VERSION'])
  
help_text = """--- CppDom Build system ---
Targets:
   install - Install this puppy
      ex: 'scons install prefix=$HOME/software' to install in your account
   Type 'scons' to just build it
 
"""

help_text = help_text + """Options:
   optimize=yes    Generate optimized code.
   profile=yes     Turn on generation of profiling code.
   
"""

help_text = help_text + opts.GenerateHelpText(baseEnv)
#help_text += "Options:\n" + opts.GenerateHelpText(baseEnv)
Help(help_text)

# --- MAIN BUILD STEPS ---- #
# If we are running the build
if not SConsAddons.Util.hasHelpFlag():
   opts.Update(baseEnv)                   # Update the options

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

   inst_paths = {}
   inst_paths['base'] = os.path.abspath(baseEnv['prefix'])
   inst_paths['lib'] = pj(inst_paths['base'], baseEnv['libdir'])
   inst_paths['bin'] = pj(inst_paths['base'], 'bin')
   if baseEnv['versioning'] == 'yes':
      INCLUDE_VERSION= "cppdom-%s.%s.%s" % CPPDOM_VERSION
      inst_paths['include'] = pj(inst_paths['base'], 'include', INCLUDE_VERSION)
   else:
      inst_paths['include'] = pj(inst_paths['base'], 'include')
      

   print "using prefix: ", inst_paths['base']   
   
   dirs = ['cppdom']
   if baseEnv['build_test'] == 'yes':
      dirs.append('test')

   if baseEnv['versioning'] == 'yes':
      CPPDOM_LIB_NAME="cppdom-%s_%s_%s" % CPPDOM_VERSION
   else:
      CPPDOM_LIB_NAME='cppdom'

   Export('baseEnv','inst_paths','cpu_arch', 'universal', 'sdk', 'opts',
          'cppunit_options', 'boost_options', 'CPPDOM_LIB_NAME')

   # Process subdirectories
   for d in dirs:
      SConscript(pj(d,'SConscript'), build_dir=pj(buildDir, d), duplicate=0)

   # Setup tar of source files
   tar_sources = Split("""
	 	  AUTHORS
                  ChangeLog
		  COPYING
		  README
		  cppdom-config.in
		  cppdom.pc.in
		  SConstruct
		  doc/cppdom.doxy
		  doc/dox/examples_index.dox
		  doc/dox/mainpage.dox
		  vc7/cppdom.sln
		  vc7/cppdom.vcproj
		  cppdom/
		  test/
   """)
   #baseEnv.Append(TARFLAGS = ['-z',])
   #baseEnv.Tar('cppdom-' + '%i.%i.%i' % CPPDOM_VERSION + '.tar.gz', tar_sources)

   # Build up substitution map
   submap = {
      '@prefix@'                    : inst_paths['base'],
      '@exec_prefix@'               : '${prefix}',
      '@cppdom_cxxflags@'           : '',
      '@includedir@'                : inst_paths['include'],
      '@cppdom_extra_cxxflags@'     : '',
      '@cppdom_extra_include_dirs@' : '',
      '@cppdom_libs@'               : "-l%s" % CPPDOM_LIB_NAME,
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

