import os
import sys
import threading
import subprocess
import platform
import distutils.sysconfig
import sconsUtils

Import('env')

# directory where we put object and linked files
# WARNING: -c (clean) removes the VARDIR, so it cannot be blank
env['VARDIR']  = os.path.join('#','build','{0}_{1}'.format(env['board'],env['toolchain']))

# common include paths
if env['board']!='python':
    env.Append(
        CPPPATH = [
            os.path.join('#','inc'),
            os.path.join('#','bsp','boards'),
            os.path.join('#','bsp','chips'),
            os.path.join('#','drivers','common'),
            os.path.join('#','kernel'),
            os.path.join('#','openapps'),
            os.path.join('#','openstack'),
        ]
    )

#============================ toolchain =======================================

# dummy
dummyFunc = Builder(
    action = '',
    suffix = '.ihex',
)

if env['dagroot']==1:
    env.Append(CPPDEFINES    = 'DAGROOT')
if env['forcetopology']==1:
    env.Append(CPPDEFINES    = 'FORCETOPOLOGY')
if env['noadaptivesync']==1:
    env.Append(CPPDEFINES    = 'NOADAPTIVESYNC')

if   env['toolchain']=='mspgcc':
    
    if env['board'] not in ['telosb','wsn430v13b','wsn430v14','gina','z1']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    # compiler
    env.Replace(CC           = 'msp430-gcc')
    env.Append(CCFLAGS       = '-Wstrict-prototypes')
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
       suffix = '.bin',
    )
    env.Append(BUILDERS = {'Elf2iBin' : elf2BinFunc})
    
    # print sizes
    printSizeFunc = Builder(
        action = 'msp430-size $SOURCE',
        suffix = '.phonysize',
    )
    env.Append(BUILDERS = {'PrintSize' : printSizeFunc})

elif env['toolchain']=='iar':
    
    if env['board'] not in ['telosb','wsn430v13b','wsn430v14','gina','z1']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    env['IAR_EW430_INSTALLDIR'] = os.environ['IAR_EW430_INSTALLDIR']
    
    try:
        iarEw430BinDir            = os.path.join(env['IAR_EW430_INSTALLDIR'],'430','bin')
    except KeyError as err:
        print 'You need to install environment variable IAR_EW430_INSTALLDIR which points to the installation directory of IAR Embedded Workbench for MSP430. Example: C:\Program Files\IAR Systems\Embedded Workbench 6.5'
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
    env.Append(CCFLAGS            = '--dlib_config "'+env['IAR_EW430_INSTALLDIR']+'\\430\\LIB\\DLIB\\dl430fn.h"')
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
    env.Append(LINKFLAGS          = '-f "'+env['IAR_EW430_INSTALLDIR']+'\\430\\config\\linker\\multiplier.xcl"')
    env.Append(LINKFLAGS          = '-D_STACK_SIZE=50')
    env.Append(LINKFLAGS          = '-rt "'+env['IAR_EW430_INSTALLDIR']+'\\430\\LIB\\DLIB\\dl430fn.r43"')
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
    
    if env['board'] not in ['telosb','gina','wsn430v13b','wsn430v14','z1','openmotestm','agilefox','OpenMote-CC2538']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    env['IAR_EW430_INSTALLDIR'] = os.environ['IAR_EW430_INSTALLDIR']
    
    try:
        iarEw430CommonBinDir      = os.path.join(env['IAR_EW430_INSTALLDIR'],'common','bin')
    except KeyError as err:
        print 'You need to install environment variable IAR_EW430_INSTALLDIR which points to the installation directory of IAR Embedded Workbench for MSP430. Example: C:\Program Files\IAR Systems\Embedded Workbench 6.5'
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
    
