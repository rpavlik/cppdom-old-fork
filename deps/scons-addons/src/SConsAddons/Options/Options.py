"""SConsAddons.Options.Options

Defines options extension for supporting modular options.
"""

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
# Based on scons Options.py code
#
# Copyright Steven Knight
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

import types, string, os.path
import SCons.Errors
import SCons.Util
import textwrap
pj = os.path.join

import SConsAddons.Util as sca_util
GetArch = sca_util.GetArch

import SCons.SConf
Configure = SCons.SConf.SConf


# TODO: Port more standard SCons options over to this interface.
#

def bogus_func():
    pass

class OptionError(Exception):
    def __init__(self, option, msg):
        self.option = option
        self.message = msg        
        Exception.__init__(self,msg)


class Option(object):
    """
    Base class for all options.
    An option in our context is:
      - Something(s) that can be set and found
      - Once set(found) has the ability to set an environment with those settings      
    """
    def __init__(self, name, keys, help, dependencies = None):
        """
        Create an option.

        @type  name: string
        @param name: Name of the option.
        @type  keys: string or list of strings
        @param keys: The name (or names) of the command line option.
        @type  help: string
        @param help: Help text about the option object. If different help per key, put help in a
                     list.
        """
        self.name = name
        if not SCons.Util.is_List(keys):
           keys = [keys]        
        self.keys = keys
        self.help = help
        self.verbose = False      # If verbose, then output more information

    def startProcess(self):
        """ Called at beginning of processing.  Perform any intialization or notification here. """
        #print "Updating ", self.name
        pass
        
    def setInitial(self, optDict):
        """ Given an initial option Dictionary (probably from option file) initialize our data """
        pass
        
    def find(self, env):
        """ 
        Find the option setting (find default).  Called even if option already set.
        env - The current environment
        post: Should store internal rep of the value to use
        """
        pass
    
    def validate(self, env):
        """ Validate the option settings.  This checks to see if there are problems
            with the configured settings.
        """
        pass
    
    def completeProcess(self,env):
        """ Called when processing is complete. """
        pass
    
    def apply(self, env):
        """ Apply the results of the option to the given environment. """
        pass
    
    def getSettings(self):
        """ Return list sequence of ((key,value),(key,value),).
            dict.iteritems() should work.
            Used to save out settings for persistent options.
        """
        assert False, "Please implement a getSettings() method to return settings to save."
        return []
    
    def getValue(self, index=0):
        """ Return value associated with the key of the given index.
            This can be used with simple options to get a value for the option.
            Ex: options.GetOption("my_opt").getValue()
        """ 
        return self.getSettings()[index][1]

    def canProcess(self):
        """
        Indicates whether this option is ready for processing. Subclasses can override this
        method to influence the sorting behavior of Options.Process().
        """
        return True

class OptionProxy(object):
   """
   An option proxy is a simple "man-in-the-middle" type of object that can be used for customizing
   the behavior of another SConsAddons.Options.Option object. Simply define the method to be
   customized in a subclass of this class and do the custom work. Generally, this will probably
   involve calling through to the method in _target that is being proxied.

   This is dangerously close to subclassing, and the value of this class is highly debatable.
   Ultimately, what is difficult about subclassing a class such as StandardPackageOption is all
   the arguments to the constructor. Using this class, it can be much easier to simply make a
   StandardPackageOption object and then define whichever methods actually need to be
   customized. Perhaps what is really wrong with this class is that its implementation is too
   simple. That is, it does not add enough bells and whistles to make it sufficiently different
   from subclassing.
   """
   def __init__(self, target):
      """
      Constructor.

      @type  target: SConsAddons.Options.Option object
      @param target: The Option object being proxied by this object. All methods in this proxy
                     object will "intercept" calls to the target object.
      """
      self._target = target

   def __getattr__(self, key):
      # If this object (its subclass, actually) has the given key, then it will be used.
      if self.__dict__.has_key(key):
         return self.__dict__[key]
      # Otherwise, pass the attribute acquisition through to self._target.
      else:
         return getattr(self._target, key)

