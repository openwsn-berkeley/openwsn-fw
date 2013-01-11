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
    action = '',
    suffix = '.ihex',
)

if   env['toolchain']=='mspgcc':
    
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
    
    # convert ELF to iHex
    elf2iHexFunc = Builder(
       action = 'msp430-objcopy --output-target=ihex $SOURCE $TARGET',
       suffix = '.ihex',
    )
    env.Append(BUILDERS = {'Elf2iHex' : elf2iHexFunc})
    
    # convert ELF to bin
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
    env.Replace(CC                = '"'+os.path.join(iarEw430BinDir,'icc430')+'"')
    env.Append(CCFLAGS            = '--no_cse')
    env.Append(CCFLAGS            = '--no_unroll')
    env.Append(CCFLAGS            = '--no_inline')
    env.Append(CCFLAGS            = '--no_code_motion')
    env.Append(CCFLAGS            = '--no_tbaa')
    env.Append(CCFLAGS            = '--debug')
    env.Append(CCFLAGS            = '-e')
    env.Append(CCFLAGS            = '--double=32 ')
    env.Append(CCFLAGS            = '--dlib_config "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\LIB\\DLIB\\dl430fn.h"')
    env.Append(CCFLAGS            = '--library_module')
    env.Append(CCFLAGS            = '-Ol ')
    env.Append(CCFLAGS            = '--multiplier=16')
    env.Replace(INCPREFIX         = '-I ')
    env.Replace(CCCOM             = '$CC $SOURCES -o $TARGET $CFLAGS $CCFLAGS $_CCCOMCOM')
    env.Replace(RANLIBCOM         = '')
    # archiver
    env.Replace(AR                = '"'+os.path.join(iarEw430BinDir,'xar')+'"')
    env.Replace(ARCOM             = '$AR $SOURCES -o $TARGET')
    env.Append(ARFLAGS            = '')
    # linker
    env.Replace(LINK              = '"'+os.path.join(iarEw430BinDir,'xlink')+'"')
    env.Replace(PROGSUFFIX        = '.exe')
    env.Replace(LIBDIRPREFIX      = '-I')
    env.Replace(LIBLINKDIRPREFIX  = '-I')
    env.Replace(LIBLINKPREFIX     = 'lib')
    env.Replace(LIBLINKSUFFIX     = '.a')
    env.Append(LINKFLAGS          = '-f "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\config\\multiplier.xcl"')
    env.Append(LINKFLAGS          = '-D_STACK_SIZE=50')
    env.Append(LINKFLAGS          = '-rt "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\LIB\\DLIB\\dl430fn.r43"')
    env.Append(LINKFLAGS          = '-e_PrintfLarge=_Printf')
    env.Append(LINKFLAGS          = '-e_ScanfLarge=_Scanf ')
    env.Append(LINKFLAGS          = '-D_DATA16_HEAP_SIZE=50')
    env.Append(LINKFLAGS          = '-s __program_start')
    env.Append(LINKFLAGS          = '-D_DATA20_HEAP_SIZE=50')
    env.Append(LINKFLAGS          = '-S')
    env.Append(LINKFLAGS          = '-Ointel-standard')
    env.Replace(LINKCOM           = '$LINK $SOURCES -o $TARGET $LINKFLAGS $__RPATH $_LIBDIRFLAGS $_LIBFLAGS')
    
    # converts ELF to iHex
    def changeExtensionFunction(target, source, env):
        baseName      = str(target[0]).split('.')[0]
        fromExtension = '.a43'
        toExtension   = '.ihex'
        print 'change extension {0} {1}->{2}'.format(baseName,fromExtension,toExtension)
        os.rename(
            baseName+fromExtension,
            baseName+toExtension,
        )
    changeExtensionBuilder = Builder(
        action = changeExtensionFunction,
        suffix = '.ihex'
    )
    env.Append(BUILDERS = {'Elf2iHex'  : changeExtensionBuilder})
    
    # convert ELF to bin
    env.Append(BUILDERS = {'Elf2iBin'  : dummyFunc})
    
    # print sizes
    env.Append(BUILDERS = {'PrintSize' : dummyFunc})

elif env['toolchain']=='iar-proj':
    
    try:
        iarEw430CommonBinDir      = os.path.join(os.environ['IAR_EW430_INSTALLDIR'],'common','bin')
    except KeyError as err:
        print 'You need to install environment variable IAR_EW430_INSTALLDIR which points to the installation directory of IAR Embedded Workbench for MSP430. Example: C:\Program Files\IAR Systems\Embedded Workbench 6.4'
        raise
    
    iarProjBuilderFunction = Builder(
        action = '"{0}" $SOURCE Debug'.format(
                    os.path.join(iarEw430CommonBinDir,'IarBuild')
                ),
        src_suffix  = '.ewp',
    )
    env.Append(BUILDERS = {'iarProjBuilder' : iarProjBuilderFunction})
    
    # converts ELF to iHex
    env.Append(BUILDERS = {'Elf2iHex'  : dummyFunc})
    
    # convert ELF to bin
    env.Append(BUILDERS = {'Elf2iBin'  : dummyFunc})
    
    # print sizes
    env.Append(BUILDERS = {'PrintSize' : dummyFunc})
    
