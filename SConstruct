import os
import sys
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

#============================ options =========================================

#===== options

command_line_options = {
    'board':       ['telosb','wsn430v14','wsn430v13b','gina','z1','pc','python'],
    'toolchain':   ['mspgcc','iar','iar-proj','gcc'],
    'fet_version': ['2','3'],
    'verbose':     ['0','1']
}

def validate_option(key, value, env):
    if key not in command_line_options:
       raise ValueError("Unknown switch {0}.".format(key))
    if value not in command_line_options[key]:
       raise ValueError("Unknown {0} \"{1}\". Options are {2}.\n\n".format(key,value,','.join(command_line_options[key])))
    
command_line_vars = Variables()
command_line_vars.AddVariables(
    (
        'board',                                           # key
        'Board to build for.',                             # help
        command_line_options['board'][0],                  # default
        validate_option,                                   # validator
        None,                                              # converter
    ),
    (
        'toolchain',                                       # key
        'Toolchain to use.',                               # help
        command_line_options['toolchain'][0],              # default
        validate_option,                                   # validator
        None,                                              # converter
    ),
    (
        'jtag',                                            # key
        'Location of the board to JTAG the binary to.',    # help
        '',                                                # default
        None,                                              # validator
        None,                                              # converter
    ),
    (
        'fet_version',                                     # key
        'Firmware version running on the MSP-FET430uif.',  # help
        command_line_options['fet_version'][0],            # default
        validate_option,                                   # validator
        int,                                               # converter
    ),
    (
        'bootload',                                        # key
        'Location of the board to bootload the binary on.',# help
        '',                                                # default
        None,                                              # validator
        None,                                              # converter
    ),
    (
        'verbose',                                         # key
        'Print complete compile/link comand.',             # help
        command_line_options['verbose'][0],                # default
        validate_option,                                   # validator
        int,                                               # converter
    ),
)

if os.name=='nt':
    env = Environment(
        tools     = ['mingw'],
        variables = command_line_vars,
    )
else:
    env = Environment(
        variables = command_line_vars,
    )

#===== help text

Help("\nUsage: scons board=<b> toolchain=<tc> project\n\nWhere:")
Help(command_line_vars.GenerateHelpText(env))
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