elif env['toolchain']=='armgcc':
    
    if env['board'] not in ['OpenMote-CC2538','iot-lab_M3','iot-lab_A8-M3']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    if   env['board']=='OpenMote-CC2538':
        
        # compiler (C)
        env.Replace(CC           = 'arm-none-eabi-gcc')
        env.Append(CCFLAGS       = '-O0')
        env.Append(CCFLAGS       = '-Wall')
        env.Append(CCFLAGS       = '-Wa,-adhlns=${TARGET.base}.lst')
        env.Append(CCFLAGS       = '-c')
        env.Append(CCFLAGS       = '-fmessage-length=0')
        env.Append(CCFLAGS       = '-mcpu=cortex-m3')
        env.Append(CCFLAGS       = '-mthumb')
        env.Append(CCFLAGS       = '-g3')
        env.Append(CCFLAGS       = '-Wstrict-prototypes')
        # assembler
        env.Replace(AS           = 'arm-none-eabi-as')
        env.Append(ASFLAGS       = '-ggdb -g3 -mcpu=cortex-m3 -mlittle-endian')
        # linker
        env.Append(LINKFLAGS     = '-Tbsp/boards/OpenMote-CC2538/cc2538.lds')
        env.Append(LINKFLAGS     = '-nostartfiles')
        env.Append(LINKFLAGS     = '-Wl,-Map,${TARGET.base}.map')
        env.Append(LINKFLAGS     = '-mcpu=cortex-m3')
        env.Append(LINKFLAGS     = '-mthumb')
        env.Append(LINKFLAGS     = '-g3')
        # object manipulation
        env.Replace(OBJCOPY      = 'arm-none-eabi-objcopy')
        env.Replace(OBJDUMP      = 'arm-none-eabi-objdump')
        # archiver
        env.Replace(AR           = 'arm-none-eabi-ar')
        env.Append(ARFLAGS       = '')
        env.Replace(RANLIB       = 'arm-none-eabi-ranlib')
        env.Append(RANLIBFLAGS   = '')
        # misc
        env.Replace(NM           = 'arm-none-eabi-nm')
        env.Replace(SIZE         = 'arm-none-eabi-size')
        
    elif env['board'] in ['iot-lab_M3', 'iot-lab_A8-M3']:
        
         # compiler (C)
        env.Replace(CC           = 'arm-none-eabi-gcc')
        if os.name=='nt':
            env.Append(CCFLAGS   = '-DHSE_VALUE=((uint32_t)16000000)')
        else:
            env.Append(CCFLAGS   = '-DHSE_VALUE=\\(\\(uint32_t\\)16000000\\)')
        env.Append(CCFLAGS       = '-DSTM32F10X_HD')
        env.Append(CCFLAGS       = '-DUSE_STDPERIPH_DRIVER')
        env.Append(CCFLAGS       = '-ggdb')
        env.Append(CCFLAGS       = '-g3')
        env.Append(CCFLAGS       = '-std=gnu99')
        env.Append(CCFLAGS       = '-O0')
        env.Append(CCFLAGS       = '-Wall')
        env.Append(CCFLAGS       = '-Wstrict-prototypes')
        env.Append(CCFLAGS       = '-mcpu=cortex-m3')
        env.Append(CCFLAGS       = '-mlittle-endian')
        env.Append(CCFLAGS       = '-mthumb')
        env.Append(CCFLAGS       = '-mthumb-interwork')
        env.Append(CCFLAGS       = '-nostartfiles')
        # compiler (C++)
        env.Replace(CXX          = 'arm-none-eabi-g++')
        # assembler
        env.Replace(AS           = 'arm-none-eabi-as')
        env.Append(ASFLAGS       = '-ggdb -g3 -mcpu=cortex-m3 -mlittle-endian')
        # linker
        env.Append(LINKFLAGS     = '-DUSE_STDPERIPH_DRIVER')
        env.Append(LINKFLAGS     = '-DUSE_STM32_DISCOVERY')
        env.Append(LINKFLAGS     = '-g3')
        env.Append(LINKFLAGS     = '-ggdb')
        env.Append(LINKFLAGS     = '-mcpu=cortex-m3')
        env.Append(LINKFLAGS     = '-mlittle-endian')
        env.Append(LINKFLAGS     = '-static')
        env.Append(LINKFLAGS     = '-lgcc')
        env.Append(LINKFLAGS     = '-mthumb')
        env.Append(LINKFLAGS     = '-mthumb-interwork')
        env.Append(LINKFLAGS     = '-nostartfiles')
        env.Append(LINKFLAGS     = '-Tbsp/boards/'+env['board']+'/stm32_flash.ld')
        env.Append(LINKFLAGS     = os.path.join('build',env['board']+'_armgcc','bsp','boards',env['board'],'startup.o'))
        env.Append(LINKFLAGS     = os.path.join('build',env['board']+'_armgcc','bsp','boards',env['board'],'configure','stm32f10x_it.o'))
        # object manipulation
        env.Replace(OBJCOPY      = 'arm-none-eabi-objcopy')
        env.Replace(OBJDUMP      = 'arm-none-eabi-objdump')
        # archiver
        env.Replace(AR           = 'arm-none-eabi-ar')
        env.Append(ARFLAGS       = '')
        env.Replace(RANLIB       = 'arm-none-eabi-ranlib')
        env.Append(RANLIBFLAGS   = '')
        # misc
        env.Replace(NM           = 'arm-none-eabi-nm')
        env.Replace(SIZE         = 'arm-none-eabi-size')
        
    else:
        raise SystemError('unexpected board={0}'.format(env['board']))
    
    # converts ELF to iHex
    elf2iHexFunc = Builder(
       action = 'arm-none-eabi-objcopy -O ihex $SOURCE $TARGET',
       suffix = '.ihex',
    )
    env.Append(BUILDERS = {'Elf2iHex'  : elf2iHexFunc})
    
    # convert ELF to bin
    elf2BinFunc = Builder(
       action = 'arm-none-eabi-objcopy -O binary $SOURCE $TARGET',
       suffix = '.bin',
    )
    env.Append(BUILDERS = {'Elf2iBin'  : elf2BinFunc})
    
    # print sizes
    printSizeFunc = Builder(
        action = 'arm-none-eabi-size --format=berkeley -x --totals $SOURCE',
        suffix = '.phonysize',
    )
    env.Append(BUILDERS = {'PrintSize' : printSizeFunc})
    
