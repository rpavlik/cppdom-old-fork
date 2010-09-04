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

import os, sys, string, copy, re
import SCons.Environment
import SCons.Platform
import SCons
import Options
from Util import GetPlatform, GetArch
default_funcs = []

class EnvironmentBuilder(object):
   """ Builder class for scons environments.
       Used to build up an environment based on user settings.
       
       There are default settings for all levels.  These settings can be overriden
       through the class interface or through the supported options processing.
   """
   # Level options
   NONE = 0
   MINIMAL = 1
   STANDARD = 2
   EXTENSIVE = 3
   MAXIMUM = 4
   
   # Opt flags
   REDUCE_SIZE = 'reduce_size'
   FAST_MATH = 'fast_math'
   ARCH_SPEC = 'arch_specific'
   
   # Warning flags
   WARN_AS_ERROR = 'warn_as_error'
   WARN_STRICT   = 'warn_strict'
   
   # MSVC runtime
   MSVC_MT_DLL_RT     = "msvc_mt_dll_rt"
   MSVC_MT_DBG_DLL_RT = "msvc_mt_dbg_dll_rt"
   MSVC_MT_RT         = "msvc_mt_rt"
   MSVC_MT_DBG_RT     = "msvc_mt_dbg_rt"
   
   # CPU ARCH
   AUTODETECT_ARCH    = "autodetect_arch"
   IA32_ARCH          = "ia32"
   X64_ARCH           = "x64"
   IA64_ARCH          = "ia64"
   PPC_ARCH           = "ppc"
   PPC64_ARCH         = "ppc64"
   UNIVERSAL_ARCH     = "universal"

   def __init__(self):
      """ Initialize the class with defaults. """
      global default_funcs
      self.debugLevel   = EnvironmentBuilder.NONE 
      self.debugTags    = []
      self.optLevel     = EnvironmentBuilder.NONE
      self.optTags      = []
      self.warningLevel = EnvironmentBuilder.MINIMAL 
      self.warningTags  = []
      self.profEnabled  = False
      self.exceptionsEnabled = True
      self.structuredExceptionsEnabled = False
      self.rttiEnabled  = True            
      self.cpuArch      = None
      
      # Darwin specific
      self.darwinUniversalEnabled = False
      self.darwinSdk = ''
      
      # MSVC specific
      self.msvcRuntime  = None
      
      # List of [ [compilers], [platforms], func ]
      # If compiler or platform list is empty, then ignore that check
      self.funcList     = copy.copy(default_funcs)
      
      # Defaults:  These levels are applied if the user just enables with no level      
      self.defaultDebugLevel   = EnvironmentBuilder.STANDARD
      self.defaultOptLevel     = EnvironmentBuilder.STANDARD
      self.defaultWarningLevel = EnvironmentBuilder.STANDARD

   def clone(self):
      return copy.copy(self)

   def buildEnvironment(self, options=None, variant=None, **kw):
      """ Build an environment object and apply any options to it.
          Takes same parameters as Environment() in SCons.
          options - If passed and is instance of SCons.Options.Options, it will
                    be applied to environment before builder apply's it's options.
          variant - If passed, it will be added as entry to environment
                    and available when applying options.
      """
      if options and not isinstance(options, Options.Options):
         kw["options"] = options  
      new_env = apply(SCons.Environment.Environment, [], kw)  
      self.applyToEnvironment(new_env,variant, options)
      return new_env
   
   def applyToEnvironment(self, env, variant=None, options=None):
      """ Apply current builder options to an existing environment.
          Returns env argument. 
         
         Ex: new_env = bldr.applyToEnvironment(env.Clone())
      """
      if variant:
         env["variant"] = variant
      if options and isinstance(options, Options.Options):
         options.Apply(env)
      self._applyOptionsToEnvironment(env)
      return env

   def enableDebug(self, level=None, tags=[]):
      if not level:
         level = self.defaultDebugLevel
      self.debugLevel = level
      self.debugTags = tags
   def disableDebug(self):
      self.enableDebug(level=EnvironmentBuilder.NONE)
      
   def enableOpt(self, level=None, tags=[]):
      if not level:
         level = self.defaultOptLevel
      self.optLevel = level
      self.optTags = tags
   def disableOpt(self):
      self.enableOpt(EnvironmentBuilder.NONE)
      
   def enableProfiling(self, val=True):
      self.profEnabled = val
   def disableProfiling(self):
      self.enableProfiling(False)
   
   def enableWarnings(self, level=None, tags=[]):
      if level is None:
         level = self.defaultWarningLevel
      self.warningLevel = level
      self.warningTags = tags
   def disableWarnings(self):
      self.enableWarnings(EnvironmentBuilder.NONE)
   
   def enableExceptions(self, val=True):
      self.exceptionsEnabled = val
   def disableExceptions(self):
      self.enableExceptions(False)
      
   def enableStructuredExceptions(self, val=True):
      self.structuredExceptionsEnabled = val
   def disableStructuredExceptions(self):
      self.enableStructuredExceptions(False)
      
   def enableRTTI(self, val=True):
      self.rttiEnabled = val
   def disableRTTI(self):
      self.enableRTTI(False)
      
   def setCpuArch(self, val=AUTODETECT_ARCH):
      if val != EnvironmentBuilder.AUTODETECT_ARCH:
         self.cpuArch = val
      else:
         arch_map = {"ia32":EnvironmentBuilder.IA32_ARCH,
                     "x64":EnvironmentBuilder.X64_ARCH,
                     "ppc":EnvironmentBuilder.PPC_ARCH,
                     "ppc64":EnvironmentBuilder.PPC64_ARCH}
         self.cpuArch = arch_map.get(GetArch(), EnvironmentBuilder.AUTODETECT_ARCH)

   # ---- Darwin specific ----- #
   def darwin_enableUniversalBinaries(self, val=True):
      self.darwinUniversalEnabled = val
   def darwin_disableUniversalBinaries(self):
      self.darwin_enableUniversalBinaries(False)

   def darwin_setSdk(self, val):
      self.darwinSdk = val
      
   # ---- MSVC specific ---- #
   def setMsvcRuntime(self, val):
      self.msvcRuntime = val
   
   # ---- Command-line option processing ---- #
   def addOptions(self, opts):
      """ The EnvironmentBuilder has support for adding command line options to an
          option processing object.  This object has to be an instance
          of SConsAddons.Options.   Once the options are added, the user
          will be able to set defaults for the environment builder.
          
          TODO: Add options for tags.
      """
      import SConsAddons.Options as sca_opts
      
      assert isinstance(opts, sca_opts.Options)
      opts.AddOption(sca_opts.SeparatorOption("\nEnvironment Builder Defaults"))      
      opts.AddOption(sca_opts.EnumOption('default_debug_level',
                                         'Default debug level for environment builder.',
                                         'standard', 
                                         ['none','minimal','standard','extensive','maximum'],
                                         {'none':EnvironmentBuilder.NONE,
                                          'minimal':EnvironmentBuilder.MINIMAL,
                                          'standard':EnvironmentBuilder.STANDARD,
                                          'extensive':EnvironmentBuilder.EXTENSIVE,
                                          'maximum':EnvironmentBuilder.MAXIMUM}))
      opts.AddOption(sca_opts.EnumOption('default_opt_level',
                                         'Default optimization level for environment builder.',
                                         'standard', 
                                         ['none','minimal','standard','extensive','maximum'],
                                         {'none':EnvironmentBuilder.NONE,
                                          'minimal':EnvironmentBuilder.MINIMAL,
                                          'standard':EnvironmentBuilder.STANDARD,
                                          'extensive':EnvironmentBuilder.EXTENSIVE,
                                          'maximum':EnvironmentBuilder.MAXIMUM}))
      opts.AddOption(sca_opts.EnumOption('default_warning_level',
                                         'Default optimization level for environment builder.',
                                         'standard', [],
                                         {'none':EnvironmentBuilder.NONE,
                                          'minimal':EnvironmentBuilder.MINIMAL,
                                          'standard':EnvironmentBuilder.STANDARD,
                                          'extensive':EnvironmentBuilder.EXTENSIVE,
                                          'maximum':EnvironmentBuilder.MAXIMUM}))            
      if GetPlatform() == "darwin":
         opts.Add(sca_opts.BoolOption('darwin_universal',
                                      'Build universal binaries.', False))
         opts.Add('darwin_sdk', 'Darwin Platform SDK.', '')
   
   def readOptions(self, optEnv):
      """ Read the processed options from the given environment. """
      self.defaultDebugLevel   = optEnv["default_debug_level"]
      self.defaultOptLevel     = optEnv["default_opt_level"]
      self.defaultWarningLevel = optEnv["default_warning_level"]

      if GetPlatform() == "darwin":
         self.darwinUniversalEnabled = optEnv["darwin_universal"]
         self.darwinSdk = optEnv["darwin_sdk"]

   # ---- Option application ---- #
   def _applyOptionsToEnvironment(self, env):
      tools = env["TOOLS"]
      #print "Using tools: ", tools
      
      # Find the compilers/builders we are using
      c_compiler = env["CC"]
      cxx_compiler = env["CXX"]
      linker = env["LINK"]
      # one of: ['cygwin','irix','sunos','linux','freebsd','darwin','win32']
      platform = GetPlatform()

      # Special case for compiler callers like distcc
      # XXX: This is a bit of a hack, but it will work for now
      for x in ["distcc",]:
         if c_compiler.startswith(x):
            c_compiler = c_compiler.split()[-1]
         if cxx_compiler.startswith(x):
            cxx_compiler = cxx_compiler.split()[-1]
         if linker.startswith(x):
            linker = linker.split()[-1]
      
      # Based on compiler and platform
      for f in self.funcList:
         (compiler_list,platform_list, func) = f
         if (len(compiler_list)==0) or (c_compiler in compiler_list) or (cxx_compiler in compiler_list):
            if (len(platform_list)==0) or (platform in platform_list):
               func(self, env)