class LocalUpdateOption(Option):
    """ Extends the base option to have another method that allows updating of environment locally """
    def __init__(self, name, keys, help):
        Option.__init__(self,name,keys,help)
        
    def updateEnv(self, env, *pa, **kw):
        """ Deprecated. """
        print "DEPRECATED: method LocalUpdateOptions.updateEnv is deprecated. Please call 'apply' instead."
        self.apply(env, *pa, **kw)
    

class PackageOption(LocalUpdateOption):
    """ Base class for options that are used for specifying options for installed software packages. """
    def __init__(self, name, keys, help, dependencies = None):
        """
        Create an option.

        @type  name:         string
        @param name:         Name of the option.
        @type  keys:         string or list of strings
        @param keys:         The name (or names) of the command line option.
        @type  help:         string
        @param help:         Help text about the option object. If different help per key, put
                             help in a list.
        @type  dependencies: list of SConsAddons.Options.PackageOption objects
        @param dependencies: Other packages upon which this option depends. The availability of
                             the dependencies is a prerequisite for this option to be processed.
        """
        LocalUpdateOption.__init__(self, name, keys, help)
        self.available = False
        if not hasattr(self,"required"):
           self.required = False

        if dependencies is None:
           dependencies = []

        self.dependencies = dependencies

    def isAvailable(self):
        " Return true if the package is available "
        return self.available
    
    def setAvailable(self, val):
        self.available = val

    def checkRequired(self, msg):
        """ Called when there is config problem.  If required, then exit with error message """
        print msg
        if self.required:
            raise OptionError(self,"Check required failed: %s"%msg)

    def depsSatisfied(self):
        """
        Indicates whether the dependencies of this package option are available.

        @rtype: boolean
        @return: True is returned if all dependencies are available. False is returned if any
                 one dependency is not.
        """
        for dependency in self.dependencies:
           if not dependency.isAvailable():
              return False

        return True

    def canProcess(self):
        """
        Indicates that this option is ready for processing by Options.Process() if and only if
        the dependencies are satisfied.
        """
        return self.depsSatisfied()

    def _applyDependencies(self, env):
        """
        Applies the dependencies of this package option to the given environment object. This
        is intended for internal use by this class and its subclasses.
        """
        for dependency in self.dependencies:
            if dependency.isAvailable():
                dependency.apply(env)

                # Apply the dependencies of our dependency.
                dependency._applyDependencies(env)

