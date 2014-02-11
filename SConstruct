import os
import sys
import platform
import SCons

#============================ banner ==========================================

banner  = []
banner += [""]
banner += [" ___                 _ _ _  ___  _ _ "]
banner += ["| . | ___  ___ ._ _ | | | |/ __>| \ |"]
banner += ["| | || . \/ ._>| ' || | | |\__ \|   |"]
banner += ["`___'|  _/\___.|_|_||__/_/ <___/|_\_|"]
banner += ["     |_|                  openwsn.org"]
banner += [""]
banner  = '\n'.join(banner)
print banner

#===== help text

Help('''
Usage:
    scons [<variable>=<value> ...] <project>
    scons docs
    scons [help-option]

project:
    A project is represented by a subdirectory of the 
    firmware{0}openos{0}projects directory, for a particular board. For 
    example, the 'oos_openwsn' project may be built for telosb. To specify a 
    project, exclude the leading digits in the directory name, like '03' for 
    oos_openwsn.

    variable=value pairs
    These pairs qualify how the project is built, and are organized here into
    functional groups. Below each variable's description are the valid 
    options, with the default value listed first.
    
    board        Board to build for. 'python' is for software simulation.
                 telosb, wsn430v14, wsn430v13b, gina, z1, python, iot-lab_M3
        
    toolchain    Toolchain implementation. The 'python' board requires gcc
                 (MinGW on Windows build host).
                 mspgcc, iar, iar-proj, gcc
    
    Connected hardware variables:
    bootload     Location of the board to bootload the binary on. 
                 COMx for Windows, /dev entries for Linux
                 Supports parallel operation with a comma-separated list,
                 for example 'COM5,COM6,COM7'.
    jtag         Location of the board to JTAG the binary to.
                 COMx for Windows, /dev entry for Linux
    fet_version  Firmware version running on the MSP-FET430uif for jtag.
                 2, 3
    
    Simulation variables:
    fastsim      Compiles the firmware for fast simulation.
                 1 (on), 0 (off)

    These simulation variables are for a cross-platform build, and are valid
    only from an amd64-linux build host.
    simhost      Host platform and OS for simulation. Default selection is
                 the current platform/OS, which of course is not a cross-
                 build. '-windows' cross-builds require MinGW-w64 toolchain.
                 amd64-linux, x86-linux, amd64-windows, x86-windows
    simhostpy    Home directory for simhost cross-build Python headers and 
                 shared library.
    
    Common variables:
    verbose      Print each complete compile/link comand.
                 0 (off), 1 (on)
    
docs:
    Generate source documentation in build{0}docs{0}html directory

help-option:
    --help       Display help text. Also display when no parameters to the
                 scons scommand.
'''.format(os.sep))

#============================ options =========================================

#===== options

# first value is default
command_line_options = {
    'board':       [
        # MSP430
        'telosb',
        'gina',
        'wsn430v13b',
        'wsn430v14',
        'z1',
        # Cortex-M3
        'cc2538', # TODO: replace by openmotecc2538 when directory renamed
        'openmotestm',
        'iot-lab_M3',
        'agilefox',
        # misc.
        'python',
    ],
    'toolchain':   [
        'mspgcc',
        'iar',
        'iar-proj',
        'armgcc',
        'gcc',
    ],
    'fet_version': ['2','3'],
    'verbose':     ['0','1'],
    'fastsim':     ['1','0'],
    'simhost':     ['amd64-linux','x86-linux','amd64-windows','x86-windows'],
    'simhostpy':   ['']                                # No reasonable default
}

def validate_option(key, value, env):
    if key not in command_line_options:
       raise ValueError("Unknown switch {0}.".format(key))
    if value not in command_line_options[key]:
       raise ValueError("Unknown {0} \"{1}\". Options are {2}.\n\n".format(key,value,','.join(command_line_options[key])))

# Define default value for simhost option
if os.name=='nt':
    defaultHost = 2
else:
    defaultHost = 0 if platform.architecture()[0]=='64bit' else 1

