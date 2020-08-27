import glob
import os
import platform
import subprocess
import sys
import threading

import SCons
import colorama as c
import distutils.sysconfig
import sconsUtils

Import('env')

c.init()

# python2/python2.7 are not recognized in windows so use 'python' directly and assume the right version is installed.
if os.name == 'nt':  # Windows
    PYTHON_PY = 'python '
elif os.name == 'posix':  # Linux
    PYTHON_PY = 'python2 '

# directory where we put object and linked files
# WARNING: -c (clean) removes the VARDIR, so it cannot be blank
env['VARDIR'] = os.path.join('#', 'build', '{0}_{1}'.format(env['board'], env['toolchain']))

# Add the board name to the build environment so we can check for configuration errors
if env['board'] == 'python':
    env.Append(CPPDEFINES='PYTHON_BOARD')
elif env['board'] == 'telosb':
    env.Append(CPPDEFINES='TELOSB')
elif env['board'] == 'wsn430v13b':
    env.Append(CPPDEFINES='WSN430V13B')
elif env['board'] == 'wsn430v14':
    env.Append(CPPDEFINES='WSN430V14')
elif env['board'] == 'gina':
    env.Append(CPPDEFINES='GINA')
elif env['board'] == 'z1':
    env.Append(CPPDEFINES='Z1')
elif env['board'] == 'openmote-cc2538':
    env.Append(CPPDEFINES='OPENMOTE_CC2538')
elif env['board'] == 'openmote-b':
    env.Append(CPPDEFINES='OPENMOTE_B')
elif env['board'] == 'openmote-b-24ghz':
    env.Append(CPPDEFINES='OPENMOTE_B_24GHZ')
elif env['board'] == 'openmote-b-subghz':
    env.Append(CPPDEFINES='OPENMOTE_B_SUBGHZ')
elif env['board'] == 'openmotestm':
    env.Append(CPPDEFINES='OPENMOTESTM')
elif env['board'] == 'agilefox':
    env.Append(CPPDEFINES='AGILEFOX')
elif env['board'] == 'iot-lab_M3':
    env.Append(CPPDEFINES='IOTLAB_M3')
elif env['board'] == 'iot-lab_A8-M3':
    env.Append(CPPDEFINES='IOTLAB_A8_M3')
elif env['board'] == 'nrf52840':
    env.Append(CPPDEFINES='NRF52840')
elif env['board'] == 'samr21_xpro':
    env.Append(CPPDEFINES='SAMR21_XPRO')
else:
    print c.Fore.RED + "Unsupported board: {}".format(env['board']) + c.Fore.RESET
    Exit(-1)

# common include paths
if env['board'] != 'python':
    env.Append(
        CPPPATH=[
            os.path.join('#', 'inc'),
            os.path.join('#', 'bsp', 'boards'),
            os.path.join('#', 'bsp', 'boards', 'common'),
            os.path.join('#', 'bsp', 'chips'),
            os.path.join('#', 'drivers', 'common'),
            os.path.join('#', 'kernel'),
            os.path.join('#', 'openweb'),
            os.path.join('#', 'openapps'),
            os.path.join('#', 'openstack'),
        ]
    )

# ============================ toolchain =======================================

# dummy
dummyFunc = Builder(action='', suffix='.ihex')

# add the build variables
if env['atmel_24ghz'] == 1:
    env.Append(CPPDEFINES='ATMEL_24GHZ')

if env['toolchain'] == 'mspgcc':
    if env['board'] not in ['telosb', 'wsn430v13b', 'wsn430v14', 'gina', 'z1']:
        print c.Fore.RED + 'Toolchain {0} can not be used for board {1}'.format(env['toolchain'],
                                                                                env['board']) + c.Fore.RESET
        Exit(-1)

    # compiler
    env.Replace(CC='msp430-gcc')
    env.Append(CCFLAGS='-Wstrict-prototypes')
    env.Append(CCFLAGS='-ffunction-sections')
    env.Append(CCFLAGS='-fdata-sections')
    env.Append(CCFLAGS='')

    # archiver
    env.Replace(AR='msp430-ar')
    env.Append(ARFLAGS='')
    env.Replace(RANLIB='msp430-ranlib')
    env.Append(RANLIBFLAGS='')
    # linker
    env.Replace(LINK='msp430-gcc')
    env.Append(LINKFLAGS='')

    # convert ELF to iHex
    elf2iHexFunc = Builder(action='msp430-objcopy --output-target=ihex $SOURCE $TARGET', suffix='.ihex')
    env.Append(BUILDERS={'Elf2iHex': elf2iHexFunc})

    # convert ELF to bin
    elf2BinFunc = Builder(action='msp430-objcopy --output-target=binary $SOURCE $TARGET', suffix='.bin')
    env.Append(BUILDERS={'Elf2iBin': elf2BinFunc})

    # print sizes
    printSizeFunc = Builder(action='msp430-size $SOURCE', suffix='.phonysize')
    env.Append(BUILDERS={'PrintSize': printSizeFunc})