class StandardPackageOption(PackageOption):
    """
    Simple package option that is meant for library and header checking with very little
    customization.  Just uses Configure.CheckXXX methods behind the scenes for verification.
    """
    def __init__(self, name, help, header = None, library = None, symbol = "main",
                 required = False, dependencies = None, linkerFlags = None):
        """
        @type  name:         string
        @param name:         Name of the option.
        @type  help:         string
        @param help:         Help text about the option object. If different help per key, put
                             help in a list.
        @type  header:       string
        @param header:       A header in the package to use for validating that the package
                             described by this option is both available and valid.
        @type  library:      string or list of strings
        @param library:      The library or libraries to link against. This can be the name of the
                             library or a list of libraries if multiple are needed.
        @type  sybmol:       string
        @param symbol:       A symbol to test for in the library when performing the validation
                             step.
        @type  required:     boolean
        @param required:     A flag indicating whether this package is a requirement for being
                             able to build the code that depends on the package.
        @type  dependencies: list of SConsAddons.Options.PackageOption objects
        @param dependencies: Other packages upon which this option depends. The availability of
                             the dependencies is a prerequisite for this option to be processed.
        @type  linkerFlags:  list of strings
        @param linkerFlags:  A list of additional flags to pass to the linker. This will be added
                             to the test context LINKFLAGS option.
        """
        self.baseKey = name
        self.incDirKey = name + "_incdir"
        self.libDirKey = name + "_libdir"

        PackageOption.__init__(self, name, [self.baseKey, self.incDirKey, self.libDirKey], help,
                               dependencies)

        self.available = False        
        self.baseDir = None 
        self.incDir = None
        self.libDir = None
        self.header = header
        self.library = library
        self.symbol = symbol
        self.required = required

        if linkerFlags is None:
            linkerFlags = []

        self.linkerFlags = linkerFlags

    def startProcess(self):
        print "Checking for:", self.name
    
    def setInitial(self, optDict):
        " Set initial values from given dict "
        if self.verbose:
            print "   Setting initial %s settings."%self.name
        if optDict.has_key(self.baseKey):
            self.baseDir = optDict[self.baseKey]
            if self.verbose:
                print "   %s specified or cached. [%s]."% (self.baseKey, self.baseDir)
        if optDict.has_key(self.incDirKey):
            inc_dir = optDict[self.incDirKey]
            if SCons.Util.is_List(inc_dir):
               self.incDir = inc_dir
            else:
               self.incDir = inc_dir.split(',')
            if self.verbose:
                print "   %s specified or cached. [%s]."% (self.incDirKey, self.incDir)
        if optDict.has_key(self.libDirKey):
            lib_dir = optDict[self.libDirKey]
            if SCons.Util.is_List(lib_dir):
               self.libDir = lib_dir
            else:
               self.libDir = lib_dir.split(',')
            if self.verbose:
                print "   %s specified or cached. [%s]."% (self.libDirKey, self.libDir)
    
    def find(self,env):
        # Don't try to find it for now.  Just use configure tests
        if self.baseDir:
            # Only fall back on the base_dir/include if a specific
            # include dir was not given.
            if self.incDir is None and os.path.exists(pj(self.baseDir,'include')):
                self.incDir = [pj(self.baseDir, 'include')]
            if self.libDir is None:
                arch_type = GetArch()
                if (arch_type == 'x64') or (arch_type == 'ia64'):
                   if os.path.exists(pj(self.baseDir,'lib64')):
                      self.libDir = [pj(self.baseDir, 'lib64')]
                
                if self.libDir is None and os.path.exists(pj(self.baseDir,'lib')):
                      self.libDir = [pj(self.baseDir, 'lib')]
 
    def validate(self, env):
        passed = True
    
        conf_env = env.Clone()
        self._applyDependencies(conf_env)

        # We do not want self.library to be added to conf_env. We let the checker use function
        # below deal with that.
        library = self.library
        self.library = None
        self.apply(conf_env)
        self.library = library

        conf_ctx = Configure(conf_env)
        if self.library and self.header:
            result = self._checkLibraryWithHeader(conf_ctx, self.library, self.header, "C++")
        elif self.library:
            result = self._checkLibrary(conf_ctx, self.library, self.symbol, self.header, "C++")
        elif self.header:
            result = conf_ctx.CheckCXXHeader(self.header)
        elif self.baseDir is not None:
            result = os.path.exists(self.baseDir)
        else:
            result = False

        if not result:
            passed = False
            self.checkRequired("Validation failed for option: %s"%self.name)
        conf_ctx.Finish()

        if not passed:
            self.baseDir = None
            self.incDir = None
            self.libDir = None
        else:
            self.available = True

    def _checkLibraryWithHeader(self, context, library, header, language):
        return context.CheckLibWithHeader(library, header, language)

    def _checkLibrary(self, context, library, symbol, header, langauge):
        return context.CheckLib(library, symbol, header, langauge)

    def apply(self, env):
        if self.incDir:
            if not SCons.Util.is_List(self.incDir):
                self.incDir = [self.incDir]
            if self.verbose:
                print "Appending inc_dir:", self.incDir
            env.Append(CPPPATH = self.incDir)

        if self.libDir:
            if not SCons.Util.is_List(self.libDir):
                self.libDir = [self.libDir]
            if self.verbose:
                print "Appending lib_dir:", self.libDir
            env.Append(LIBPATH = self.libDir)

        if self.library:
            if self.verbose:
                print "Adding lib:", self.library
            env.Append(LIBS = [self.library])

        if self.linkerFlags:
            if self.verbose:
                print "Adding linker flags:", self.linkerFlags
            env.Append(LINKFLAGS = self.linkerFlags)

    def getSettings(self):
        """ Return list sequence of ((key,value),(key,value),).
            dict.iteritems() should work.
            Used to save out settings for persistent options.
        """
        return [(self.baseKey,self.baseDir),
                (self.incDirKey, self.incDir),
                (self.libDirKey, self.libDir)]

