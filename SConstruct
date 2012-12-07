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


#========= Argument checking =====================

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
