import os
pj = os.path.join

Import('baseEnv PREFIX')

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

# Setup the library target
lib = env.SharedLibrary('cppdom', sources)

# Install headers
for h in headers:
   env.Install(pj(PREFIX, 'include', 'cppdom'), h)

# Install the library
env.Install(pj(PREFIX, 'lib'), lib)