class MultiNamePackageOption(StandardPackageOption):
   """
   An extension of StandardPackageOption that allows calling code to pass in a list of possible
   library names for a package. Using this capability, user-level code can indicate an order of
   preference for library names. This helps a lot on Windows where a package may have one name for
   its static library and another for its dynamic.
   """

   def __init__(self, name, help, header = None, library = None, symbol = "main", required = False,
                dependencies = None, linkerFlags = None):
      """
      @type  name:         string
      @param name:         Name of the option.
      @type  help:         string
      @param help:         Help text about the option object. If different help per key, put help
                           in a list.
      @type  header:       string
      @param header:       A header in the package to use for validating that the package
                           described by this option is both available and valid.
      @type  library:      string or list of strings
      @param library:      The library to link against. This can be the name of the library or a
                           list of library names if the library may have differnet names depending
                           on the platform or the type (dynamic versus static). If multiple
                           libraries are involved, then they must be grouped using a nested list.
                           For example, [["libname1", "libname2"], ["altname1", "altname2"]].
      @type  sybmol:       string
      @param symbol:       A symbol to test for in the library when performing the validation
                           step.
      @type  required:     boolean
      @param required:     A flag indicating whether this package is a requirement for being able
                           to build the code that depends on the package.
      @type  dependencies: list of SConsAddons.Options.PackageOption objects
      @param dependencies: Other packages upon which this option depends. The availability of the
                           dependencies is a prerequisite for this option to be processed.
      @type  linkerFlags:  list of strings
      @param linkerFlags:  A list of additional flags to pass to the linker. This will be added to
                           the test context LINKFLAGS option.
      """
      StandardPackageOption.__init__(self, name, help, header, library, symbol, required,
                                     dependencies, linkerFlags)

   def _checkLibraryWithHeader(self, context, library, header, language):
      if type(library) is list:
         for lib in library:
            result = context.CheckLibWithHeader(library, header, language)
            if result:
               self.library = lib
               break
      else:
         result = context.CheckLibWithHeader(library, header, language)

      return result

   def _checkLibrary(self, context, library, symbol, header, language):
      if type(library) is list:
         for lib in library:
            result = context.CheckLib(lib, symbol, header, language)
            if result:
               self.library = lib
               break
      else:
         result = context.CheckLib(library, symbol, header, language)

      return result

class SimpleOption(Option):
    """
    Implementation of a simple option wrapper.  This is used by Options.Add()
    This Option works for a single option value with a single key (stored internally)
    """
    def __init__(self, name, keys, help, finder, converter, setter, validator):
        """
        Create an option
        name - Name of the option
        key - the name of the commandline option
        help - Help text about the option object
        finder - Function for searching for value or default value.
                 If method, called with (key, environment)
        converter - option function that is called to convert the options's value before
                    putting in the environment
        setter - Method called to set/generate the environment
        validator - Function called to validate the option value
                    called with (key, value, environment)
        """
        Option.__init__(self, name, keys, help)
        if None == finder:
            self.finder_cb = None
        elif type(bogus_func) == type(finder):
            self.finder_cb = finder
        else:
            self.finder_cb = lambda key, env: finder
        self.converter_cb = converter
        self.setter_cb = setter
        self.validator_cb = validator
        self.value = None
        
    def setInitial(self, optDict):
        if optDict.has_key(self.keys[0]):
            self.value = optDict[self.keys[0]]

    def find(self, env):
        if None == self.value:     # If not already set by setInitial()
            if self.finder_cb:
                self.value = self.finder_cb(self.keys[0], env)
            else:
                self.value = None    
        
    def validate(self, env):
        """ Validate and convert the option settings """
        # -- convert the values -- #
        if self.converter_cb and self.value:
            try:
                self.value = self.converter_cb(self.value)
            except ValueError, x:
                raise SCons.Errors.UserError, 'Error converting option: %s value:%s\n%s'%(self.keys[0], self.value, x)
            
        # -- validate --- #
        if self.validator_cb and self.value:
            self.validator_cb(self.keys[0], self.value, env)
    
    def apply(self, env):
        env[self.keys[0]] = self.value
    
    def getSettings(self):
        return [(self.keys[0], self.value)]