else:
    raise SystemError('toolchain={0} unsupported.'.format(toolchain))

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

#============================ helpers =========================================

def buildLibs(projectDir):
    libs_dict = {
        '00std': [],
        '01bsp': ['libbsp'],
        '02drv': ['libbsp','libdrivers'],
        '03oos': ['libbsp','libdrivers','libopenos','libopenstack'],
    }
    
    returnVal = None
    for k,v in libs_dict.items():
        if projectDir.startswith(k):
           returnVal = v
    
    assert returnVal!=None
    
    return returnVal

def buildIncludePath(projectDir,localEnv):
    if projectDir.startswith('03oos_'):
        localEnv.Append(
            CPPPATH = [
                os.path.join('#','firmware','openos','openwsn'),
                os.path.join('#','firmware','openos','openwsn','03b-IPv6'),
                os.path.join('#','firmware','openos','openwsn','02b-MAChigh'),
                os.path.join('#','firmware','openos','openwsn','02a-MAClow'),
                os.path.join('#','firmware','openos','openwsn','cross-layers'),
                os.path.join('#','firmware','openos','drivers','common'),
            ]
        )

def populateTargetGroup(localEnv,targetName):
    env['targets']['all'].append(targetName)
    for prefix in ['std','bsp','drv','oos']:
        if targetName.startswith(prefix):
            env['targets']['all_'+prefix].append(targetName)

def sconscript_scanner(localEnv):
    
    # list subdirectories
    subdirs = [name for name in os.listdir('.') if os.path.isdir(os.path.join('.', name)) ]
    
    # parse dirs and build targets
    for projectDir in subdirs:
        added      = False
        targetName = projectDir[2:]
        
        '''
        # determine variant_dir
        if localEnv['toolchain']!='iar-proj':
            VariantDir(
                variant_dir = os.path.join(localEnv['VARDIR'],'projects',projectDir),
                src_dir     = projectDir,
            )
        '''
        
        if   (
                ('{0}.c'.format(projectDir) in os.listdir(projectDir)) and
                (localEnv['toolchain']!='iar-proj')
             ):
    
            target =  projectDir
            source = [os.path.join(projectDir,'{0}.c'.format(projectDir))]
            libs   = buildLibs(projectDir)
            
            buildIncludePath(projectDir,localEnv)
            
            exe = localEnv.Program(
                target  = target,
                source  = source,
                LIBS    = libs,
            )
            targetAction = localEnv.PostBuildExtras(exe)
            
            Alias(targetName, [targetAction])
            added = True
            
        elif (
                ('{0}.ewp'.format(projectDir) in os.listdir(projectDir)) and
                (localEnv['toolchain']=='iar-proj')
             ):
            
            source = [os.path.join(projectDir,'{0}.ewp'.format(projectDir))]
        
            targetAction = localEnv.iarProjBuilder(
                source  = source,
            )
            
            Alias(targetName, [targetAction])
            added = True
        
        if added:
            populateTargetGroup(localEnv,targetName)

env.AddMethod(sconscript_scanner, 'SconscriptScanner')
    
#============================ board ===========================================

# Get build environment from platform directory
buildEnv = env.SConscript(
    os.path.join('firmware','openos','projects',env['board'],'SConscript.env'),
    exports     = ['env'],
)

# bsp
bspDir          = os.path.join('#','firmware','openos','bsp','boards',buildEnv['BSP'])
bspVarDir       = os.path.join(buildEnv['VARDIR'],'bsp')
buildEnv.Append(CPPPATH = [bspDir])
buildEnv.SConscript(
    os.path.join(bspDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = bspVarDir,
)
buildEnv.Clean('libbsp', Dir(bspVarDir).abspath)
buildEnv.Append(LIBPATH = [bspVarDir])

# kernel
kernelDir       = os.path.join('#','firmware','openos','kernel','openos')
kernelVarDir    = os.path.join(buildEnv['VARDIR'],'kernel','openos')
buildEnv.SConscript(
    os.path.join(kernelDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = kernelVarDir,
)
buildEnv.Clean('libopenos', Dir(kernelVarDir).abspath)
buildEnv.Append(LIBPATH = [kernelVarDir])

# drivers
driversDir      = os.path.join('#','firmware','openos','drivers')
driversVarDir   = os.path.join(buildEnv['VARDIR'],'drivers')
buildEnv.SConscript(
    os.path.join(driversDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = driversVarDir,
)
buildEnv.Clean('libdrivers', Dir(driversVarDir).abspath)
buildEnv.Append(LIBPATH = [driversVarDir])

# openstack
openstackDir    = os.path.join('#','firmware','openos','openwsn')
openstackVarDir = os.path.join(buildEnv['VARDIR'],'openwsn')
buildEnv.SConscript(
    os.path.join(openstackDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = openstackVarDir,
)
buildEnv.Clean('libopenstack', Dir(openstackVarDir).abspath)
buildEnv.Append(LIBPATH = [openstackVarDir])

# projects
buildEnv.SConscript(
    os.path.join('firmware','openos','projects','SConscript'),
    exports     = {'env': buildEnv},
    #variant_dir = os.path.join(env['VARDIR'],'projects'),
)
