env = Environment()


# the directory everything is built in
buildDir = 'build/'

# some help text printed when SCons is called with '--help' parameter
Help('''
Build OpenWSN!
''')

# by default, no target is specified
Default(None)

#============================ SConscripts =====================================

#===== libraries
# bsp
SConscript('firmware/openos/bsp/boards/pc/SConscript',
           exports     = {'env':env},
           )
# kernel
SConscript('firmware/openos/kernel/openos/SConscript',
           exports     = {'env':env},
           )
# drivers
SConscript('firmware/openos/drivers/common/SConscript',
           exports     = {'env':env},
           )
# openstack
SConscript('firmware/openos/openwsn/SConscript',
           exports     = {'env':env},
           )

#===== projects
# bsp
SConscript('firmware/openos/projects/common/01bsp_bothtimers/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_bsp_timer/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_closetimers/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_debugpins/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_leds/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_radio/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_radiotimer/SConscript',
           exports     = {'env':env},
           )
SConscript('firmware/openos/projects/common/01bsp_uart/SConscript',
           exports     = {'env':env},
           )
# drivers
SConscript('firmware/openos/projects/common/02drv_opentimers/SConscript',
           exports     = {'env':env},
           )
# openos
SConscript('firmware/openos/projects/common/03oos_openwsn/SConscript',
           exports     = {'env':env},
           )