class BoolOption(Option):
    """
    Implementation of a bool option wrapper. 
    This option wraps a single 'truth' value expressed as a string.
    """
    true_strings  = ('y', 'yes', 'true', 't', '1', 'on' , 'all' )
    false_strings = ('n', 'no', 'false', 'f', '0', 'off', 'none') 
   
    def __init__(self, key, help, default):
        """
        Create a bool option
        key - the name of the commandline option
        help - Help text about the option object
        default - Default truth value
        """
        Option.__init__(self, key, key, "%s (yes|no)" % help)
        self.value = None
        self.default = default
    
    def textToBool(self, val):
        """
        Converts strings to True/False depending on the 'truth' expressed by
        the string. If the string can't be converted, the original value
        will be returned.
        """
        if isinstance(val, types.BooleanType):
            return val
        
        lval = string.lower(val)
        if lval in BoolOption.true_strings: return True
        if lval in BoolOption.false_strings: return False
        raise ValueError("Invalid value for boolean option: %s" % val)

    def setInitial(self, optDict):
        if optDict.has_key(self.keys[0]):
            self.value = optDict[self.keys[0]]

    def find(self, env):
        if None == self.value:     # If not already set by setInitial()
            self.value = self.default
    
    def validate(self, env):
        """ Validate and convert the option settings """
        if self.value:
            try:
                self.value = self.textToBool(self.value)
            except ValueError, x:
                raise SCons.Errors.UserError, 'Error converting option: %s value:%s\n%s'%(self.keys[0], self.value, x)
    
    def apply(self,env):
        env[self.keys[0]] = self.value
    
    def getSettings(self):
        return [(self.keys[0], self.value)]


class EnumOption(Option):
    """
    Implementation of a enumerated option.
    Based off EnumOption from SCons.
    """    
    def __init__(self, key, help, default, allowed_values=[], map={}):
        """
        Create a enumerated option
        key - the name of the commandline option
        help - Help text about the option object
        default - The default value to use.
        allowed_values - List of values allowed.
        map - Dictionary to map string value passed to real value to use.
        note: If allowed_values is empty the value values are pulled from map.
        """
        if len(allowed_values) == 0 and len(map) == 0:
            raise SCons.Errors.UserError, 'EnumOption: Must have one entry in either allowed_values or map'
        self.allowed_values = allowed_values
        self.map = map
        if len(self.allowed_values) == 0:
            self.allowed_values = self.map.keys()
            
        help = '%s (%s)' % (help, '|'.join(self.allowed_values))
        Option.__init__(self, key, key, help)
        self.value = None
        self.default = default                

    def setInitial(self, optDict):
        if optDict.has_key(self.keys[0]):
            self.value = optDict[self.keys[0]]

    def find(self, env):
        if None == self.value:     # If not already set by setInitial()            
            self.value = self.default
    
    def validate(self, env):
        """ Validate and convert the option settings """
        if self.value:
            if self.value not in self.allowed_values:
                raise SCons.Errors.UserError, 'Invalid value [%s] value use for option [%s]'%(self.value,self.keys[0])
    
    def apply(self,env):        
        env[self.keys[0]] = self.map.get(self.value, self.value)
    
    def getSettings(self):
        return [(self.keys[0], self.value)]


class ListOption(Option):
    """
    Implementation of a list option wrapper.
    Based off ListOption from SCons.
    """    
    def __init__(self, key, help, elems, defaultElems):
        """
        Create a bool option
        key - the name of the commandline option
        help - Help text about the option object
        elems - List of allowable items.
        default - List of default elements.
        """
        Option.__init__(self, key, key, "%s -- ('all'|%s)"%(help,elems))
        self.value = None
        self.default = defaultElems[:]    
        self.allowedElems = elems[:]

    def setInitial(self, optDict):
        if optDict.has_key(self.keys[0]):
            self.value = optDict[self.keys[0]]

    def find(self, env):
        if None == self.value:     # If not already set by setInitial()
            self.value = self.default
    
    def validate(self, env):
        """ Validate and convert the option settings """
        if self.value and not isinstance(self.value,types.ListType):
            if 'all' == self.value:
                self.value = self.allowedElems
            elif isinstance(self.value, types.StringType):
                elems = self.value.split(',')
                for i in elems:
                    if not i in self.allowedElems:
                        raise SCons.Errors.UserError, "List option '%s' does not allow item '%s'"%(self.keys[0],i)
                self.value = elems
            else:
                raise SCons.Errors.UserError, 'Error converting option: %s value:%s\n%s\nMust be list or string.'%(self.keys[0], self.value, x)                
    
    def apply(self,env):
        env[self.keys[0]] = self.value
    
    def getSettings(self):
        if self.value == self.allowedElems:
            ret_val = 'all'
        else:
            ret_val = self.value
        return [(self.keys[0], ret_val)]