# ----------- Option appliers ------------ #
# ---- GCC ---- #
def gcc_optimizations(bldr, env):
   if EnvironmentBuilder.NONE == bldr.optLevel:
      return
   
   CCFLAGS = []
   CXXFLAGS = []
   CPPDEFINES = []

   if EnvironmentBuilder.REDUCE_SIZE in bldr.optTags:
      CCFLAGS.append('-Os')
   else:
      if bldr.optLevel == EnvironmentBuilder.MINIMAL:
         CCFLAGS.append('-O1')
      elif bldr.optLevel == EnvironmentBuilder.STANDARD:
         CCFLAGS.append('-O2')
      elif ((bldr.optLevel == EnvironmentBuilder.EXTENSIVE) or
            (bldr.optLevel == EnvironmentBuilder.MAXIMUM)):
         CCFLAGS.append('-O3')

   # Fast math
   if EnvironmentBuilder.FAST_MATH in bldr.optTags:
      CCFLAGS.append('-ffast-math')
   
   # TODO: Do architecture specific optimizations here
   env.Append(CXXFLAGS=CXXFLAGS, CCFLAGS=CCFLAGS, CPPDEFINES=CPPDEFINES)

def gcc_debug(bldr, env):
   #print "Calling gcc_debug."
   if EnvironmentBuilder.NONE == bldr.debugLevel:
      return
   env.Append(CCFLAGS = ["-g", "-fno-inline"],
              CXXFLAGS = ["-fno-implicit-inline-templates", "-fno-default-inline"])

