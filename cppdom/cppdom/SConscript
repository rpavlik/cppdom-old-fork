#!python
import os
pj = os.path.join

Import('baseEnv PREFIX GetPlatform opts')
boost_options = opts.GetOption('boost')

headers = Split("""
   config.h
   cppdom.h
   predicates.h
   shared_ptr.h
   SpiritParser.h
   xmlparser.h
   xmltokenizer.h
   version.h
""")

sources = Split("""
   cppdom.cpp
   xmlparser.cpp
   xmltokenizer.cpp
   version.cpp
""")

# If boost is available then compile on the spirit addtion
if boost_options.isAvailable():
   sources.append("SpiritParser.cpp")

env = baseEnv.Copy()
env.Append(CPPPATH = ['#'])

if GetPlatform() == 'irix':
   env['SHCXXFLAGS'] = '${CXXFLAGS}'
if GetPlatform == 'win32':
   try:
      env.MSVSProject(target = 'cppdom' + env['MSVSPROJECTSUFFIX'],
						 srcs = sources,
						 incs = headers,
						 buildtarget = dll,
						 variant = 'Release')
      env.MSVSProject(target = 'cppdom_debug' + env['MSVSPROJECTSUFFIX'],
						 srcs = sources,
						 incs = headers,
						 buildtarget = dll,
						 variant = 'Debug')
   except:
      print '[WRN] Unable to make MSVS Project Files.'

if env['StaticOnly'] == "no":
   shlib = env.SharedLibrary('cppdom', source = sources)
   env.Install(pj(PREFIX, 'lib'), shlib)

lib = env.StaticLibrary('cppdom', source = sources)
env.Install(pj(PREFIX, 'lib'), lib)
for h in headers:
   env.Install(pj(PREFIX, 'include', 'cppdom'), h)
