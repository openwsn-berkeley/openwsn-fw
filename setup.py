import glob
from distutils.core import setup, Extension

openwsnmodule = Extension(
    'openwsn',
    sources        = \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/bsp/boards/python/*.c')           + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/drivers/common/*.c')              + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/kernel/openos/*.c')               + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/*.c')                     + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/02.5-MPLS/*.c')           + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/02a-MAClow/*.c')          + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/02b-MAChigh/*.c')         + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/03a-IPHC/*.c')            + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/03b-IPv6/*.c')            + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/04-TRAN/*.c')             + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/*.c')              + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/layerdebug/*.c')   + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/ohlone/*.c')       + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rex/*.c')          + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rinfo/*.c')        + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rleds/*.c')        + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rreg/*.c')         + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rwellknown/*.c')   + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/tcpecho/*.c')      + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/tcpinject/*.c')    + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/tcpprint/*.c')     + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udpecho/*.c')      + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udpinject/*.c')    + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udplatency/*.c')   + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udpprint/*.c')     + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udprand/*.c')      + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/cross-layers/*.c')        + \
        glob.glob('C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/projects/common/03oos_openwsn/*.c'),
    include_dirs   = [
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/bsp/boards"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/bsp/boards/python"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/drivers/common"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/kernel/openos"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/02.5-MPLS"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/02a-MAClow"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/02b-MAChigh"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/03a-IPHC"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/03b-IPv6"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/04-TRAN"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/layerdebug"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/ohlone"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rex"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rinfo"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rleds"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rreg"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/rwellknown"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/tcpecho"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/tcpinject"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/tcpprint"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udpecho"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udpinject"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udplatency"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udpprint"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udprand"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/07-App/udpstorm"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/openwsn/cross-layers"',
        '"C:/Users/Thomas/Desktop/openwsn-fw/firmware/openos/projects/common/03oos_openwsn"',
    ],
)

setup (
    name           = 'openwsn',
    version        = '1.0',
    description    = 'OpenWSN mote',
    ext_modules    = [openwsnmodule]
)