def gcc_warnings(bldr, env):
   CCFLAGS = []
   
   if EnvironmentBuilder.NONE == bldr.warningLevel:
      CCFLAGS.append(['-w',])
   elif bldr.warningLevel == EnvironmentBuilder.MINIMAL:
      pass
   elif bldr.warningLevel == EnvironmentBuilder.STANDARD:
      CCFLAGS.append(['-Wall',])
   elif bldr.warningLevel == EnvironmentBuilder.EXTENSIVE:
      CCFLAGS.append(['-Wall','-Wextra'])
   elif bldr.warningLevel == EnvironmentBuilder.MAXIMUM:
      CCFLAGS.extend(['-Wall','-Wextra'])

   # warnings as errors
   if EnvironmentBuilder.WARN_AS_ERROR in bldr.debugTags:
      CCFLAGS.append(['-Werror',])
   
   if EnvironmentBuilder.WARN_STRICT in bldr.debugTags:
      CCFLAGS.append(['-pedantic',])
      
   env.Append(CCFLAGS=CCFLAGS)
   
def gcc_misc(bldr, env):
   if bldr.profEnabled:
      env.Append(CCFLAGS=["-pg",], LINKFLAGS=['-pg',])

def gcc_linux_misc(bldr, env):
   assert isinstance(bldr, EnvironmentBuilder)
   env.Append(CCFLAGS = ['-pipe',])    # Add pipe to speed up compiles on Linux

   if bldr.cpuArch:
      if bldr.cpuArch == EnvironmentBuilder.IA32_ARCH:
         env.Append(CCFLAGS = ['-m32'],
                    LINKFLAGS = ['-m32'])
      elif bldr.cpuArch == EnvironmentBuilder.X64_ARCH:
         env.Append(CCFLAGS = ['-m64'],
                    LINKFLAGS = ['-m64'])
      else:
         assert False, "Invalid arch used for Linux gcc."