elif env['toolchain']=='gcc':
    
    if env['board'] not in ['python']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    if env['board'] in ['python']:
        env.Append(CPPDEFINES = 'OPENSIM')
    
    if env['fastsim']==1:
        env.Append(CPPDEFINES = 'FASTSIM')
        #env.Append(CPPDEFINES = 'TRACE_ON')
    
    if os.name!='nt':
        if env['simhost'].endswith('linux'):
            # enabling shared library to be reallocated 
            env.Append(CCFLAGS        = '-fPIC')
            if not sys.platform.startswith('darwin'): # line added per FW-230
                env.Append(SHLINKFLAGS    = '-Wl,-Bsymbolic-functions') # per FW-176
                env.Append(SHCFLAGS       = '-Wl,-Bsymbolic-functions') # per FW-176
            
            if platform.architecture()[0]=='64bit' and env['simhost']=='x86-linux':
                # Cross-compile x86 Linux target from 64-bit host. Also see
                # projects/python/SConscript.env.
                env.Append(CCFLAGS        = '-m32')
                # Resolves a conflict between Python's pyconfig.h, which uses 
                # '200112'L, and libc's features.h, which uses '200809L'.
                env.Append(CPPDEFINES     = [('_POSIX_C_SOURCE','200112L')])
                env.Append(SHLINKFLAGS    = '-m32')
                
        if env['simhost'] == 'amd64-windows':
            # Used by Python includes
            env.Append(CPPDEFINES = 'MS_WIN64')
    
    # converts ELF to iHex
    env.Append(BUILDERS = {'Elf2iHex'  : dummyFunc})
    
    # convert ELF to bin
    env.Append(BUILDERS = {'Elf2iBin'  : dummyFunc})
    
    # print sizes
    env.Append(BUILDERS = {'PrintSize' : dummyFunc})

