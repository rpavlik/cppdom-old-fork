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
""")

sources = Split("""
   cppdom.cpp
   xmlparser.cpp
   xmltokenizer.cpp
""")

env = baseEnv.Copy()

# Setup the library target
lib = env.SharedLibrary('cppdom', sources)

# Install headers
for h in headers:
   env.Install(pj(PREFIX, 'include', 'cppdom'), h)

# Install the library
env.Install(pj(PREFIX, 'lib'), lib)