def gcc_darwin_misc(bldr,env):
   assert isinstance(bldr, EnvironmentBuilder)

   # We use libtool(1) here instead of ar(1) to ensure that we can build
   # static universal binaries.
   env['AR'] = 'libtool'
   env['ARFLAGS'] = ['-static', '-o']
   env.Append(CCFLAGS = ['-pipe'])

   # XXX: This list should be hard coded. It should contain the architectures
   # that have been detected as being valid.
   universal_arch_list = ['ppc', 'i386', 'ppc64']

   print os.uname()
   if os.uname()[2].split('.')[0] >= '9':
      universal_arch_list.append('x64')

   if bldr.darwinUniversalEnabled:
      for a in universal_arch_list:
         env.Append(CCFLAGS = ['-arch', a], LINKFLAGS = ['-arch', a])
   else:
      if bldr.cpuArch != None:
         if bldr.cpuArch == EnvironmentBuilder.UNIVERSAL_ARCH:
            for a in universal_arch_list:
               env.Append(CCFLAGS = ['-arch', a], LINKFLAGS = ['-arch', a])
         elif bldr.cpuArch == EnvironmentBuilder.X64_ARCH:
            env.Append(CCFLAGS = ['-arch', 'x86_64'],
                       LINKFLAGS = ['-arch', 'x86_64'])
         elif bldr.cpuArch == EnvironmentBuilder.IA32_ARCH:
            env.Append(CCFLAGS = ['-arch', 'i386'],
                       LINKFLAGS = ['-arch', 'i386'])
         elif bldr.cpuArch == EnvironmentBuilder.PPC_ARCH:
            env.Append(CCFLAGS = ['-arch', 'ppc'],
                       LINKFLAGS = ['-arch', 'ppc'])
         elif bldr.cpuArch == EnvironmentBuilder.PPC64_ARCH:
            env.Append(CCFLAGS = ['-arch', 'ppc64'],
                       LINKFLAGS = ['-arch', 'ppc64'])
         else:
            assert False, "Invalid arch used for darwin gcc."

   if bldr.darwinSdk != '':
      env.Append(CCFLAGS = ['-isysroot', bldr.darwinSdk],
                 LINKFLAGS = ['-isysroot', bldr.darwinSdk])

      sdk_re = re.compile('MacOSX(10\..*?)u?\.sdk')
      match = sdk_re.search(bldr.darwinSdk)
      if match is not None:
         min_osx_ver = '-mmacosx-version-min=' + match.group(1)
         env.Append(CCFLAGS = [min_osx_ver], LINKFLAGS = [min_osx_ver])

# GCC functions
default_funcs.append([['gcc','g++'],[],gcc_optimizations])
default_funcs.append([['gcc','g++'],[],gcc_debug])
default_funcs.append([['gcc','g++'],[],gcc_warnings])
default_funcs.append([['gcc','g++'],[],gcc_misc])
default_funcs.append([['gcc','g++'],['linux'],gcc_linux_misc])
default_funcs.append([['gcc','g++'],['darwin'],gcc_darwin_misc])

# ---- Irix ---- #
# XXX: Irix support is very minimal at this time.  
#      I don't have access to an Irix box anymore and I don't compile
#      code for Irix very often.  This could be easily extended to support
#      many more features (archs, warnings, etc)
def irix_opt(bldr, env):
   if EnvironmentBuilder.NONE == bldr.optLevel:
      return
   
   CCFLAGS = []
   
   if bldr.optLevel == EnvironmentBuilder.MINIMAL:
      CCFLAGS.append('-O1')
   elif bldr.optLevel == EnvironmentBuilder.STANDARD:
      CCFLAGS.append('-O2')
   elif ((bldr.optLevel == EnvironmentBuilder.EXTENSIVE) or
         (bldr.optLevel == EnvironmentBuilder.MAXIMUM)):
      CCFLAGS.append('-O3')

   # TODO: Do architecture specific optimizations here
   env.Append(CXXFLAGS=CXXFLAGS, CCFLAGS=CCFLAGS, CPPDEFINES=CPPDEFINES)

