#!python
import os
pj = os.path.join

Import('baseEnv PREFIX GetPlatform opts cppdom_pkg')
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

cppdom_lib_env = cppdom_pkg.getEnv().Copy()
cppdom_lib_env.Append(CPPPATH = ['#'])

#if GetPlatform() == 'irix':
#   env['SHCXXFLAGS'] = '${CXXFLAGS}'


# If should not do static only, then create static and shared libraries
if baseEnv['StaticOnly'] == "no":
   cppdom_lib = cppdom_pkg.createStaticAndSharedLibrary('cppdom', cppdom_lib_env)
else:
   cppdom_lib = cppdom_pkg.createStaticLibrary('cppdom', cppdom_lib_env)
   
cppdom_lib.addSources(sources)
cppdom_lib.addHeaders(headers, 'cppdom')
cppdom_lib.build()