else:
    raise SystemError('unexpected toolchain {0}'.format(env['toolchain']))
    
    
#============================ upload over JTAG ================================

def jtagUploadFunc(location):
    if env['toolchain']=='armgcc':
        if env['board'] in ['iot-lab_M3','iot-lab_A8-M3']:
            return Builder(
                action      = os.path.join('bsp','boards',env['board'],'tools','flash.sh') + " $SOURCE",
                suffix      = '.phonyupload',
                src_suffix  = '.ihex',
            )
    else:
        if env['fet_version']==2:
            # MSP-FET430uif is running v2 Firmware
            return Builder(
                action      = 'mspdebug -d {0} -j uif "prog $SOURCE"'.format(location),
                suffix      = '.phonyupload',
                src_suffix  = '.ihex',
            )
        elif env['fet_version']==3:
            # MSP-FET430uif is running v2 Firmware
            if location in 'default':
                return Builder(
                action      = 'mspdebug tilib "prog $SOURCE"',
                suffix      = '.phonyupload',
                src_suffix  = '.ihex',
                )
            else:
                return Builder(
                    action      = 'mspdebug tilib -d {0} "prog $SOURCE"'.format(location),
                    suffix      = '.phonyupload',
                    src_suffix  = '.ihex',
                )
        else:
            raise SystemError('fet_version={0} unsupported.'.format(fet_version))
if env['jtag']:
    env.Append(BUILDERS = {'JtagUpload' : jtagUploadFunc(env['jtag'])})

#============================ bootload ========================================

class telosb_bootloadThread(threading.Thread):
    def __init__(self,comPort,hexFile,countingSem):
        
        # store params
        self.comPort         = comPort
        self.hexFile         = hexFile
        self.countingSem     = countingSem
        
        # initialize parent class
        threading.Thread.__init__(self)
        self.name            = 'telosb_bootloadThread_{0}'.format(self.comPort)
    
    def run(self):
        print 'starting bootloading on {0}'.format(self.comPort)
        subprocess.call(
            'python '+os.path.join('bootloader','telosb','bsl')+' --telosb -c {0} -r -e -I -p "{1}"'.format(self.comPort,self.hexFile),
            shell=True
        )
        print 'done bootloading on {0}'.format(self.comPort)
        
        # indicate done
        self.countingSem.release()

def telosb_bootload(target, source, env):
    bootloadThreads = []
    countingSem     = threading.Semaphore(0)
    # create threads
    for comPort in env['bootload'].split(','):
        bootloadThreads += [
            telosb_bootloadThread(
                comPort      = comPort,
                hexFile      = source[0],
                countingSem  = countingSem,
            )
        ]
    # start threads
    for t in bootloadThreads:
        t.start()
    # wait for threads to finish
    for t in bootloadThreads:
        countingSem.acquire()

# bootload
def BootloadFunc():
    if   env['board']=='telosb':
        return Builder(
            action      = telosb_bootload,
            suffix      = '.phonyupload',
            src_suffix  = '.ihex',
        )
    else:
        raise SystemError('bootloading on board={0} unsupported.'.format(env['board']))
if env['bootload']:
    env.Append(BUILDERS = {'Bootload' : BootloadFunc()})

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
        '00std': [                                                              ],
        '01bsp': [                                                      'libbsp'],
        '02drv': [                             'libkernel','libdrivers','libbsp'],
        '03oos': ['libopenstack','libopenapps','libkernel','libdrivers','libbsp'], # this order needed for mspgcc
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
                os.path.join('#','openstack'),
                os.path.join('#','openstack','03b-IPv6'),
                os.path.join('#','openstack','02b-MAChigh'),
                os.path.join('#','openstack','02a-MAClow'),
                os.path.join('#','openstack','cross-layers'),
                os.path.join('#','drivers','common'),
            ]
        )

