#!python
import os
pj = os.path.join

Import('*')

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
cppdom_lib_env.Append(CPPPATH = [inst_paths['include'],],
                      CPPDEFINES=["CPPDOM_EXPORTS",])

# If should not do static only, then create static and shared libraries
if "shared" in combo["libtype"]:
   cppdom_lib = cppdom_lib_env.SharedLibrary(cppdom_shared_libname, sources)   
   cppdom_lib_env.Install(inst_paths['lib'], cppdom_lib)

if "static" in combo["libtype"]:
   cppdom_static_lib = cppdom_lib_env.StaticLibrary(cppdom_static_libname, sources)
   cppdom_lib_env.Install(inst_paths['lib'], cppdom_static_lib)

if variant_pass == 0:
   header_path = pj(inst_paths['include'],'cppdom')
   cppdom_lib_env.InstallAs(target = [pj(header_path, h) for h in headers], 
                            source = headers)
