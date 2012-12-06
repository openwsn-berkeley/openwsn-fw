import os

env = Environment()

Default(None)

# the directory everything is built in
#buildDir = 'build/'

# some help text printed when SCons is called with '--help' parameter


Help(os.linesep.join(('',
                      'Usage: scons [options] target...',
                      'Options:',
                      '  project=<project>    project to build (e.g. telosb)',
                      ''
                     ))
    )
 

# by default, no target is specified
if not 'project' in ARGUMENTS:
    Help(os.linesep.join(('','Use scons -h project=<project> to see any additional project-specific options.')))
    

#============================ SConscripts =====================================


#bsp
#SConscript('firmware/openos/projects/common/01bsp_bothtimers/SConscript',
#           exports     = {'env':env},
#           )
#SConscript('firmware/openos/projects/common/01bsp_bsp_timer/SConscript',
#           exports     = {'env':env},
#           )
#SConscript('firmware/openos/projects/common/01bsp_closetimers/SConscript',
#           exports     = {'env':env},
#           )
#SConscript('firmware/openos/projects/common/01bsp_debugpins/SConscript',
#           exports     = {'env':env},
#           )
#SConscript('firmware/openos/projects/common/01bsp_leds/SConscript',
#           exports     = {'env':env},
#           )
#SConscript('firmware/openos/projects/common/01bsp_radio/SConscript',
#           exports     = {'env':env},
#           )
#SConscript('firmware/openos/projects/common/01bsp_radiotimer/SConscript',
#           exports     = {'env':env},
#           )
#SConscript('firmware/openos/projects/common/01bsp_uart/SConscript',
#           exports     = {'env':env},
#           )
# drivers
#SConscript('firmware/openos/projects/common/02drv_opentimers/SConscript',
#           exports     = {'env':env},
#           )
# openos
#SConscript('firmware/openos/projects/common/03oos_openwsn/SConscript',
#           exports     = {'env':env},
#           )

#===== libraries

# bsp
#SConscript('firmware/openos/bsp/boards/pc/SConscript',
#           exports     = {'env':env},
#           )
# kernel
#SConscript('firmware/openos/kernel/openos/SConscript',
#           exports     = {'env':env},
#           )
# drivers
#SConscript('firmware/openos/drivers/common/SConscript',
#           exports     = {'env':env},
#           )
# openstack
#SConscript('firmware/openos/openwsn/SConscript',
#           exports     = {'env':env},
#           )
#======================== Argument checking =====================

if 'project' in ARGUMENTS:
    env.Append(PROJECT = ARGUMENTS['project'])
elif len(BUILD_TARGETS) > 0:
    print 'Project must be specified to build targets. Use "scons --help" for details.'
    os._exit(-1)

#======================== Project setup =========================

# only set up for building if a project is specified
if 'PROJECT' in env:
    
    # directory where we put object and linked files
    # WARNING: -c (clean) removes the VARDIR, so it cannot be blank
    env['BUILDDIR'] = 'build'
    env['VARDIR']   = os.path.join(env['BUILDDIR'],env['PROJECT'])
    
    # include main sconscript
    env.SConscript('SConscript', exports = ['env'])