elif env['toolchain'] == 'iar':

    if env['board'] not in ['telosb', 'wsn430v13b', 'wsn430v14', 'gina', 'z1']:
        print c.Fore.RED + 'Toolchain {0} can not be used for board {1}'.format(env['toolchain'],
                                                                                env['board']) + c.Fore.RESET
        Exit(-1)

    env['IAR_EW430_INSTALLDIR'] = os.environ['IAR_EW430_INSTALLDIR']

    try:
        iarEw430BinDir = os.path.join(env['IAR_EW430_INSTALLDIR'], '430', 'bin')
    except KeyError as err:
        print c.Fore.RED + 'You need to install environment variable IAR_EW430_INSTALLDIR which points to the ' \
                           'installation directory of IAR Embedded Workbench for MSP430.' \
                           ' Example: C:\Program Files\IAR Systems\Embedded Workbench 6.5' + c.Fore.RESET
        Exit(-1)

    # compiler
    env.Replace(CC='"' + os.path.join(iarEw430BinDir, 'icc430') + '"')
    env.Append(CCFLAGS='--no_cse')
    env.Append(CCFLAGS='--no_unroll')
    env.Append(CCFLAGS='--no_inline')
    env.Append(CCFLAGS='--no_code_motion')
    env.Append(CCFLAGS='--no_tbaa')
    env.Append(CCFLAGS='--debug')
    env.Append(CCFLAGS='-e')
    env.Append(CCFLAGS='--double=32 ')
    env.Append(CCFLAGS='--dlib_config "' + env['IAR_EW430_INSTALLDIR'] + '\\430\\LIB\\DLIB\\dl430fn.h"')
    env.Append(CCFLAGS='--library_module')
    env.Append(CCFLAGS='-Ol ')
    env.Append(CCFLAGS='--multiplier=16')
    env.Replace(INCPREFIX='-I ')
    env.Replace(CCCOM='$CC $SOURCES -o $TARGET $CFLAGS $CCFLAGS $_CCCOMCOM')
    env.Replace(RANLIBCOM='')

    # archiver
    env.Replace(AR='"' + os.path.join(iarEw430BinDir, 'xar') + '"')
    env.Replace(ARCOM='$AR $SOURCES -o $TARGET')
    env.Append(ARFLAGS='')

    # linker
    env.Replace(LINK='"' + os.path.join(iarEw430BinDir, 'xlink') + '"')
    env.Replace(PROGSUFFIX='.exe')
    env.Replace(LIBDIRPREFIX='-I')
    env.Replace(LIBLINKDIRPREFIX='-I')
    env.Replace(LIBLINKPREFIX='lib')
    env.Replace(LIBLINKSUFFIX='.a')
    env.Append(LINKFLAGS='-f "' + env['IAR_EW430_INSTALLDIR'] + '\\430\\config\\linker\\multiplier.xcl"')
    env.Append(LINKFLAGS='-D_STACK_SIZE=50')
    env.Append(LINKFLAGS='-rt "' + env['IAR_EW430_INSTALLDIR'] + '\\430\\LIB\\DLIB\\dl430fn.r43"')
    env.Append(LINKFLAGS='-e_PrintfLarge=_Printf')
    env.Append(LINKFLAGS='-e_ScanfLarge=_Scanf ')
    env.Append(LINKFLAGS='-D_DATA16_HEAP_SIZE=50')
    env.Append(LINKFLAGS='-s __program_start')
    env.Append(LINKFLAGS='-D_DATA20_HEAP_SIZE=50')
    env.Append(LINKFLAGS='-S')
    env.Append(LINKFLAGS='-Ointel-standard')
    env.Replace(LINKCOM='$LINK $SOURCES -o $TARGET $LINKFLAGS $__RPATH $_LIBDIRFLAGS $_LIBFLAGS')

    # converts ELF to iHex
    def change_ext_function(target, source, env):
        base_name = str(target[0]).split('.')[0]
        from_extension = '.a43'
        to_extension = '.ihex'
        print 'change extension {0} {1}->{2}'.format(base_name, from_extension, to_extension)
        os.rename(base_name + from_extension,base_name + to_extension)

    change_ext_builder = Builder(action=change_ext_function,suffix='.ihex')
    env.Append(BUILDERS={'Elf2iHex': change_ext_builder})

    # convert ELF to bin
    env.Append(BUILDERS={'Elf2iBin': dummyFunc})

    # print sizes
    env.Append(BUILDERS={'PrintSize': dummyFunc})

elif env['toolchain'] == 'iar-proj':
    if env['board'] not in ['telosb', 'gina', 'wsn430v13b', 'wsn430v14', 'z1', 'openmotestm', 'agilefox',
                            'openmote-cc2538', 'openmote-b', 'openmote-b-24ghz', 'openmote-b-subghz', 'iot-lab_M3']:
        print c.Fore.RED + 'Toolchain {0} can not be used for board {1}'.format(env['toolchain'],
                                                                                env['board']) + c.Fore.RESET
        Exit(-1)

    env['IAR_EW430_INSTALLDIR'] = os.environ['IAR_EW430_INSTALLDIR']

    try:
        iarEw430CommonBinDir = os.path.join(env['IAR_EW430_INSTALLDIR'], 'common', 'bin')
    except KeyError as err:
        print c.Fore.RED + 'You need to install environment variable IAR_EW430_INSTALLDIR which points to the ' \
                           'installation directory of IAR Embedded Workbench for MSP430. ' \
                           'Example: C:\Program Files\IAR Systems\Embedded Workbench 6.5' + c.Fore.RESET
        Exit(-1)

    iar_proj_builder_func = Builder(
        action='"{0}" $SOURCE Debug'.format(os.path.join(iarEw430CommonBinDir, 'IarBuild')),src_suffix='.ewp')
    env.Append(BUILDERS={'iarProjBuilder': iar_proj_builder_func})

    # converts ELF to iHex
    env.Append(BUILDERS={'Elf2iHex': dummyFunc})

    # convert ELF to bin
    env.Append(BUILDERS={'Elf2iBin': dummyFunc})

    # print sizes
    env.Append(BUILDERS={'PrintSize': dummyFunc})