def populateTargetGroup(localEnv,targetName):
    env['targets']['all'].append(targetName)
    for prefix in ['std','bsp','drv','oos']:
        if targetName.startswith(prefix):
            env['targets']['all_'+prefix].append(targetName)

def sconscript_scanner(localEnv):
    '''
    This function is called from the following directories:
    - projects\common\
    - projects\<board>\
    '''
    # list subdirectories
    subdirs = [name for name in os.listdir('.') if os.path.isdir(os.path.join('.', name)) ]
    
    # parse dirs and build targets
    for projectDir in subdirs:
        
        src_dir     = os.path.join(os.getcwd(),projectDir)
        variant_dir = os.path.join(env['VARDIR'],'projects',projectDir),
        
        added      = False
        targetName = projectDir[2:]
        
        if   (
                ('{0}.c'.format(projectDir) in os.listdir(projectDir)) and
                (localEnv['toolchain']!='iar-proj') and 
                (localEnv['board']!='python')
             ):
            
            localEnv.VariantDir(
                variant_dir = variant_dir,
                src_dir     = src_dir,
                duplicate   = 0,
            )
    
            target =  projectDir
            source = [os.path.join(projectDir,'{0}.c'.format(projectDir))]
            libs   = buildLibs(projectDir)
            
            buildIncludePath(projectDir,localEnv)
            
            # In Linux, you cannot have the same target name as the name of the
            # directory name.
            target=target+"_prog"
            
            exe = localEnv.Program(
                target  = target,
                source  = source,
                LIBS    = libs,
            )
            targetAction = localEnv.PostBuildExtras(exe)
            
            Alias(targetName, [targetAction])
            added = True
        
        elif (
                ('{0}.c'.format(projectDir) in os.listdir(projectDir)) and
                (localEnv['board']=='python')
             ):
            
            # build the artifacts in a separate directory
            localEnv.VariantDir(
                variant_dir = variant_dir,
                src_dir     = src_dir,
                duplicate   = 1,
            )
            
            # build both the application's and the Python module's main files
            sources_c = [
                os.path.join(projectDir,'{0}.c'.format(projectDir)),
                os.path.join('#','bsp','boards','python','openwsnmodule.c'),
            ]
            
            # objectify those two files
            for s in sources_c:
                temp = localEnv.Objectify(
                    target = localEnv.ObjectifiedFilename(s),
                    source = s,
                )
            
            # prepare environment for this build
            if os.name!='nt' and localEnv['simhost'].endswith('-windows'):
                # Cross-build handling -- find DLL, rather than hardcode version,
                # like 'python27.dll'
                pathnames = sconsUtils.findPattern('python*.dll', localEnv['simhostpy'])
                if pathnames:
                    pathname = pathnames[0]
                else:
                    raise SystemError("Can't find python dll in provided simhostpy")
                
                # ':' means no prefix, like 'lib', for shared library name
                pysyslib = ':{0}'.format(os.path.basename(pathname))
                pylibExt = '.pyd'
            else:
                pysyslib = 'python' + distutils.sysconfig.get_config_var('VERSION')
                pylibExt = distutils.sysconfig.get_config_var('SO')
            
            target = targetName
            source = [localEnv.ObjectifiedFilename(s) for s in sources_c]
            libs   = buildLibs(projectDir)
            libs  += [[pysyslib]]
            
            buildIncludePath(projectDir,localEnv)
            
            # build a shared library (a Python extension module) rather than an exe
            
            targetAction = localEnv.SharedLibrary(
                target,
                source,
                LIBS           = libs,
                SHLIBPREFIX    = '',
                SHLIBSUFFIX    = pylibExt,
            )
            
            Alias(targetName, [targetAction])
            added = True
            
        elif (
                ('{0}.ewp'.format(projectDir) in os.listdir(projectDir)) and
                (localEnv['toolchain']=='iar-proj')
             ):
            
            VariantDir(
                variant_dir = variant_dir,
                src_dir     = src_dir,
                duplicate   = 0,
            )
            
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
    os.path.join('projects',env['board'],'SConscript.env'),
    exports     = ['env'],
)