command_line_vars = Variables()
command_line_vars.AddVariables(
    (
        'board',                                           # key
        '',                                                # help
        command_line_options['board'][0],                  # default
        validate_option,                                   # validator
        None,                                              # converter
    ),
    (
        'toolchain',                                       # key
        '',                                                # help
        command_line_options['toolchain'][0],              # default
        validate_option,                                   # validator
        None,                                              # converter
    ),
    (
        'jtag',                                            # key
        '',                                                # help
        '',                                                # default
        None,                                              # validator
        None,                                              # converter
    ),
    (
        'fet_version',                                     # key
        '',                                                # help
        command_line_options['fet_version'][0],            # default
        validate_option,                                   # validator
        int,                                               # converter
    ),
    (
        'bootload',                                        # key
        '',                                                # help
        '',                                                # default
        None,                                              # validator
        None,                                              # converter
    ),
    (
        'verbose',                                         # key
        '',                                                # help
        command_line_options['verbose'][0],                # default
        validate_option,                                   # validator
        int,                                               # converter
    ),
    (
        'fastsim',                                         # key
        '',                                                # help
        command_line_options['fastsim'][0],                # default
        validate_option,                                   # validator
        int,                                               # converter
    ),
    (
        'simhost',                                         # key
        '',                                                # help
        command_line_options['simhost'][defaultHost],      # default
        validate_option,                                   # validator
        None,                                              # converter
    ),
    (
        'simhostpy',                                       # key
        '',                                                # help
        command_line_options['simhostpy'][0],              # default
        None,                                              # validator
        None,                                              # converter
    ),
)

if os.name=='nt':
    env = Environment(
        tools     = ['mingw'],
        variables = command_line_vars
    )
else:
    simhost = ARGUMENTS.get('simhost', 'none')
    board   = ARGUMENTS.get('board', 'none')
    if board=='python' and simhost.endswith('-windows'):
        # Cross-compile for simulation on Windows
        env = Environment(
            # crossMingw64 requires 'mingw_prefer_amd64' key
            tools              = ['crossMingw64'],
            mingw_prefer_amd64 = simhost.startswith('amd64-'),
            variables          = command_line_vars
        )
    else:
        env = Environment(
            variables = command_line_vars
        )

def default(env,target,source): print SCons.Script.help_text
Default(env.Command('default', None, default))

#============================ verbose =========================================

if not env['verbose']:
   env[    'CCCOMSTR']  = "Compiling $TARGET"
   env[  'SHCCCOMSTR']  = "Compiling (shared) $TARGET"
   env[    'ARCOMSTR']  = "Archiving $TARGET"
   env['RANLIBCOMSTR']  = "Indexing  $TARGET"
   env[  'LINKCOMSTR']  = "Linking   $TARGET"
   env['SHLINKCOMSTR']  = "Linking (shared)   $TARGET"

#============================ load SConscript's ===============================

# initialize targets
env['targets'] = {
   'all':     [],
   'all_std': [],
   'all_bsp': [],
   'all_drv': [],
   'all_oos': [],
}

# include docs SConscript
# which will discover targets for this board/toolchain
env.SConscript(
    os.path.join('docs','SConscript'),
    exports = ['env'],
)

# include main SConscript
# which will discover targets for this board/toolchain
env.SConscript(
    'SConscript',
    exports = ['env'],
)

# declare target group alias
for k,v in env['targets'].items():
   Alias(k,v)

#============================ admin targets ===================================

#===== list

def listFunction(env,target,source):
    output  = []
    output += ['\n']
    output += ['Avaiable targets for board={0} toolchain={1}'.format(env['board'],env['toolchain'])]
    output += ['\n']
    for k,v in env['targets'].items():
        output += [' - {0}'.format(k)]
        for t in v:
            output += ['    - {0}'.format(t)]
    output = '\n'.join(output)
    print output
list = env.Command('list', None, listFunction)
AlwaysBuild(list)
Alias('list',list)

#===== env

def envFunction(env,target,source):
    print env.Dump()
envCommand = env.Command('env', None, envFunction)
AlwaysBuild(envCommand)
Alias('env',envCommand)