elif env['toolchain'] == 'armgcc':

    if env['board'] not in ['silabs-ezr32wg', 'openmote-cc2538', 'openmote-b', 'openmote-b-24ghz', 'openmote-b-subghz',
                            'iot-lab_M3', 'iot-lab_A8-M3', 'openmotestm', 'samr21_xpro', 'scum', 'nrf52840']:
        print c.Fore.RED + 'Toolchain {0} can not be used for board {1}'.format(env['toolchain'],
                                                                                env['board']) + c.Fore.RESET
        Exit(-1)

    if env['board'] in ['openmote-cc2538', 'openmote-b', 'openmote-b-24ghz', 'openmote-b-subghz']:
        if env['revision'] == "A1":
            linker_file = 'cc2538sf23.lds'
            print "*** OPENMOTE CC2538 REV. A1 ***\n"
        else:
            linker_file = 'cc2538sf53.lds'

        # compiler (C)
        env.Replace(CC='arm-none-eabi-gcc')
        env.Append(CCFLAGS='-O0')
        env.Append(CCFLAGS='-Wall')
        env.Append(CCFLAGS='-Wa,-adhlns=${TARGET.base}.lst')
        env.Append(CCFLAGS='-c')
        env.Append(CCFLAGS='-ffunction-sections')
        env.Append(CCFLAGS='-fdata-sections')
        env.Append(CCFLAGS='-fmessage-length=0')
        env.Append(CCFLAGS='-mcpu=cortex-m3')
        env.Append(CCFLAGS='-mthumb')
        env.Append(CCFLAGS='-g3')
        env.Append(CCFLAGS='-Wstrict-prototypes')
        if env['revision'] == "A1":
            env.Append(CCFLAGS='-DREVA1=1')

        # assembler
        env.Replace(AS='arm-none-eabi-as')
        env.Append(ASFLAGS='-ggdb -g3 -mcpu=cortex-m3 -mlittle-endian')
        if env['revision'] == "A1":
            env.Append(ASFLAGS='-DREVA1=1')

        # linker
        env.Append(LINKFLAGS='-Tbsp/boards/' + env['board'] + '/' + linker_file)
        env.Append(LINKFLAGS='-nostartfiles')
        env.Append(LINKFLAGS='-specs=nosys.specs')
        env.Append(LINKFLAGS='-specs=nano.specs')
        env.Append(LINKFLAGS='-Wl,-Map,${TARGET.base}.map')
        env.Append(LINKFLAGS='-Wl,--gc-sections')
        env.Append(LINKFLAGS='-mcpu=cortex-m3')
        env.Append(LINKFLAGS='-mthumb')
        env.Append(LINKFLAGS='-g3')
        if env['revision'] == "A1":
            env.Append(LINKFLAGS='-DREVA1=1')

        # object manipulation
        env.Replace(OBJCOPY='arm-none-eabi-objcopy')
        env.Replace(OBJDUMP='arm-none-eabi-objdump')
        # archiver
        env.Replace(AR='arm-none-eabi-ar')
        env.Append(ARFLAGS='')
        env.Replace(RANLIB='arm-none-eabi-ranlib')
        env.Append(RANLIBFLAGS='')
        # misc
        env.Replace(NM='arm-none-eabi-nm')
        env.Replace(SIZE='arm-none-eabi-size')

    elif env['board'] == 'silabs-ezr32wg':
        # compiler (C)
        env.Replace(CC='arm-none-eabi-gcc')
        env.Append(CCFLAGS='-O0')
        env.Append(CCFLAGS='-Wall')
        env.Append(CCFLAGS='-Wa,-adhlns=${TARGET.base}.lst')
        env.Append(CCFLAGS='-c')
        env.Append(CCFLAGS='-fmessage-length=0')
        env.Append(CCFLAGS='-mcpu=cortex-m4')
        env.Append(CCFLAGS='-mthumb')
        env.Append(CCFLAGS='-g')
        env.Append(CCFLAGS='-std=gnu99')
        env.Append(CCFLAGS='-O0')
        env.Append(CCFLAGS='-Wall')
        env.Append(CCFLAGS='-Wstrict-prototypes')
        env.Append(CCFLAGS='-ffunction-sections')
        env.Append(CCFLAGS='-fdata-sections')
        env.Append(CCFLAGS='-mfpu=fpv4-sp-d16')
        env.Append(CCFLAGS='-mfloat-abi=softfp')
        env.Append(CCFLAGS='-DEZR32WG330F256R60=1')

        # assembler
        env.Replace(AS='arm-none-eabi-as')
        env.Append(ASFLAGS='-g -gdwarf-2 -mcpu=cortex-m4 -mthumb -c -x assembler-with-cpp ')
        env.Append(ASFLAGS='-DEZR32WG330F256R60=1')

        # linker
        env.Append(LINKFLAGS='-g -gdwarf-2 -mcpu=cortex-m4 -mthumb -Tbsp/boards/silabs-ezr32wg/GCC/ezr32wg.ld')
        env.Append(LINKFLAGS='-Xlinker --gc-sections -Xlinker')
        env.Append(LINKFLAGS='-Map=${TARGET.base}.map')

        env.Append(LINKFLAGS='-mfpu=fpv4-sp-d16 -mfloat-abi=softfp  --specs=nosys.specs')
        # --specs=nano.specs
        # env.Append(LINKFLAGS     = '-lgcc -lc -lnosys')
        env.Append(LINKFLAGS='-Wl,--start-group -lgcc -lc -lg -lm -lnosys -Wl,--end-group')

        # object manipulation
        env.Replace(OBJCOPY='arm-none-eabi-objcopy')
        env.Replace(OBJDUMP='arm-none-eabi-objdump')
        # archiver
        env.Replace(AR='arm-none-eabi-ar')
        env.Append(ARFLAGS='')
        env.Replace(RANLIB='arm-none-eabi-ranlib')
        env.Append(RANLIBFLAGS='')
        # misc
        env.Replace(NM='arm-none-eabi-nm')
        env.Replace(SIZE='arm-none-eabi-size')

    elif env['board'] in ['openmotestm', 'iot-lab_M3', 'iot-lab_A8-M3']:

        # compiler (C)
        env.Replace(CC='arm-none-eabi-gcc')
        if os.name == 'nt':
            env.Append(CCFLAGS='-DHSE_VALUE=((uint32_t)16000000)')
        else:
            env.Append(CCFLAGS='-DHSE_VALUE=\\(\\(uint32_t\\)16000000\\)')
        env.Append(CCFLAGS='-DSTM32F10X_HD')
        env.Append(CCFLAGS='-DUSE_STDPERIPH_DRIVER')
        env.Append(CCFLAGS='-ggdb')
        env.Append(CCFLAGS='-g3')
        env.Append(CCFLAGS='-std=gnu99')
        env.Append(CCFLAGS='-O0')
        env.Append(CCFLAGS='-Wall')
        env.Append(CCFLAGS='-Wstrict-prototypes')
        env.Append(CCFLAGS='-mcpu=cortex-m3')
        env.Append(CCFLAGS='-mlittle-endian')
        env.Append(CCFLAGS='-mthumb')
        env.Append(CCFLAGS='-mthumb-interwork')
        env.Append(CCFLAGS='-nostartfiles')
        # compiler (C++)
        env.Replace(CXX='arm-none-eabi-g++')
        # assembler
        env.Replace(AS='arm-none-eabi-as')
        env.Append(ASFLAGS='-ggdb -g3 -mcpu=cortex-m3 -mlittle-endian')
        # linker
        env.Append(LINKFLAGS='-DUSE_STDPERIPH_DRIVER')
        env.Append(LINKFLAGS='-DUSE_STM32_DISCOVERY')
        env.Append(LINKFLAGS='-g3')
        env.Append(LINKFLAGS='-ggdb')
        env.Append(LINKFLAGS='-mcpu=cortex-m3')
        env.Append(LINKFLAGS='-mlittle-endian')
        env.Append(LINKFLAGS='-static')
        env.Append(LINKFLAGS='-lgcc')
        env.Append(LINKFLAGS='-mthumb')
        env.Append(LINKFLAGS='-mthumb-interwork')
        env.Append(LINKFLAGS='-nostartfiles')
        env.Append(LINKFLAGS='-Tbsp/boards/' + env['board'] + '/stm32_flash.ld')
        env.Append(
            LINKFLAGS=os.path.join('build', env['board'] + '_armgcc', 'bsp', 'boards', env['board'], 'startup.o'))
        env.Append(LINKFLAGS=os.path.join('build', env['board'] + '_armgcc', 'bsp', 'boards', env['board'], 'configure',
                                          'stm32f10x_it.o'))
        # object manipulation
        env.Replace(OBJCOPY='arm-none-eabi-objcopy')
        env.Replace(OBJDUMP='arm-none-eabi-objdump')
        # archiver
        env.Replace(AR='arm-none-eabi-ar')
        env.Append(ARFLAGS='')
        env.Replace(RANLIB='arm-none-eabi-ranlib')
        env.Append(RANLIBFLAGS='')
        # misc
        env.Replace(NM='arm-none-eabi-nm')
        env.Replace(SIZE='arm-none-eabi-size')

    elif env['board'] == 'samr21_xpro':

        # compiler (C)
        env.Replace(CC='arm-none-eabi-gcc')
        env.Append(CCFLAGS='-O1')
        env.Append(CCFLAGS='-Wall')
        env.Append(CCFLAGS='-Wa,-adhlns=${TARGET.base}.lst')
        env.Append(CCFLAGS='-c')
        env.Append(CCFLAGS='-fmessage-length=0')
        env.Append(CCFLAGS='-mcpu=cortex-m0plus')
        env.Append(CCFLAGS='-mthumb')
        env.Append(CCFLAGS='-g3')
        env.Append(CCFLAGS='-Wstrict-prototypes')
        env.Append(CCFLAGS='-Ibsp/boards/samr21_xpro/drivers/inc')
        env.Append(CCFLAGS='-Ibsp/boards/samr21_xpro/cmsis/inc')
        env.Append(CCFLAGS='-D__SAMR21G18A__')
        env.Append(CCFLAGS='-Ibsp/boards/samr21_xpro/SAMR21_DFP/1.0.34/include')
        env.Append(CCFLAGS='-std=c99')
        # assembler
        env.Replace(AS='arm-none-eabi-as')
        env.Append(ASFLAGS='-ggdb -g3 -mcpu=cortex-m0plus -mlittle-endian')
        # linker
        env.Append(LINKFLAGS='-Tbsp/boards/samr21_xpro/cmsis/linkerScripts/samr21g18a_flash.ld')
        #        env.Append(LINKFLAGS     = '-Tbsp/boards/samr21_xpro/cmsis/linkerScripts/samr21g18a_sram.ld')
        env.Append(LINKFLAGS='-nostartfiles')
        env.Append(LINKFLAGS='-Wl,-Map,${TARGET.base}.map')
        env.Append(LINKFLAGS='-mcpu=cortex-m0plus')
        env.Append(LINKFLAGS='-mthumb')
        env.Append(LINKFLAGS='-g3')
        # object manipulation
        env.Replace(OBJCOPY='arm-none-eabi-objcopy')
        env.Replace(OBJDUMP='arm-none-eabi-objdump')
        # archiver
        env.Replace(AR='arm-none-eabi-ar')
        env.Append(ARFLAGS='')
        env.Replace(RANLIB='arm-none-eabi-ranlib')
        env.Append(RANLIBFLAGS='')
        # misc
        env.Replace(NM='arm-none-eabi-nm')
        env.Replace(SIZE='arm-none-eabi-size')

    elif env['board'] == 'nrf52840':

        # compiler (C)
        env.Replace(CC='arm-none-eabi-gcc')
        env.Append(CCFLAGS='-O0')
        env.Append(CCFLAGS='-Wall')
        env.Append(CCFLAGS='-Wa,-adhlns=${TARGET.base}.lst')
        env.Append(CCFLAGS='-c')
        env.Append(CCFLAGS='-fmessage-length=0')
        env.Append(CCFLAGS='-mcpu=cortex-m4')
        env.Append(CCFLAGS='-mthumb')
        env.Append(CCFLAGS='-g')
        env.Append(CCFLAGS='-std=gnu99')
        env.Append(CCFLAGS='-Wstrict-prototypes')
        env.Append(CCFLAGS='-ffunction-sections')
        env.Append(CCFLAGS='-fdata-sections')
        env.Append(CCFLAGS='-mfpu=fpv4-sp-d16')
        env.Append(CCFLAGS='-mfloat-abi=hard')
        env.Append(CCFLAGS='-D__FPU_PRESENT=1')
        env.Append(CCFLAGS='-DUSE_APP_CONFIG=1')
        env.Append(CCFLAGS='-DNRF52840_XXAA=1')  # set the CPU to nRF52840 (ARM Cortex M4f)
        if env['revision'] == "DK":
            env.Append(CCFLAGS='-DBOARD_PCA10056=1')  # set the board to be the nRF52840 Development Kit
            print "*** nrf52840-DK ***\n"
        elif env['revision'] == "DONGLE":
            env.Append(CCFLAGS='-DBOARD_PCA10059=1')  # set the board to be the nRF52840 Dongle
            env.Append(CCFLAGS='-DCONFIG_NFCT_PINS_AS_GPIOS=1')  # configure NFCT pins as GPIOs
            print "*** nrf52840-DONGLE ***\n"
        else:
            print "*** unknown ***\n"

        env.Append(
            CCFLAGS='-DCONFIG_GPIO_AS_PINRESET=1')  # just to be able to reset the board via the on-board reset pin

        # assembler
        env.Replace(AS='arm-none-eabi-as')
        env.Append(ASFLAGS='-g -gdwarf-2 -mcpu=cortex-m4 -mthumb -c -x assembler-with-cpp ')
        env.Append(ASFLAGS='-DNRF52840_XXAA=1')

        # linker
        env.Append(LINKFLAGS='-Lbsp/boards/nrf52840/sdk/modules/nrfx/mdk')
        env.Append(LINKFLAGS='-g -gdwarf-2 -mcpu=cortex-m4 -mthumb')

        # @todo: Decide which linker script to use
        if env['revision'] == "DK":
            env.Append(LINKFLAGS='-Tbsp/boards/nrf52840/nrf52840_xxaa.ld')
        elif env['revision'] == "DONGLE":
            env.Append(LINKFLAGS='-Tbsp/boards/nrf52840/nrf52840_xxaa_dongle.ld')
        # env.Append(LINKFLAGS     = '-Tbsp/boards/nrf52840/sdk/config/nrf52840/armgcc/generic_gcc_nrf52.ld')

        # env.Append(LINKFLAGS     = '--strip-debug')

        env.Append(LINKFLAGS='-Xlinker --gc-sections -Xlinker')
        env.Append(LINKFLAGS='-Map=${TARGET.base}.map')

        env.Append(LINKFLAGS='-mfpu=fpv4-sp-d16 -mfloat-abi=hard --specs=nosys.specs')

        # --specs=nano.specs
        env.Append(LINKFLAGS='-Wl,--start-group -lgcc -lc -lg -lm -lnosys -Wl,--end-group')
        env.Append(
            LINKFLAGS=os.path.join('build', env['board'] + '_armgcc', 'bsp', 'boards', env['board'], 'sdk', 'modules',
                                   'nrfx', 'mdk', 'gcc_startup_nrf52840.o'))

    elif env['board'] == 'scum':

        # compiler (C)
        env.Replace(CC='arm-none-eabi-gcc')
        env.Append(CCFLAGS='-O3')
        env.Append(CCFLAGS='-Wall')
        env.Append(CCFLAGS='-mcpu=cortex-m0')
        env.Append(CCFLAGS='-mthumb')
        env.Append(CCFLAGS='-g')
        env.Append(CCFLAGS='-Ibsp/boards/scum')
        env.Append(CCFLAGS='-ffunction-sections')
        env.Append(CCFLAGS='-fdata-sections')
        # assembler
        env.Replace(AS='arm-none-eabi-as')
        env.Append(ASFLAGS='-ggdb -g3 -mcpu=cortex-m0 -mlittle-endian -mthumb')
        # linker
        env.Append(LINKFLAGS='-Tbsp/boards/scum/scum_linker.ld')
        env.Append(LINKFLAGS='-nostartfiles')
        env.Append(LINKFLAGS='-Wl,--gc-sections')
        env.Append(LINKFLAGS='-Wl,-Map,${TARGET.base}.map')
        env.Append(LINKFLAGS='-specs=nosys.specs')
        env.Append(LINKFLAGS='-D__STACK_SIZE=0x800')
        env.Append(LINKFLAGS='-D__HEAP_SIZE=0x400')
        # object manipulation
        env.Replace(OBJCOPY='arm-none-eabi-objcopy')
        env.Replace(OBJDUMP='arm-none-eabi-objdump')
        # archiver
        env.Replace(AR='arm-none-eabi-ar')
        env.Append(ARFLAGS='')
        env.Replace(RANLIB='arm-none-eabi-ranlib')
        env.Append(RANLIBFLAGS='')
        # misc
        env.Replace(NM='arm-none-eabi-nm')
        env.Replace(SIZE='arm-none-eabi-size')

    else:
        print c.Fore.RED + 'Unexpected board={0}'.format(env['board']) + c.Fore.RESET
        Exit(-1)

    # converts ELF to iHex
    elf2iHexFunc = Builder(
        action='arm-none-eabi-objcopy -O ihex $SOURCE $TARGET',
        suffix='.ihex',
    )
    env.Append(BUILDERS={'Elf2iHex': elf2iHexFunc})

    # convert ELF to bin
    elf2BinFunc = Builder(
        action='arm-none-eabi-objcopy -O binary $SOURCE $TARGET',
        suffix='.bin',
    )
    env.Append(BUILDERS={'Elf2iBin': elf2BinFunc})

    # print sizes
    printSizeFunc = Builder(
        action='arm-none-eabi-size --format=berkeley -d --totals $SOURCE',
        suffix='.phonysize',
    )
    env.Append(BUILDERS={'PrintSize': printSizeFunc})

