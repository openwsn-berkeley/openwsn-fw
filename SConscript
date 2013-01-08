import os

Import('env')

# directory where we put object and linked files
# WARNING: -c (clean) removes the VARDIR, so it cannot be blank
env['VARDIR']  = os.path.join('#','build','{0}_{1}'.format(env['board'],env['toolchain']))

# common include paths
env.Append(
    CPPPATH = [
        os.path.join('#','firmware','openos','openwsn'),
        os.path.join('#','firmware','openos','bsp','boards'),
        os.path.join('#','firmware','openos','bsp','chips'),
        os.path.join('#','firmware','openos','kernel','openos'),
        os.path.join('#','firmware','openos','drivers','common'),
    ]
)

#============================ toolchain =======================================

if env['toolchain']=='mspgcc':
    env.Replace(CC          = 'msp430-gcc')
    env.Replace(LINK        = 'msp430-gcc')
    env.Replace(AR          = 'msp430-ar')
    env.Replace(RANLIB      = 'msp430-ranlib')
    
    env.Append(CCFLAGS      = '')
    env.Append(LINKFLAGS    = '')
    env.Append(ARFLAGS      = '')
    env.Append(RANLIBFLAGS  = '')
    
    # converts ELF to iHex
    elf2iHexFunc = Builder(
       action = 'msp430-objcopy --output-target=ihex $SOURCE $TARGET',
       suffix = '.ihex',
    )
    env.Append(BUILDERS = {'Elf2iHex' : elf2iHexFunc})
    
    # converts ELF to bin
    elf2BinFunc = Builder(
       action = 'msp430-objcopy --output-target=binary $SOURCE $TARGET',
       suffix = '.ihex',
    )
    env.Append(BUILDERS = {'Elf2iBin' : elf2BinFunc})
    
    # print sizes
    printSizeFunc = Builder(
        action = 'msp430-size $SOURCE',
        suffix = '.phonysize',
    )
    env.Append(BUILDERS = {'PrintSize' : printSizeFunc})
    
# upload over JTAG
def jtagUploadFunc(location):
    if   env['fet_version']==2:
        # MSP-FET430uif is running v2 Firmware
        return Builder(
            action      = 'mspdebug -d {0} -j uif "prog $SOURCE"'.format(location),
            suffix      = '.phonyupload',
            src_suffix  = '.ihex',
        )
    elif env['fet_version']==3:
        # MSP-FET430uif is running v2 Firmware
        return Builder(
            action      = 'mspdebug tilib -d {0} "prog $SOURCE"'.format(location),
            suffix      = '.phonyupload',
            src_suffix  = '.ihex',
        )
    else:
        raise SystemError('fet_version={0} unsupported.'.format(fet_version))
if env['jtag']:
   env.Append(BUILDERS = {'JtagUpload' : jtagUploadFunc(env['jtag'])})

# bootload
def BootloadFunc(location):
    if   env['board']=='telosb':
        return Builder(
            action      = 'python '+os.path.join('firmware','openos','bootloader','telosb','bsl')+' --telosb -c {0} -r -e -I -p $SOURCE"'.format(location),
            suffix      = '.phonyupload',
            src_suffix  = '.ihex',
        )
    else:
        raise SystemError('bootloading on board={0} unsupported.'.format(env['board']))
if env['bootload']:
    env.Append(BUILDERS = {'Bootload' : BootloadFunc(env['bootload'])})

# PostBuildExtras is a method called after a program (not a library) is built.
# You can any addition step in this function, such as converting the binary
# or copying it somewhere.
def extras(env, source):
    returnVal  = []
    returnVal += [env.PrintSize(source=source)]
    returnVal += [env.Elf2iHex(source=source)]
    returnVal += [env.Elf2iBin(source=source)]
    if   env['jtag']:
        returnVal += [env.JtagUpload(env.Elf2iHex(source))]
    elif env['bootload']:
        returnVal += [env.Bootload(env.Elf2iHex(source))]
    return returnVal
env.AddMethod(extras, 'PostBuildExtras')

#============================ board ===========================================

# Get build environment from platform directory
buildEnv = env.SConscript(
    os.path.join('firmware','openos','projects',env['board'],'SConscript.env'),
    exports = ['env']
)

# bsp
bspDir    = os.path.join('#','firmware','openos','bsp','boards',buildEnv['BSP'])
bspVarDir = os.path.join(buildEnv['VARDIR'],'bsp')
buildEnv.Append(CPPPATH = [bspDir])
buildEnv.SConscript(
    os.path.join(bspDir,'SConscript'),
    variant_dir = bspVarDir,
    exports     = {'env': buildEnv},
)
buildEnv.Clean('libbsp', Dir(bspVarDir).abspath)
buildEnv.Append(LIBPATH = [bspVarDir])

# kernel
kernelDir    = os.path.join('#','firmware','openos','kernel','openos')
kernelVarDir = os.path.join(buildEnv['VARDIR'],'kernel','openos')
buildEnv.SConscript(
    os.path.join(kernelDir,'SConscript'),
    variant_dir = kernelVarDir,
    exports     = {'env': buildEnv},
)
buildEnv.Clean('libopenos', Dir(kernelVarDir).abspath)
buildEnv.Append(LIBPATH = [kernelVarDir])

# drivers
driversDir    = os.path.join('#','firmware','openos','drivers')
driversVarDir = os.path.join(buildEnv['VARDIR'],'drivers')
buildEnv.SConscript(
    os.path.join(driversDir,'SConscript'),
    variant_dir = driversVarDir,
    exports     = {'env': buildEnv},
)
buildEnv.Clean('libdrivers', Dir(driversVarDir).abspath)
buildEnv.Append(LIBPATH = [driversVarDir])

# openstack
openstackDir    = os.path.join('#','firmware','openos','openwsn')
openstackVarDir = os.path.join(buildEnv['VARDIR'],'openwsn')
buildEnv.SConscript(
    os.path.join(openstackDir,'SConscript'),
    variant_dir = openstackVarDir,
    exports     = {'env': buildEnv},
)
buildEnv.Clean('libopenstack', Dir(openstackVarDir).abspath)
buildEnv.Append(LIBPATH = [openstackVarDir])

# projects
buildEnv.SConscript(
    os.path.join('firmware','openos','projects','SConscript'),
    exports = {'env': buildEnv},
)