def irix_debug(bldr, env):
   #print "Calling gcc_debug."
   if EnvironmentBuilder.NONE == bldr.debugLevel:
      return
   env.Append(CCFLAGS=["-g",])

def irix_misc(bldr, env):
   CCFLAGS = []
   env.Append(CXXFLAGS=["-mips3","-LANG:std","-n32"])

default_funcs.append([['cc',],['irix'],irix_opt])
default_funcs.append([['cc',],['irix'],irix_debug])
default_funcs.append([['cc',],['irix'],irix_misc])


# ---- MSVC ---- #      
def msvc_optimizations(bldr, env):
   if EnvironmentBuilder.NONE == bldr.optLevel:
      return
   
   CCFLAGS = []
   CXXFLAGS = []
   CPPDEFINES = []
   LINKFLAGS = ["/RELEASE",]

   if EnvironmentBuilder.REDUCE_SIZE in bldr.optTags:
      CCFLAGS.extend(['/O1'])
   else:
      if bldr.optLevel == EnvironmentBuilder.MINIMAL:
         CCFLAGS.extend(['/Ot','/Og'])
      elif bldr.optLevel == EnvironmentBuilder.STANDARD:
         CCFLAGS.append(['/O2'])
      elif ((bldr.optLevel == EnvironmentBuilder.EXTENSIVE) or
            (bldr.optLevel == EnvironmentBuilder.MAXIMUM)):
         CCFLAGS.append(['/Ox'])

   # Fast math
   if EnvironmentBuilder.FAST_MATH in bldr.optTags:
      CCFLAGS.append(['/fp:fast'])
   
   # TODO: Do architecture specific optimizations here
   # /arch:SSE/SEE2 /G1 /G2 
   # /favor
   env.Append(CXXFLAGS=CXXFLAGS, CCFLAGS=CCFLAGS, CPPDEFINES=CPPDEFINES, LINKFLAGS=LINKFLAGS)

def msvc_debug(bldr, env):
   """ TODO: Update to handle PDB debug database files. 
       TODO: Add support for run-time error checking.
   """
   #print "Calling msvc_debug."
   if EnvironmentBuilder.NONE == bldr.debugLevel:
      return
   env.Append(CCFLAGS=['/Od','/Ob0','/Z7'],
              LINKFLAGS=['/DEBUG'])

def msvc_warnings(bldr, env):
   CCFLAGS = []
   
   if EnvironmentBuilder.NONE == bldr.warningLevel:
      CCFLAGS.append(['/W0'])
   if bldr.warningLevel == EnvironmentBuilder.MINIMAL:
      CCFLAGS.append(['/W1'])
   elif bldr.warningLevel == EnvironmentBuilder.STANDARD:
      CCFLAGS.append(['/W2'])
   elif bldr.warningLevel == EnvironmentBuilder.EXTENSIVE:
      CCFLAGS.append(['/W3'])
   elif bldr.warningLevel == EnvironmentBuilder.MAXIMUM:
      CCFLAGS.append(['/Wall'])

   # warnings as errors
   if EnvironmentBuilder.WARN_AS_ERROR in bldr.debugTags:
      CCFLAGS.append(['/WX'])
   
   if EnvironmentBuilder.WARN_STRICT in bldr.debugTags:
      CCFLAGS.append(['/Za'])
      
   env.Append(CCFLAGS=CCFLAGS)
   
def msvc_misc(bldr, env):
   # Runtime library
   rt_map = { EnvironmentBuilder.MSVC_MT_DLL_RT:'/MD',
              EnvironmentBuilder.MSVC_MT_DBG_DLL_RT:'/MDd',
              EnvironmentBuilder.MSVC_MT_RT:'/MT',
              EnvironmentBuilder.MSVC_MT_DBG_RT:'/MTd'
            }   
   if rt_map.has_key(bldr.msvcRuntime):
      env.Append(CCFLAGS=[rt_map[bldr.msvcRuntime]])

   # Exception handling
   if bldr.exceptionsEnabled:
      if env["MSVS"]["VERSION"] >= "7.1":
         if bldr.structuredExceptionsEnabled:
            env.Append(CCFLAGS=['/EHa'])
         else:
            env.Append(CCFLAGS=['/EHsc'])
      else:
         env.Append(CCFLAGS=['/GX',])

   # RTTI
   if bldr.rttiEnabled:
      env.Append(CCFLAGS=["/GR"])
      
   # Default defines
   env.AppendUnique(CPPDEFINES = ["_WINDOWS"])

   if bldr.cpuArch == 'x64':