elif env['toolchain'] == 'gcc':

    # compiler (C)
    env.Append(CCFLAGS='-Wall')

    if env['board'] not in ['python']:
        print c.Fore.Red + 'Toolchain {0} can not be used for board {1}'.format(env['toolchain'],
                                                                                env['board']) + c.Fore.RESET
        Exit(-1)

    if env['board'] in ['python']:
        env.Append(CPPDEFINES='OPENSIM')

    if os.name != 'nt':
        if env['simhost'].endswith('linux'):
            # enabling shared library to be reallocated 
            env.Append(CCFLAGS='-fPIC')
            if not sys.platform.startswith('darwin'):  # line added per FW-230
                env.Append(SHLINKFLAGS='-Wl,-Bsymbolic-functions')  # per FW-176
                env.Append(SHCFLAGS='-Wl,-Bsymbolic-functions')  # per FW-176

            if platform.architecture()[0] == '64bit' and env['simhost'] == 'x86-linux':
                # Cross-compile x86 Linux target from 64-bit host. Also see projects/python/SConscript.env.
                env.Append(CCFLAGS='-m32')
                # Resolves a conflict between Python's pyconfig.h, which uses '200112'L, and libc's features.h, which
                # uses '200809L'.
                env.Append(CPPDEFINES=[('_POSIX_C_SOURCE', '200112L')])
                env.Append(SHLINKFLAGS='-m32')

        if env['simhost'] == 'amd64-windows':
            # Used by Python includes
            env.Append(CPPDEFINES='MS_WIN64')

    # converts ELF to iHex
    env.Append(BUILDERS={'Elf2iHex': dummyFunc})

    # convert ELF to bin
    env.Append(BUILDERS={'Elf2iBin': dummyFunc})

    # print sizes
    env.Append(BUILDERS={'PrintSize': dummyFunc})

