env = Environment()

# the directory everything is built in
buildDir = 'build/'

# some help text printed when SCons is called with '--help' parameter
Help('''
Build OpenWSN!
''')

# by default, no target is specified
Default(None)

# list of SConscripts
SConscript('firmware/openos/projects/common/01bsp_leds/SConscript',
           #variant_dir = buildDir+'firmware/openos/projects/common/01bsp_leds/',
           exports     = {'env':env},
           #duplicate   = 0,
           )

SConscript('firmware/openos/bsp/boards/pc/SConscript',
           #variant_dir = buildDir+'firmware/openos/bsp/boards/pc/',
           exports     = {'env':env},
           #duplicate   = 0,
           )