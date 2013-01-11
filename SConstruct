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
    'board':       ['telosb','gina','pc'],
    'toolchain':   ['mspgcc','iar','iar-proj','gcc','visualstudio'],
    'fet_version': ['2','3'],
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
)

env = Environment(variables = command_line_vars)

#===== help text

Help("\nUsage: scons board=<b> toolchain=<tc> project\n\nWhere:")
Help(command_line_vars.GenerateHelpText(env))
def default(env,target,source): print SCons.Script.help_text
Default(env.Command('default', None, default))

#============================ load SConscript's ===============================

# initialize targets
env['targets'] = {
   'all':     [],
   'all_std': [],
   'all_bsp': [],
   'all_drv': [],
   'all_oos': [],
}

# include main sconscript
env.SConscript(
    'SConscript',
    exports = ['env'],
)

# print list of targets
output = []
for k,v in env['targets'].items():
    output += [' - {0}'.format(k)]
    for t in v:
        output += ['    - {0}'.format(t)]
output = '\n'.join(output)
print output

# declare target group alias
for k,v in env['targets'].items():
   Alias(k,v)

# print final environment
# print env.Dump()