else:
    print c.Fore.RED + 'Unexpected toolchain {0}'.format(env['toolchain']) + c.Fore.RESET
    Exit(-1)


# ============================ upload over JTAG ================================

def jtag_upload_func(location):
    if env['toolchain'] == 'armgcc':
        if env['board'] in ['iot-lab_M3', 'iot-lab_A8-M3']:
            return Builder(
                action=os.path.join('bsp', 'boards', env['board'], 'tools', 'flash.sh') + " $SOURCE",
                suffix='.phonyupload',
                src_suffix='.ihex',
            )
        if env['board'] == 'nrf52840':
            if env['revision'] == 'DK':
                return Builder(
                    action=os.path.join('bsp', 'boards', env['board'], 'tools', 'flash.sh') + " $SOURCE",
                    suffix='.phonyupload',
                    src_suffix='.elf',
                )
            else:
                print c.Fore.RED + 'Only nRF52840 DK flashing is supported at the moment.' + c.Fore.RESET
                Exit(-1)
    else:
        if env['fet_version'] == 2:
            # MSP-FET430uif is running v2 Firmware
            return Builder(
                action='mspdebug -d {0} -j uif "prog $SOURCE"'.format(location),
                suffix='.phonyupload',
                src_suffix='.ihex',
            )
        elif env['fet_version'] == 3:
            # MSP-FET430uif is running v2 Firmware
            if location in 'default':
                return Builder(
                    action='mspdebug tilib "prog $SOURCE"',
                    suffix='.phonyupload',
                    src_suffix='.ihex',
                )
            else:
                return Builder(
                    action='mspdebug tilib -d {0} "prog $SOURCE"'.format(location),
                    suffix='.phonyupload',
                    src_suffix='.ihex',
                )
        else:
            print c.Fore.RED + 'fet_version={0} unsupported.'.format(fet_version) + c.Fore.RESET
            Exit(-1)


if env['jtag']:
    env.Append(BUILDERS={'JtagUpload': jtag_upload_func(env['jtag'])})

# ============================ bootload ========================================

"""
Pass a list, a range or all ports to do the bootloading
  list  -> /dev/ttyUSB0,ttyUSB1,/dev/ttyUSB2
  range -> /dev/ttyUSB[0-2] = /dev/ttyUSB0,ttyUSB1,/dev/ttyUSB2
  all   -> /dev/ttyUSBX = /dev/ttyUSB0,ttyUSB1,/dev/ttyUSB2
"""


