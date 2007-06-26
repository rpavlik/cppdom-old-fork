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
   ext/OptionRepository.cpp
""")

# If boost is available then compile on the spirit addtion
if boost_options.isAvailable():
   sources.append("SpiritParser.cpp")

cppdom_lib_env = build_env.Copy()
cppdom_lib_env.Append(CPPPATH = [inst_paths['include'],])

if "debug" in combo["type"] or "hybrid" in combo["type"]:
   cppdom_lib_env.AppendUnique(CPPDEFINES = ["CPPDOM_DEBUG"])

# If should not do static only, then create static and shared libraries
if "shared" in combo["libtype"]:
   shlinkcom = cppdom_lib_env['SHLINKCOM']
   # When using Visual C++ 8.0 or newer, embed the manifest in the DLL.
   if cppdom_lib_env.has_key('MSVS_VERSION') and float(cppdom_lib_env['MSVS_VERSION']) >= 8.0:
      shlinkcom = [shlinkcom,
                   'mt.exe -manifest ${TARGET}.manifest -outputresource:$TARGET;2']
   cppdom_shared_lib_env = cppdom_lib_env.Copy()
   cppdom_shared_lib_env.AppendUnique(CPPDEFINES = ["CPPDOM_EXPORTS"])
   cppdom_lib = cppdom_shared_lib_env.SharedLibrary(cppdom_shared_libname, sources,
                                                    SHLINKCOM = shlinkcom)
   cppdom_shared_lib_env.Install(inst_paths['lib'], cppdom_lib)

if "static" in combo["libtype"]:
   cppdom_static_lib_env = cppdom_lib_env.Copy()
   cppdom_static_lib = cppdom_static_lib_env.StaticLibrary(cppdom_static_libname, sources)
   cppdom_static_lib_env.Install(inst_paths['lib'], cppdom_static_lib)

if variant_pass == 0:
   header_path = pj(inst_paths['include'],'cppdom')
   cppdom_lib_env.InstallAs(target = [pj(header_path, h) for h in headers], 
                            source = headers)