class SeparatorOption(Option):
    """
    Helper option to all a 'separator' to be added to the debug output.
    """
    def __init__(self, helpText):
        """
        Create an option
        name - Name of the option
        keys - the name (or names) of the commandline option
        help - Help text about the option object. If different help per key, put help in a list.
        """
        Option.__init__(self, "separator", "separator", helpText)
        
    def startProcess(self):
        pass
        
    def setInitial(self, optDict):
        """ Given an initial option Dictionary (probably from option file) initialize our data """
        pass
        
    def find(self, env):
        pass
    
    def validate(self, env):
        pass
    
    def completeProcess(self,env):
        pass
    
    def apply(self, env):
        pass
    
    def getSettings(self):
        return []


class Options:
    """
    Holds all the options, updates the environment with the variables,
    and renders the help text.
    """
    def __init__(self, files=None, args={}):
        """
        files - [optional] List of option configuration files to load
            (backward compatibility) If a single string is passed it is 
                                     automatically placed in a file list
        """

        self.unique_id = 0          # Id used to create unique names
        self.options = []           # List of option objects managed
        self.files = []             # Options files to load
        self.args = args            # Set the default args from command line
        self.verbose = False        # If true, then we will set all contained options to verbose before processing
        
        if SCons.Util.is_String(files):
           self.files = [files]
        elif (files != None):
           self.files = files

        try:
           # SCons 0.98+ name.
           self._isValidConstructionVar = SCons.Environment.is_valid_construction_var
        except AttributeError:
           # Pre-0.98 name.
           self._isValidConstructionVar = SCons.Util.is_valid_construction_var
        except:
           # XXX: This is not really the right way to do this.
           self._isValidConstructionVar = lambda v: True

    def Add(self, key, help="", default=None, validator=None, converter=None, name=None):
        """
        Add an option.

        Backwards compatible with standard scons options.
        Note: This means you can add any option that you can add to standard scons options
              include the standard scons option helpers.
        
        key - the name of the variable
        help - optional help text for the options
        default - optional default value
        validater - optional function that is called to validate the option's value
                    Called with (key, value, environment)
        converter - optional function that is called to convert the option's value before
                    putting it in the environment.
        """
        # If meant to call AddOption, call it for them
        if isinstance(key, Option):            
            self.AddOption(key)
            return

        if not self._isValidConstructionVar(key):
            raise SCons.Errors.UserError, "Illegal Options.Add() key `%s'" % key

        if None == name:            
            name = key
            
        option = SimpleOption(name, key, help, default, converter, None, validator)

        self.options.append(option)
        
    def AddOption(self, option):
        """ Add a single option object"""
        for k in option.keys:
            if not self._isValidConstructionVar(k):
                raise SCons.Errors.UserError, "Illegal construction var: Options.AddOption(): opt: '%s' -- key `%s'" % (option.name, k)

        self.options.append(option)
     
        
    def GetOption(self, name):
        """ Return the named option or None if not found.
            See also: Option.getValue()
        """
        for option in self.options:
            if name == option.name:
                return option
        return None

    def Update(self, env, args=None):
        """ Deprecated method of calling Process. """
        self.Process(env,args,applySimple=True)
        
    def Process(self, env, args=None, applySimple=True):
        """
        Process all options within the given environment.
        This will go through all options and will initialize, find, and validate
        them against this environment.
        If applySimple is set true, it will finish off by applying all simple options.
        Update an environment with the option variables.

        env - the environment to update.
        args - the dictionary to get the command line arguments from.
        """

        values = {}
        
        # Setup verbosity
        if self.verbose:
            for option in self.options:
                option.verbose = True

        # first load previous values from file
        for filename in self.files:
           if os.path.exists(filename):
              execfile(filename, values)
        
        # Next over-ride those with command line args
        if args is None:
            args = self.args
        values.update(args)

        pending      = self.options
        last_pending = []

        # Until the pending list is determined to be in a "stale" state (one iteration when no
        # pending Option objects are processed), loop over the pending list and process all
        # Option objects that are available to be processed.
        while pending != last_pending:
            new_pending = []
            for option in pending:
                if option.canProcess():
                    option.startProcess()         # Start processing
                    option.setInitial(values)     # Set initial values
                    option.find(env)              # Find values if needed
                    option.validate(env)          # Validate the settings
                    option.completeProcess(env)   # Signal processing completed
                else:
                    new_pending.append(option)

            last_pending = pending
            pending = new_pending

        # Apply options if requested
        if True == applySimple:
            self.Apply(env, allowedTypes=(SimpleOption,BoolOption,ListOption,EnumOption))
    

    def Apply(self, env, all=False, allowedTypes=(), allowedNames=()):
        """ Apply options from this option group to the given environment.
            all - If set true, then applies all options.
            allowedTypes - If set, it is a list of option types that will be applied.
            allowedNames - If set, it is a list of option names that will be applied.
            note: if both are set, then either must be true.
        """
        #print "Options.Apply: %s %s %s"%(all, allowedTypes, allowedNames)
        for option in self.options:
            #print "Checking: ", option.name
            if (True == all) or (isinstance(option, allowedTypes)) or (option.name in allowedNames):
                #print "    Passed, applying."
                option.apply(env)

    def Save(self, filename, env):
        """
        Saves all the options in the given file.  This file can
        then be used to load the options next run.  This can be used
        to create an option cache file.

        filename - Name of the file to save into
        env - the environment get the option values from
        """

        # Create the file and write out the header
        try:
            fh = open(filename, 'w')

            try:
                # Make an assignment in the file for each option within the environment
                # that was assigned a value other than the default.
                # For each option and each key
                key_value_list = []
                for o in self.options:
                    key_value_list.extend(o.getSettings())                    
                for (key,value) in key_value_list:
                    if None != value:
                        try:
                            eval(repr(value))
                        except:
                            # Convert stuff that has a repr that cannon be evaled to string
                            value = SCons.Util.to_string(value)                        
                        fh.write('%s = %s\n' % (key, repr(value)))
            finally:
                fh.close()

        except IOError, x:
            raise SCons.Errors.UserError, 'Error writing options to file: %s\n%s' % (filename, x)

    def GenerateHelpText(self, env, sort=None, width=0):
        """
        Generate the help text for the options.

        env   - an environment that is used to get the current values of the options.
        sort  - A sort method to use
        width - max line width
        """
        # Find width
        if 0 == width:
            try:
                import curses
                curses.setupterm()
                width = curses.tigetnum('cols')
            except:
                width = 80
        
        help_text = ""
        if sort:
            options = self.options[:]
            options.sort(lambda x,y,func=sort: func(x.key[0],y.key[0]))
        else:
            options = self.options

        key_list = []
        for o in options:
            key_list.extend(o.keys)                
        max_key_len = max([len(k) for k in key_list])             # Get max key for spacing
        leading_indent = ' '*2
        key_spacing = leading_indent + " "*(max_key_len+2)

        wrapper = textwrap.TextWrapper(width=width,
                                       initial_indent=leading_indent,
                                       subsequent_indent=key_spacing)
        
        for option in options:
            if isinstance(option, SeparatorOption):
                help_text = help_text + option.help + "\n"
            else:
                for ki in range(len(option.keys)):
                    k = option.keys[ki]
                    k_help = option.help
                    if SCons.Util.is_List(option.help):
                        k_help = option.help[ki]
                    option_help = '%-*s %s\n' % (max_key_len, k+':', k_help)
                    help_text = help_text + wrapper.fill(option_help) + "\n"
    
                    if env.has_key(k):
                        value = env[k]
                        help_text += key_spacing
                        if isinstance(value,types.ListType):
                            help_text += '%s\n'%value
                        elif isinstance(option, EnumOption):
                            enum_text = str(value)
                            for (k,v) in option.map.iteritems():
                               if value == v:
                                  enum_text = k
                            help_text += '[%s]\n'%enum_text
                        else:
                            help_text += '[%s]\n'%value
                    

        return help_text

