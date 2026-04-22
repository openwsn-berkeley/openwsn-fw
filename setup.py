import os
import sys
from glob import glob
from setuptools import setup, Extension

if sys.version_info.major < 3 or sys.version_info.minor < 6:
    raise Exception("Must be using Python 3.6 or higher")

here = os.path.abspath(os.path.dirname(__file__))


def src(*path_parts):
    return os.path.join(here, *path_parts)


sources = [
    # Python BSP board
    *glob(src('bsp', 'boards', 'python', '*.c')),
    # Kernel
    *glob(src('kernel', 'openos', '*.c')),
    # Drivers (common only — no hardware-specific drivers)
    *glob(src('drivers', 'common', '*.c')),
    *glob(src('drivers', 'common', 'crypto', '*.c')),
    # OpenStack
    *glob(src('openstack', '02a-MAClow', '*.c')),
    *glob(src('openstack', '02b-MAChigh', '*.c')),
    *glob(src('openstack', '03a-IPHC', '*.c')),
    *glob(src('openstack', '03b-IPv6', '*.c')),
    *glob(src('openstack', '04-TRAN', '*.c')),
    *glob(src('openstack', '04-TRAN', 'sock', '*.c')),
    *glob(src('openstack', 'cross-layers', '*.c')),
    src('openstack', 'openstack.c'),
    # OpenApps
    *glob(src('openapps', '*.c')),
    *glob(src('openapps', 'c6t', '*.c')),
    *glob(src('openapps', 'cexample', '*.c')),
    *glob(src('openapps', 'cinfo', '*.c')),
    *glob(src('openapps', 'cjoin', '*.c')),
    *glob(src('openapps', 'cled', '*.c')),
    *glob(src('openapps', 'cstorm', '*.c')),
    *glob(src('openapps', 'cwellknown', '*.c')),
    *glob(src('openapps', 'rrt', '*.c')),
    *glob(src('openapps', 'uecho', '*.c')),
    *glob(src('openapps', 'uexpiration', '*.c')),
    *glob(src('openapps', 'uexpiration_monitor', '*.c')),
    *glob(src('openapps', 'uinject', '*.c')),
    *glob(src('openapps', 'userialbridge', '*.c')),
    # OpenWeb
    *glob(src('openweb', '*.c')),
    *glob(src('openweb', 'opencoap', '*.c')),
    # Main project entry point
    src('projects', 'common', '03oos_openwsn', '03oos_openwsn.c'),
]

include_dirs = [
    src('inc'),
    src('bsp', 'boards'),
    src('bsp', 'boards', 'python'),
    src('kernel'),
    src('kernel', 'openos'),
    src('drivers', 'common'),
    src('drivers', 'common', 'crypto'),
    src('openstack'),
    src('openstack', '02a-MAClow'),
    src('openstack', '02b-MAChigh'),
    src('openstack', '03a-IPHC'),
    src('openstack', '03b-IPv6'),
    src('openstack', '04-TRAN'),
    src('openstack', '04-TRAN', 'sock'),
    src('openstack', 'cross-layers'),
    src('openapps'),
    src('openapps', 'c6t'),
    src('openapps', 'cexample'),
    src('openapps', 'cinfo'),
    src('openapps', 'cjoin'),
    src('openapps', 'cled'),
    src('openapps', 'cstorm'),
    src('openapps', 'cwellknown'),
    src('openapps', 'rrt'),
    src('openapps', 'uecho'),
    src('openapps', 'uexpiration'),
    src('openapps', 'uexpiration_monitor'),
    src('openapps', 'uinject'),
    src('openapps', 'userialbridge'),
    src('openweb'),
    src('openweb', 'opencoap'),
    src('projects', 'common', '03oos_openwsn'),
]

openmote = Extension(
    'openmote',
    sources=sources,
    include_dirs=include_dirs,
    define_macros=[
        ('PYTHON_BOARD', '1'),
        ('OPENWSN_COAP_C', '1'),
        ('OPENWSN_UDP_C', '1'),
        ('OPENWSN_ICMPV6ECHO_C', '0'),
    ],
    extra_compile_args=[] if sys.platform == 'win32' else ['-Wno-implicit-function-declaration'],
)

setup(
    name='openmote',
    version='1.0',
    python_requires='>=3.6',
    description='OpenWSN firmware compiled as a Python extension',
    author='Timothy Claeys',
    author_email='timothy.claeys@gmail.com',
    ext_modules=[openmote],
    zip_safe=False,
)
