"""
Tool-specific initialization for MinGW (http://www.mingw.org/)

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

Copied from http://www.scons.org/wiki/CrossCompilingMingw.
"""
import os
import os.path

import SCons.Action
import SCons.Builder
import SCons.Tool
import SCons.Util

def find(env):
    prefixes = ['i686-w64-mingw32-']
    try:
        if env['mingw_prefer_amd64']:
            prefixes.insert(0, 'x86_64-w64-mingw32-')
            print "Inserted amd64 prefix"
    except KeyError:
        pass
    print "prefixes list len {0}".format(len(prefixes))
        
    for prefix in prefixes:
        # First search in the SCons path and then the OS path:
        if env.WhereIs(prefix + 'gcc') or SCons.Util.WhereIs(prefix + 'gcc'):
            return prefix

    return ''

def shlib_generator(target, source, env, for_signature):
    cmd = SCons.Util.CLVar(['$SHLINK', '$SHLINKFLAGS'])

    dll = env.FindIxes(target, 'SHLIBPREFIX', 'SHLIBSUFFIX')
    if dll: cmd.extend(['-o', dll])

    cmd.extend(['$SOURCES', '$_LIBDIRFLAGS', '$_LIBFLAGS'])

    implib = env.FindIxes(target, 'LIBPREFIX', 'LIBSUFFIX')
    if implib:
        cmd.append('-Wl,--out-implib,'+implib.get_string(for_signature))

    def_target = env.FindIxes(target, 'WIN32DEFPREFIX', 'WIN32DEFSUFFIX')
    if def_target:
        cmd.append('-Wl,--output-def,'+def_target.get_string(for_signature))

    return [cmd]

def shlib_emitter(target, source, env):
    dll = env.FindIxes(target, 'SHLIBPREFIX', 'SHLIBSUFFIX')
    no_import_lib = env.get('no_import_lib', 0)

    if not dll:
        raise SCons.Errors.UserError, "A shared library should have exactly one target with the suffix: %s" % env.subst("$SHLIBSUFFIX")

    if not no_import_lib and \
       not env.FindIxes(target, 'LIBPREFIX', 'LIBSUFFIX'):

        # Append an import library to the list of targets.
        target.append(env.ReplaceIxes(dll,
                        'SHLIBPREFIX', 'SHLIBSUFFIX',
                        'LIBPREFIX', 'LIBSUFFIX'))

    # Append a def file target if there isn't already a def file target
    # or a def file source. There is no option to disable def file
    # target emitting, because I can't figure out why someone would ever
    # want to turn it off.
    def_source = env.FindIxes(source, 'WIN32DEFPREFIX', 'WIN32DEFSUFFIX')
    def_target = env.FindIxes(target, 'WIN32DEFPREFIX', 'WIN32DEFSUFFIX')
    if not def_source and not def_target:
        target.append(env.ReplaceIxes(dll,
                      'SHLIBPREFIX', 'SHLIBSUFFIX',
                      'WIN32DEFPREFIX', 'WIN32DEFSUFFIX'))

    return (target, source)


#shlib_action = SCons.Action.CommandGenerator(shlib_generator)
# in Scons 2.1 the line above needs to be replace with:
shlib_action = SCons.Action.Action(shlib_generator, generator=1)

res_action = SCons.Action.Action('$RCCOM', '$RCCOMSTR')

res_builder = SCons.Builder.Builder(action=res_action, suffix='.o',
                  source_scanner=SCons.Tool.SourceFileScanner)
SCons.Tool.SourceFileScanner.add_scanner('.rc', SCons.Defaults.CScan)

def generate(env):
    mingw_prefix = find(env)

    if mingw_prefix:
        dir = os.path.dirname(env.WhereIs(mingw_prefix + 'gcc') or SCons.Util.WhereIs(mingw_prefix + 'gcc'))

        # The mingw bin directory must be added to the path:
        path = env['ENV'].get('PATH', [])
        if not path:
            path = []
        if SCons.Util.is_String(path):
            path = path.split(os.pathsep)

        env['ENV']['PATH'] = os.pathsep.join([dir] + path)

    # Most of mingw is the same as gcc and friends...
    gnu_tools = ['gcc', 'g++', 'gnulink', 'ar', 'gas']
    for tool in gnu_tools:
        SCons.Tool.Tool(tool)(env)

    #... but a few things differ:
    env['CC'] = mingw_prefix + 'gcc'
    env['SHCCFLAGS'] = SCons.Util.CLVar('$CCFLAGS')
    env['CXX'] = mingw_prefix + 'g++'
    env['SHCXXFLAGS'] = SCons.Util.CLVar('$CXXFLAGS')
    env['SHLINKFLAGS'] = SCons.Util.CLVar('$LINKFLAGS -shared')
    env['SHLINKCOM']   = shlib_action
    env.Append(SHLIBEMITTER = [shlib_emitter])
    # This line isn't required and breaks C++ linking
    #env['LINK'] = mingw_prefix + 'g++'
    env['AS'] = mingw_prefix + 'as'
    env['AR'] = mingw_prefix + 'ar'
    env['RANLIB'] = mingw_prefix + 'ranlib'
    env['WIN32DEFPREFIX']    = ''
    env['WIN32DEFSUFFIX']    = '.def'
    env['SHOBJSUFFIX'] = '.o'
    env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

    env['RC'] = mingw_prefix + 'windres'
    env['RCFLAGS'] = SCons.Util.CLVar('')
    env['RCINCFLAGS'] = '$( ${_concat(RCINCPREFIX, CPPPATH, RCINCSUFFIX, __env__, RDirs, TARGET)} $)'
    env['RCINCPREFIX'] = '--include-dir '
    env['RCINCSUFFIX'] = ''
    env['RCCOM'] = '$RC $RCINCFLAGS $RCINCPREFIX $SOURCE.dir $RCFLAGS -i $SOURCE -o $TARGET'
    env['BUILDERS']['RES'] = res_builder

    # Some setting from the platform also have to be overridden:
    env['OBJPREFIX']    = ''
    env['OBJSUFFIX']    = '.o'
    env['LIBPREFIX']    = 'lib'
    env['LIBSUFFIX']    = '.a'
    env['SHOBJPREFIX']  = '$OBJPREFIX'
    env['SHOBJSUFFIX']  = '$OBJSUFFIX'
    env['PROGPREFIX']   = ''
    env['PROGSUFFIX']   = '.exe'
    env['LIBPREFIX']    = ''
    env['LIBSUFFIX']    = '.lib'
    env['SHLIBPREFIX']  = ''
    env['SHLIBSUFFIX']  = '.dll'
    env['LIBPREFIXES']  = [ '$LIBPREFIX' ]
    env['LIBSUFFIXES']  = [ '$LIBSUFFIX' ]

def exists(env):
    return find(env)