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

# dummy
dummyFunc = Builder(
    action = 'echo poipoi',
    suffix = '.phonysize',
)

if env['toolchain']=='mspgcc':
    
    # compiler
    env.Replace(CC           = 'msp430-gcc')
    env.Append(CCFLAGS       = '')
    # archiver
    env.Replace(AR           = 'msp430-ar')
    env.Append(ARFLAGS       = '')
    env.Replace(RANLIB       = 'msp430-ranlib')
    env.Append(RANLIBFLAGS   = '')
    # linker
    env.Replace(LINK         = 'msp430-gcc')
    env.Append(LINKFLAGS     = '')
    
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

elif env['toolchain']=='iar':
    
    try:
       iarEw430BinDir          = os.path.join(os.environ['IAR_EW430_INSTALLDIR'],'430','bin')
    except KeyError as err:
        print 'You need to install environment variable IAR_EW430_INSTALLDIR which points to the installation directory of IAR Embedded Workbench for MSP430. Example: C:\Program Files\IAR Systems\Embedded Workbench 6.4'
        raise
    
    # compiler
    env.Replace(CC           = '"'+os.path.join(iarEw430BinDir,'icc430')+'"')
    env.Append(CCFLAGS       = '--no_cse')
    env.Append(CCFLAGS       = '--no_unroll')
    env.Append(CCFLAGS       = '--no_inline')
    env.Append(CCFLAGS       = '--no_code_motion')
    env.Append(CCFLAGS       = '--no_tbaa')
    env.Append(CCFLAGS       = '--debug')
    env.Append(CCFLAGS       = '-D__MSP430F1611__')
    env.Append(CCFLAGS       = '-e')
    env.Append(CCFLAGS       = '--double=32 ')
    env.Append(CCFLAGS       = '--dlib_config "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\LIB\\DLIB\\dl430xsfn.h"')
    #env.Append(CCFLAGS       = '--library_module')
    env.Append(CCFLAGS       = '--core=430X')
    env.Append(CCFLAGS       = '--data_model=small')
    env.Append(CCFLAGS       = '-Ol ')
    env.Append(CCFLAGS       = '--multiplier=16s')
    #env.Append(CCFLAGS       = '--silent')
    env.Replace(INCPREFIX    = '-I ')
    env.Replace(CCCOM        = '$CC $SOURCES -o $TARGET $CFLAGS $CCFLAGS $_CCCOMCOM')
    env.Replace(RANLIBCOM    = '')
    # archiver
    env.Replace(AR           = '"'+os.path.join(iarEw430BinDir,'xar')+'"')
    env.Replace(ARCOM        = '$AR $SOURCES -o $TARGET')
    env.Append(ARFLAGS       = '')
    # linker
    env.Replace(LINK         = '"'+os.path.join(iarEw430BinDir,'xlink')+'"')
    env.Replace(PROGSUFFIX = '.exe')
    env.Replace(LIBDIRPREFIX = '-I')
    env.Replace(LIBLINKDIRPREFIX = '-I')
    env.Replace(LIBLINKPREFIX = 'lib')
    env.Replace(LIBLINKSUFFIX = '.a')
    #env.Append(LINKFLAGS     = '-Felf')
    #env.Append(LINKFLAGS     = '-yn')
    #env.Append(LINKFLAGS     = '-xsnh')
    #env.Append(LINKFLAGS     = '-I"C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\LIB\\"')
    env.Append(LINKFLAGS     = '-f "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\CONFIG\\lnk430F1611.xcl"')
    env.Append(LINKFLAGS     = '-f "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\config\\multiplier.xcl"')
    env.Append(LINKFLAGS     = '-D_STACK_SIZE=50')
    env.Append(LINKFLAGS     = '-rt "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\LIB\\DLIB\\dl430xsfn.r43"')
    env.Append(LINKFLAGS     = '-e_PrintfLarge=_Printf')
    env.Append(LINKFLAGS     = '-e_ScanfLarge=_Scanf ')
    env.Append(LINKFLAGS     = '-D_DATA16_HEAP_SIZE=50')
    env.Append(LINKFLAGS     = '-s __program_start')
    env.Append(LINKFLAGS     = '-S')
    env.Append(LINKFLAGS     = '-D_DATA20_HEAP_SIZE=50')
    env.Append(LINKFLAGS     = '-Ointel-standard=poipoi.bin')
    env.Replace(LINKCOM      = '$LINK -o $TARGET $LINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS $_LIBFLAGS')
    #env.Replace(LINKCOM      = '$LINK $SOURCES -o $TARGET $LINKFLAGS')
    
    #print env.Dump()
    
    env.Append(BUILDERS = {'Elf2iHex' : dummyFunc})
    env.Append(BUILDERS = {'Elf2iBin' : dummyFunc})    
    env.Append(BUILDERS = {'PrintSize' : dummyFunc})

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
