#!python
import os
pj = os.path.join

Import('*')
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
   ext/OptionRepository.h
""")

sources = Split("""
   cppdom.cpp
   xmlparser.cpp
   xmltokenizer.cpp
   version.cpp
   ext/OptionRepository.cpp
""")

# If boost is available then compile on the spirit addtion
if boost_options.isAvailable():
   sources.append("SpiritParser.cpp")

cppdom_lib_env = baseEnv.Copy()
cppdom_lib_env.Append(CPPPATH = [inst_paths['include'],])


# If should not do static only, then create static and shared libraries
if baseEnv['StaticOnly'] == "no":
   cppdom_lib = cppdom_lib_env.SharedLibrary(CPPDOM_LIB_NAME, sources)
   cppdom_lib_env.Install(inst_paths['lib'], cppdom_lib)

cppdom_static_lib = cppdom_lib_env.StaticLibrary(CPPDOM_LIB_NAME, sources)
cppdom_lib_env.Install(inst_paths['lib'], cppdom_static_lib)

header_path = pj(inst_paths['include'],'cppdom')
cppdom_lib_env.InstallAs(target = [pj(header_path, h) for h in headers], 
                         source = headers)