#      if float(msvs_version[:3]) < 8.0:
#         pass

      env.AppendUnique(CPPDEFINES = ['WIN64'])
      platform = 'amd64'

      env.AppendUnique(ARFLAGS = '/MACHINE:X64',
                       LINKFLAGS = '/MACHINE:X64')
   else:
      env.AppendUnique(CPPDEFINES = ['WIN32'])
      platform = 'x86'
      env.AppendUnique(ARFLAGS = '/MACHINE:X86',
                       LINKFLAGS = '/MACHINE:X86')

# MSVC functions
default_funcs.append([['cl'],[],msvc_optimizations])
default_funcs.append([['cl'],[],msvc_debug])
default_funcs.append([['cl'],[],msvc_warnings])
default_funcs.append([['cl'],[],msvc_misc])

# ---- DEFAULT ---- #
def default_debug_define(bldr,env):
   if EnvironmentBuilder.NONE != bldr.optLevel and EnvironmentBuilder.NONE == bldr.debugLevel:
      env.Append(CPPDEFINES=["NDEBUG",])

default_funcs.append([[],[],default_debug_define])


# ---- Helpers ---- #
def detectValidArchs():
   """ Helper method that uses environment builder and SCon Confs to detect valid
       arch targets for the current system.
       Returns list of valid archs with the default first.
   """
   def CheckArch(context, archName):
      """ Custom config context check for checking arch in this method. """
      context.Message( 'Checking for arch [%s] ...'%archName )
      ret = context.TryCompile("""int main() { return 0; }""",'.c')
      context.Result( ret )
      return ret
   
   valid_archs = []
   cur_arch = GetArch()
   if "ia32" == cur_arch:
      valid_archs.append(EnvironmentBuilder.IA32_ARCH)
   elif "x64" == cur_arch:
      valid_archs.append(EnvironmentBuilder.X64_ARCH)
   elif "ppc" == cur_arch:
      valid_archs.append(EnvironmentBuilder.PPC_ARCH)   
   elif "ppc64" == cur_arch:
      valid_archs.append(EnvironmentBuilder.PPC64_ARCH)      
   
   # Only handle case of using Visual C++ or GCC as the compiler for now.
   test_env = EnvironmentBuilder().buildEnvironment()
   # XXX: This is bad: GCC is not always called 'gcc'. It may also exist as
   # cc or as gcc-X.Y if multiple versions are installed.
   if test_env["CC"] != 'gcc' and test_env["CC"] != 'cl':
      return valid_archs

   # We are going to try to compile a program targetting potential valid architectures
   # if the build works, then we add that one to valid possible architectures
   arch_checks = []
   if GetPlatform() == "darwin":    # Treat Darwin specially
      arch_checks = [EnvironmentBuilder.PPC_ARCH,
                     EnvironmentBuilder.PPC64_ARCH,
                     EnvironmentBuilder.IA32_ARCH,
                     EnvironmentBuilder.UNIVERSAL_ARCH]
      if os.uname()[2][0] >= '9':
         arch_checks.insert(3, EnvironmentBuilder.X64_ARCH)
   elif cur_arch in ["ia32","x64"]: # Check x86 platforms
      arch_checks = [EnvironmentBuilder.IA32_ARCH,
                     EnvironmentBuilder.X64_ARCH]
   elif cur_arch in ["ppc","ppc64"]:   # Check PowerPC architectures
      arch_checks = [EnvironmentBuilder.PPC_ARCH,
                     EnvironmentBuilder.PPC64_ARCH]

   for test_arch in arch_checks:
      if test_arch not in valid_archs:
         env_bldr = EnvironmentBuilder()
         env_bldr.setCpuArch(test_arch)
         conf_env = env_bldr.buildEnvironment()
         conf_ctxt = conf_env.Configure(custom_tests={"CheckArch":CheckArch})
         passed_test = conf_ctxt.CheckArch(test_arch)
         conf_ctxt.Finish()
         if passed_test:
            valid_archs.append(test_arch)

   return valid_archs