def expand_bootload_port_list(ports):
    # Process only when there is a single port
    if len(ports) == 1:
        port = ports[0]
        ports = []
        last_char = port[-1:]
        base_dir = os.path.dirname(port)

        # /dev/ttyUSBX means bootload all ttyUSB ports
        if last_char == "X":
            base_file = os.path.basename(port[:-1])
            ports = sorted(glob.glob(os.path.join(base_dir, base_file) + "*"))

        # /dev/ttyUSB[1-2] means bootload a range of ttyUSB ports
        elif last_char == "]":
            base_file = os.path.basename(port.split('[')[0])
            first, last = sorted(map(int, ((port.split('['))[1].split(']')[0]).split('-')))

            # For all elements in range
            for i in range(first, last + 1):
                p = os.path.join(base_dir, base_file + str(i))
                ports.append(p)
        else:
            ports = [port]

    # Check if new list is empty
    if not ports:
        print c.Fore.RED + "Bootload port expansion is empty or erroneous!" + c.Fore.RESET
        Exit(-1)

    return ports


class TelsosbBootloadThread(threading.Thread):
    def __init__(self, com_port, hex_file, counting_sem):
        # store params
        self.com_port = com_port
        self.hex_file = hex_file
        self.counting_sem = counting_sem

        # initialize parent class
        threading.Thread.__init__(self)
        self.name = 'TelsosbBootloadThread_{0}'.format(self.com_port)

    def run(self):
        print 'starting bootloading on {0}'.format(self.com_port)
        subprocess.call(
            PYTHON_PY + os.path.join('bootloader', 'telosb', 'bsl') + ' --telosb -c {0} -r -e -I -p "{1}"'.format(
                self.com_port, self.hex_file),
            shell=True
        )
        print 'done bootloading on {0}'.format(self.com_port)

        # indicate done
        self.counting_sem.release()


def telosb_bootload(target, source, env):
    bootload_threads = []
    counting_sem = threading.Semaphore(0)
    # create threads
    for com_port in env['bootload'].split(','):
        bootload_threads += [
            TelsosbBootloadThread(
                com_port=com_port,
                hex_file=source[0],
                counting_sem=counting_sem,
            )
        ]
    # start threads
    for t in bootload_threads:
        t.start()
    # wait for threads to finish
    for t in bootload_threads:
        counting_sem.acquire()


class OpenMoteCC2538BootloadThread(threading.Thread):
    def __init__(self, com_port, hex_file, counting_sem):
        # store params
        self.com_port = com_port
        self.hex_file = hex_file
        self.counting_sem = counting_sem

        # initialize parent class
        threading.Thread.__init__(self)
        self.name = 'OpenMoteCC2538BootloadThread_{0}'.format(self.com_port)

    def run(self):
        print 'starting bootloading on {0}'.format(self.com_port)
        subprocess.call(
            PYTHON_PY + os.path.join('bootloader', 'openmote-cc2538',
                                     'cc2538-bsl.py') + ' -e --bootloader-invert-lines -w -b 400000 -p {0} {1}'.format(
                self.com_port, self.hex_file),
            shell=True
        )
        print 'done bootloading on {0}'.format(self.com_port)

        # indicate done
        self.counting_sem.release()


def openmote_cc2538_bootload(target, source, env):
    bootload_threads = []
    counting_sem = threading.Semaphore(0)

    # Enumerate ports
    com_ports = env['bootload'].split(',')

    # Check com_ports to bootload
    com_ports = expand_bootload_port_list(com_ports)

    # create threads
    for com_port in com_ports:
        bootload_threads += [
            OpenMoteCC2538BootloadThread(
                com_port=com_port,
                # hex_file      = os.path.split(source[0].path)[1].split('.')[0]+'.bin',
                hex_file=source[0].path.split('.')[0] + '.ihex',
                counting_sem=counting_sem,
            )
        ]
    # start threads
    for t in bootload_threads:
        t.start()
    # wait for threads to finish
    for t in bootload_threads:
        counting_sem.acquire()


class OpentestbedBootloadThread(threading.Thread):
    def __init__(self, mote, hex_file, counting_sem):

        # store params
        self.mote = mote
        self.hex_file = hex_file
        self.counting_sem = counting_sem

        # initialize parent class
        threading.Thread.__init__(self)
        self.name = 'OpenMoteCC2538BootloadThread_{0}'.format(self.mote)

    def run(self):
        print 'starting bootloading on {0}'.format(self.mote)
        if self.mote == 'opentestbed':
            target = 'all'
        else:
            target = self.mote
        subprocess.call(
            PYTHON_PY + os.path.join('bootloader', 'openmote-cc2538', 'ot_program.py') + ' -a {0} {1}'.format(target,
                                                                                                              self.hex_file),
            shell=True
        )
        print 'done bootloading on {0}'.format(self.mote)

        # indicate done
        self.counting_sem.release()


def opentestbed_bootload(target, source, env):
    bootload_threads = []
    counting_sem = threading.Semaphore(0)

    # Enumerate ports
    motes = env['bootload'].split(',')

    # create threads
    for mote in motes:
        bootload_threads += [
            OpentestbedBootloadThread(
                mote=mote,
                hex_file=source[0].path.split('.')[0] + '.ihex',
                counting_sem=counting_sem,
            )
        ]
    # start threads
    for t in bootload_threads:
        t.start()
    # wait for threads to finish
    for t in bootload_threads:
        counting_sem.acquire()


class OpenMoteStmBootloadThread(threading.Thread):
    def __init__(self, com_port, binary_file, counting_sem):
        # store params
        self.com_port = com_port
        self.binary_file = binary_file
        self.counting_sem = counting_sem

        # initialize parent class
        threading.Thread.__init__(self)
        self.name = 'OpenMoteStmBootloadThread_{0}'.format(self.com_port)

    def run(self):
        print 'starting bootloading on {0}'.format(self.com_port)
        subprocess.call(
            PYTHON_PY + os.path.join('bootloader', 'openmotestm',
                                     'bin.py' + ' -p {0} {1}'.format(self.com_port, self.binary_file)),
            shell=True
        )
        print 'done bootloading on {0}'.format(self.com_port)

        # indicate done
        self.counting_sem.release()


def openmotestm_bootload(target, source, env):
    bootload_threads = []
    counting_sem = threading.Semaphore(0)
    # create threads
    for com_port in env['bootload'].split(','):
        bootload_threads += [
            OpenMoteStmBootloadThread(
                com_port=com_port,
                binary_file=source[0].path.split('.')[0] + '.bin',
                counting_sem=counting_sem,
            )
        ]
    # start threads
    for t in bootload_threads:
        t.start()
    # wait for threads to finish
    for t in bootload_threads:
        counting_sem.acquire()


class IotLabM3BootloadThread(threading.Thread):
    def __init__(self, com_port, binary_file, counting_sem):
        # store params
        self.com_port = com_port
        self.binary_file = binary_file
        self.counting_sem = counting_sem

        # initialize parent class
        threading.Thread.__init__(self)
        self.name = 'IotLabM3BootloadThread_{0}'.format(self.com_port)

    def run(self):
        print 'starting bootloading on {0}'.format(self.com_port)
        subprocess.call(
            PYTHON_PY + os.path.join('bootloader', 'iot-lab_M3',
                                     'iotlab-m3-bsl.py' + ' -i {0} -p {1}'.format(self.binary_file, self.com_port)),
            shell=True
        )
        print 'done bootloading on {0}'.format(self.com_port)

        # indicate done
        self.counting_sem.release()


