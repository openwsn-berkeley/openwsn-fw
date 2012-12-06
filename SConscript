import os

Import('env')

# common include paths
env.Append(CPPPATH = [os.path.join('#','firmware','openos','openwsn'),
                      os.path.join('#','firmware','openos','bsp','boards'),
                      os.path.join('#','firmware','openos','bsp','chips'),
                      os.path.join('#','firmware','openos','kernel','openos'),
                      os.path.join('#','firmware','openos','drivers','common')
                     ])
                   
# Default function to execute after program is compiled, this one does nothing.
# It is overridden to perform extra steps (convert to .hex for example) by
# calling AddMethod(...) inside of the platform-specific SConscript.env
def extras(source, env):
  return source
env.AddMethod(extras, 'PostBuildExtras')

# Get build environment from platform directory
buildEnv = env.SConscript(os.path.join('firmware','openos','projects',env['PROJECT'],'SConscript.env'),
                          exports = ['env']
                         )

# bsp
bspDir = os.path.join('#','firmware','openos','bsp','boards',buildEnv['BSP'])
bspVarDir = os.path.join(bspDir,buildEnv['VARDIR'])
buildEnv.Append(CPPPATH = [bspDir])
buildEnv.SConscript(os.path.join(bspDir,'SConscript'),
                    variant_dir = bspVarDir,
                    exports     = {'env': buildEnv},
                   )
buildEnv.Clean('libbsp', Dir(bspVarDir).abspath)
buildEnv.Append(LIBPATH = [bspVarDir])


# kernel
kernelDir = os.path.join('#','firmware','openos','kernel','openos')
kernelVarDir = os.path.join(kernelDir,buildEnv['VARDIR'])
buildEnv.SConscript(os.path.join(kernelDir,'SConscript'),
                    variant_dir = kernelVarDir,
                    exports     = {'env': buildEnv},
                   )
buildEnv.Clean('libopenos', Dir(kernelVarDir).abspath)
buildEnv.Append(LIBPATH = [kernelVarDir])


# drivers
driversDir = os.path.join('#','firmware','openos','drivers')
driversVarDir = os.path.join(driversDir,buildEnv['VARDIR'])
buildEnv.SConscript(os.path.join(driversDir,'SConscript'),
                    variant_dir = driversVarDir,
                    exports     = {'env': buildEnv},
                   )
buildEnv.Clean('libdrivers', Dir(driversVarDir).abspath)
buildEnv.Append(LIBPATH = [driversVarDir])


# openstack
openstackDir = os.path.join('#','firmware','openos','openwsn')
openstackVarDir = os.path.join(openstackDir,buildEnv['VARDIR'])
buildEnv.SConscript(os.path.join(openstackDir,'SConscript'),
                    variant_dir = openstackVarDir,
                    exports     = {'env': buildEnv},
                   )
buildEnv.Clean('libopenstack', Dir(openstackVarDir).abspath)
buildEnv.Append(LIBPATH = [openstackVarDir])


# projects
buildEnv.SConscript(os.path.join('firmware','openos','projects','SConscript'),
                    exports = {'env': buildEnv}
                   )