# inc
incDir          = os.path.join('#','inc')
incVarDir       = os.path.join(buildEnv['VARDIR'],'inc')
buildEnv.SConscript(
    os.path.join(incDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = incVarDir,
    duplicate   = 0,
)

# bspheader
bspHDir         = os.path.join('#','bsp','boards')
bspHVarDir      = os.path.join(buildEnv['VARDIR'],'bsp','boards')
buildEnv.SConscript(
    os.path.join(bspHDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = bspHVarDir,
    duplicate   = 0,
)

# bsp
bspDir          = os.path.join('#','bsp','boards',buildEnv['BSP'])
bspVarDir       = os.path.join(buildEnv['VARDIR'],'bsp','boards',buildEnv['BSP'])
buildEnv.Append(CPPPATH = [bspDir])
buildEnv.SConscript(
    os.path.join(bspDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = bspVarDir,
    duplicate   = 0,
)
buildEnv.Clean('libbsp', Dir(bspVarDir).abspath)
buildEnv.Append(LIBPATH = [bspVarDir])

# kernelheader
kernelHDir      = os.path.join('#','kernel')
kernelHVarDir   = os.path.join(buildEnv['VARDIR'],'kernel')
buildEnv.SConscript(
    os.path.join(kernelHDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = kernelHVarDir,
    duplicate   = 0,
)

# kernel
kernelDir       = os.path.join('#','kernel',buildEnv['kernel'])
kernelVarDir    = os.path.join(buildEnv['VARDIR'],'kernel',buildEnv['kernel'])
buildEnv.SConscript(
    os.path.join(kernelDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = kernelVarDir,
    duplicate   = 0,
)
buildEnv.Clean('libkernel', Dir(kernelVarDir).abspath)
buildEnv.Append(LIBPATH = [kernelVarDir])

# drivers
driversDir      = os.path.join('#','drivers')
driversVarDir   = os.path.join(buildEnv['VARDIR'],'drivers')
buildEnv.SConscript(
    os.path.join(driversDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = driversVarDir,
    duplicate   = 0,
)
buildEnv.Clean('libdrivers', Dir(driversVarDir).abspath)
buildEnv.Append(LIBPATH = [driversVarDir])

# openstack
openstackDir    = os.path.join('#','openstack')
openstackVarDir = os.path.join(buildEnv['VARDIR'],'openstack')
buildEnv.SConscript(
    os.path.join(openstackDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = openstackVarDir,
    duplicate   = 0,
)
buildEnv.Clean('libopenstack', Dir(openstackVarDir).abspath)
buildEnv.Append(LIBPATH = [openstackVarDir])

# openapps
openappsDir        = os.path.join('#','openapps')
openappsVarDir     = os.path.join(buildEnv['VARDIR'],'openapps')
buildEnv.SConscript(
    os.path.join(openappsDir,'SConscript'),
    exports        = {'env': buildEnv},
    variant_dir    = openappsVarDir,
    duplicate   = 0,
)
buildEnv.Clean('libopenapps', Dir(openappsVarDir).abspath)
buildEnv.Append(LIBPATH = [openappsVarDir])

# projects
buildEnv.SConscript(
    os.path.join('#','projects','SConscript'),
    exports     = {'env': buildEnv},
    #variant_dir = os.path.join(env['VARDIR'],'projects'),
)