def iotlabm3_bootload(target, source, env):
    bootload_threads = []
    counting_sem = threading.Semaphore(0)
    # create threads
    for com_port in env['bootload'].split(','):
        if os.name == 'nt':
            suffix = '.exe'
        else:
            suffix = ''
        bootload_threads += [
            IotLabM3BootloadThread(
                com_port=com_port,
                binary_file=source[0].path.split('.')[0] + suffix,
                counting_sem=counting_sem,
            )
        ]
    # start threads
    for t in bootload_threads:
        t.start()
    # wait for threads to finish
    for t in bootload_threads:
        counting_sem.acquire()


class ScumBootloadThread(threading.Thread):
    def __init__(self, com_port, binary_file, counting_sem):
        # store params
        self.com_port = com_port
        self.binary_file = binary_file
        self.counting_sem = counting_sem

        # initialize parent class
        threading.Thread.__init__(self)
        self.name = 'ScumBootloadThread{0}'.format(self.com_port)

    def run(self):
        print 'starting bootloading on {0}'.format(self.com_port)
        subprocess.call(
            PYTHON_PY + os.path.join('bootloader', 'scum',
                                     'scum_bootloader.py' + ' -p {0} {1}'.format(self.com_port, self.binary_file)),
            shell=True
        )
        print 'done bootloading on {0}'.format(self.com_port)

        # indicate done
        self.counting_sem.release()


def scum_bootload(target, source, env):
    bootload_threads = []
    counting_sem = threading.Semaphore(0)
    # create threads
    for com_port in env['bootload'].split(','):
        if os.name == 'nt':
            suffix = '.bin'
        else:
            suffix = ''
        bootload_threads += [
            ScumBootloadThread(
                com_port=com_port,
                binary_file=source[0].path.split('.')[0] + suffix,
                counting_sem=counting_sem,
            )
        ]
    # start threads
    for t in bootload_threads:
        t.start()
    # wait for threads to finish
    for t in bootload_threads:
        counting_sem.acquire()


# bootload
def bootload_func():
    if env['board'] == 'telosb':
        return Builder(
            action=telosb_bootload,
            suffix='.phonyupload',
            src_suffix='.ihex',
        )
    elif env['board'] in ['openmote-cc2538', 'openmote-b', 'openmote-b-24ghz', 'openmote-b-subghz']:
        if 'testbed' in env['bootload'] or len(env['bootload'].split(',')[0].split('-')) == 8:
            return Builder(
                action=opentestbed_bootload,
                suffix='.phonyupload',
                src_suffix='.ihex',
            )
        else:
            return Builder(
                action=openmote_cc2538_bootload,
                suffix='.phonyupload',
                src_suffix='.bin',
            )
    elif env['board'] == 'iot-lab_M3':
        return Builder(
            action=iotlabm3_bootload,
            suffix='.phonyupload',
            src_suffix=''
        )
    elif env['board'] == 'openmotestm':
        return Builder(
            action=openmotestm_bootload,
            suffix='.phonyupload',
            src_suffix='.bin'
        )
    elif env['board'] == 'scum':
        return Builder(
            action=scum_bootload,
            suffix='.phonyupload',
            src_suffix='.bin'
        )
    else:
        print c.Fore.RESET + 'bootloading on board={0} unsupported.'.format(env['board']) + c.Fore.RESET
        Exit(-1)


if env['bootload']:
    env.Append(BUILDERS={'Bootload': bootload_func()})


# PostBuildExtras is a method called after a program (not a library) is built.
# You can any addition step in this function, such as converting the binary
# or copying it somewhere.
def extras(env, source):
    return_val = []
    return_val += [env.PrintSize(source=source)]
    return_val += [env.Elf2iHex(source=source)]
    if env['board'] != 'nrf52840':
        return_val += [env.Elf2iBin(source=source)]
    if env['jtag']:
        if env['board'] == 'nrf52840' and env['revision'] == 'DK' and env['jtag'] == 'bflash':
            return_val += [env.JtagUpload(source)]
        else:
            return_val += [env.JtagUpload(env.Elf2iHex(source))]
    elif env['bootload']:
        return_val += [env.Bootload(env.Elf2iHex(source))]
    return return_val


env.AddMethod(extras, 'PostBuildExtras')


# ============================ helpers =========================================

def buildLibs(projectDir):
    # this order needed for mspgcc
    libs_dict = {
        '00std': [],
        '01bsp': ['libbsp'],
        '02drv': ['libkernel', 'libdrivers', 'libbsp'],
        '03oos': ['libopenstack', 'libopenapps', 'libopenweb', 'libkernel', 'libdrivers', 'libbsp'],
    }

    return_val = None
    for k, v in libs_dict.items():
        if projectDir.startswith(k):
            return_val = v

    assert return_val is not None

    return return_val


def buildIncludePath(project_dir, local_env):
    if project_dir.startswith('03oos_'):
        local_env.Append(
            CPPPATH=[
                os.path.join('#', 'inc'),
                os.path.join('#', 'kernel'),
                os.path.join('#', 'openstack'),
                os.path.join('#', 'openstack', '02a-MAClow'),
                os.path.join('#', 'openstack', '02b-MAChigh'),
                os.path.join('#', 'openstack', '03a-IPHC'),
                os.path.join('#', 'openstack', '03b-IPv6'),
                os.path.join('#', 'openstack', '04-TRAN'),
                os.path.join('#', 'openstack', 'cross-layers'),
                os.path.join('#', 'drivers', 'common'),
            ]
        )


def populateTargetGroup(localEnv, targetName):
    env['targets']['all'].append(targetName)
    for prefix in ['std', 'bsp', 'drv', 'oos']:
        if targetName.startswith(prefix):
            env['targets']['all_' + prefix].append(targetName)


