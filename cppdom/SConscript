import os
pj = os.path.join

Import('pkg baseEnv PREFIX GetPlatform')

headers = Split("""
   config.h
   cppdom.h
   predicates.h
   shared_ptr.h
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

env = baseEnv.Copy()
env.Append(CPPPATH = ['#'])

if GetPlatform() == 'irix':
   env['SHCXXFLAGS'] = '${CXXFLAGS}'

#shlib = pkg.createSharedLibrary('cppdom', env)
#shlib.addSources(sources)
#shlib.addHeaders(headers, 'cppdom')

lib = pkg.createStaticLibrary('cppdom', env)
lib.addSources(sources)
lib.addHeaders(headers, 'cppdom')
