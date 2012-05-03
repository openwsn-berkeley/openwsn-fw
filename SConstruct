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
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_radio/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_timer/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_uart/SConscript',
           exports     = {'env':env},
           )

SConscript('firmware/openos/projects/common/02drv_opentimers/SConscript',
           exports     = {'env':env},
           )

SConscript('firmware/openos/drivers/common/SConscript',
           exports     = {'env':env},
           )

SConscript('firmware/openos/bsp/boards/pc/SConscript',
           #variant_dir = buildDir+'firmware/openos/projects/common/01bsp_leds/',
           exports     = {'env':env},
           #duplicate   = 0,
           )