def project_finder(localEnv):
    """
    This function is called from the following directories:
    - projects\common\
    - projects\<board>\
    """

    # list subdirectories

    if env['toolchain'] == 'iar-proj':
        # no VariantDir is used
        projects = os.getcwd()
    else:
        # VariantDir is used
        projects = os.path.join('..', '..', '..', '..', 'projects', os.path.split(os.getcwd())[-1])

    sub_dirs = [name for name in os.listdir(projects) if os.path.isdir(os.path.join(projects, name))]

    # parse dirs and build targets
    for project_dir in sub_dirs:

        src_dir = os.path.join(projects, project_dir)
        variant_dir = os.path.join(env['VARDIR'], 'projects', project_dir)

        added = False
        target_name = project_dir[2:]

        if '{0}.c'.format(project_dir) in os.listdir(os.path.join(projects, project_dir)) and \
                localEnv['toolchain'] != 'iar-proj' and localEnv['board'] != 'python':
            # "normal" case
            localEnv.VariantDir(src_dir=src_dir, variant_dir=variant_dir)

            target = project_dir
            source = [os.path.join(project_dir, '{0}.c'.format(project_dir))]
            libs = buildLibs(project_dir)

            buildIncludePath(project_dir, localEnv)

            # In Linux, you cannot have the same target name as the name of the
            # directory name.
            target = target + "_prog"

            exe = localEnv.Program(target=target, source=source, LIBS=libs)
            target_action = localEnv.PostBuildExtras(exe)

            Alias(target_name, [target_action])
            added = True

        elif '{0}.c'.format(project_dir) in os.listdir(os.path.join(projects, project_dir)) and \
                localEnv['board'] == 'python':
            # Python case

            # build the artifacts in a separate directory
            localEnv.VariantDir(src_dir=src_dir, variant_dir=variant_dir)

            # build both the application's and the Python module's main files
            sources_c = [
                os.path.join(project_dir, '{0}.c'.format(project_dir)),
                os.path.join(project_dir, 'openwsnmodule.c'),
            ]

            localEnv.Command(
                os.path.join(project_dir, 'openwsnmodule.c'),
                os.path.join('#', 'bsp', 'boards', 'python', 'openwsnmodule.c'),
                [Copy('$TARGET', '$SOURCE')]
            )

            # objectify those two files
            for s in sources_c:
                localEnv.Objectify(target=localEnv.ObjectifiedFilename(s), source=s)

            # prepare environment for this build
            if os.name != 'nt' and localEnv['simhost'].endswith('-windows'):
                # Cross-build handling -- find DLL, rather than hardcode version,
                # like 'python27.dll'
                path_names = sconsUtils.findPattern('python*.dll', localEnv['simhostpy'])
                if path_names:
                    path_name = path_names[0]
                else:
                    print c.Fore.RED + "Can't find python dll in provided simhostpy" + c.Fore.RESET
                    Exit(-1)

                # ':' means no prefix, like 'lib', for shared library name
                pysyslib = ':{0}'.format(os.path.basename(path_name))
                pylib_ext = '.pyd'
            else:
                pysyslib = 'python' + distutils.sysconfig.get_config_var('VERSION')
                pylib_ext = distutils.sysconfig.get_config_var('SO')

            target = target_name
            source = [localEnv.ObjectifiedFilename(s) for s in sources_c]
            libs = buildLibs(project_dir)
            libs += [[pysyslib]]

            buildIncludePath(project_dir, localEnv)

            # build a shared library (a Python extension module) rather than an exe

            targetAction = localEnv.SharedLibrary(
                target,
                source,
                LIBS=libs,
                SHLIBPREFIX='',
                SHLIBSUFFIX=pylib_ext,
            )

            Alias(target_name, [targetAction])
            added = True

        elif (
                ('{0}.ewp'.format(project_dir) in os.listdir(os.path.join(projects, project_dir))) and
                (localEnv['toolchain'] == 'iar-proj')
        ):
            # iar-proj case

            source = [os.path.join(project_dir, '{0}.ewp'.format(project_dir)), ]

            targetAction = localEnv.iarProjBuilder(
                source=source,
            )

            Alias(target_name, [targetAction])
            added = True

        if added:
            populateTargetGroup(localEnv, target_name)


env.AddMethod(project_finder, 'ProjectFinder')

# ============================ board ===========================================

# Get build environment from platform directory
buildEnv = env.SConscript(
    os.path.join('projects', env['board'], 'SConscript.env'),
    exports=['env'],
)

# inc
incDir = os.path.join('#', 'inc')
incVarDir = os.path.join(buildEnv['VARDIR'], 'inc')
buildEnv.SConscript(
    os.path.join(incDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=incVarDir,
)

# bsp
bspDir = os.path.join('#', 'bsp', 'boards')
bspVarDir = os.path.join(buildEnv['VARDIR'], 'bsp', 'boards')
buildEnv.Append(CPPPATH=[bspDir])
buildEnv.SConscript(
    os.path.join(bspDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=bspVarDir,
)
buildEnv.Clean('libbsp', Dir(bspVarDir).abspath)
buildEnv.Append(LIBPATH=[bspVarDir])

# kernelheader
kernelHDir = os.path.join('#', 'kernel')
kernelHVarDir = os.path.join(buildEnv['VARDIR'], 'kernel')
buildEnv.SConscript(
    os.path.join(kernelHDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=kernelHVarDir,
)

# kernel
kernelDir = os.path.join('#', 'kernel', 'openos')
kernelVarDir = os.path.join(buildEnv['VARDIR'], 'kernel', 'openos')
buildEnv.SConscript(
    os.path.join(kernelDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=kernelVarDir,
)
buildEnv.Clean('libkernel', Dir(kernelVarDir).abspath)
buildEnv.Append(LIBPATH=[kernelVarDir])

# drivers
driversDir = os.path.join('#', 'drivers')
driversVarDir = os.path.join(buildEnv['VARDIR'], 'drivers')
buildEnv.SConscript(
    os.path.join(driversDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=driversVarDir,
)
buildEnv.Clean('libdrivers', Dir(driversVarDir).abspath)
buildEnv.Append(LIBPATH=[driversVarDir])

# openstack
openstackDir = os.path.join('#', 'openstack')
openstackVarDir = os.path.join(buildEnv['VARDIR'], 'openstack')
buildEnv.SConscript(
    os.path.join(openstackDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=openstackVarDir,
)
buildEnv.Clean('libopenstack', Dir(openstackVarDir).abspath)
buildEnv.Append(LIBPATH=[openstackVarDir])

# openweb
openwebDir = os.path.join('#', 'openweb')
openwebVarDir = os.path.join(buildEnv['VARDIR'], 'openweb')
buildEnv.SConscript(
    os.path.join(openwebDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=openwebVarDir,
)
buildEnv.Clean('libopenweb', Dir(openwebVarDir).abspath)
buildEnv.Append(LIBPATH=[openwebVarDir])

# openapps
openappsDir = os.path.join('#', 'openapps')
openappsVarDir = os.path.join(buildEnv['VARDIR'], 'openapps')
buildEnv.SConscript(
    os.path.join(openappsDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=openappsVarDir,
)
buildEnv.Clean('libopenapps', Dir(openappsVarDir).abspath)
buildEnv.Append(LIBPATH=[openappsVarDir])

# projects
projectsDir = os.path.join('#', 'projects')
if env['toolchain'] == 'iar-proj':
    projectsVarDir = None
else:
    projectsVarDir = os.path.join(buildEnv['VARDIR'], 'projects')
buildEnv.SConscript(
    os.path.join(projectsDir, 'SConscript'),
    exports={'env': buildEnv},
    variant_dir=projectsVarDir,
)
