import glob
import logging
import os
import platform
import shutil
import subprocess

import setuptools
import sys
from setuptools import Extension, Command
from setuptools.command.build_ext import build_ext

try:
    import colorama as c

    colors = True
    c.init()
except ImportError:
    colors = False

here = os.path.abspath('.')

# Filename for the C extension module library
c_module_name = '_openmote'

enable_configure = False

if sys.version_info.major < 3:
    raise Exception("Must be using Python 3.6 or higher")

if sys.version_info.minor < 6:
    raise Exception("Must be using Python 3.6 or higher")

if not os.path.exists(os.path.join(os.path.abspath('.'), 'build')):
    os.mkdir(os.path.join(os.path.abspath('.'), 'build'))
    os.mkdir(os.path.join(os.path.abspath('.'), 'build', 'openwsn'))

    with open('build/openwsn/__init__.py', 'w') as f:
        f.write('from openmote import *')
else:
    if os.path.exists(os.path.join(os.path.abspath('.'), 'build')) and not os.path.exists(
            os.path.join(os.path.abspath('.'), 'build', 'openwsn')):
        os.mkdir(os.path.join(os.path.abspath('./build'), 'openwsn'))

    with open('build/openwsn/__init__.py', 'w') as f:
        f.write('from openmote import *')

# Command line flags forwarded to CMake (for debug purpose)
cmake_cmd_args = []
for f in sys.argv:
    if f.startswith('-D'):
        cmake_cmd_args.append(f)
    elif f.startswith('--configure'):
        enable_configure = True
        sys.argv.remove(f)

for f in cmake_cmd_args:
    sys.argv.remove(f)


def _get_env_variable(name, default='OFF'):
    if name not in os.environ.keys():
        return default
    return os.environ[name]


class CMakeExtension(Extension):
    def __init__(self, name, cmake_lists_dir='.', sources=None, **kwa):
        if sources is None:
            _sources = []
        else:
            _sources = sources
        Extension.__init__(self, name, sources=_sources, **kwa)
        self.cmake_lists_dir = os.path.abspath(cmake_lists_dir)


class CMakeBuild(build_ext):
    def build_extensions(self):
        # Ensure that CMake is present and working
        try:
            out = subprocess.check_output(['cmake', '--version'])
            logging.info(out)
        except OSError:
            raise RuntimeError('Cannot find CMake executable')

        for ext in self.extensions:

            ext_dir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
            cfg = 'Debug'

            cmake_args = [
                '-DCMAKE_BUILD_TYPE=%s' % cfg,
                # Ask CMake to place the resulting library in the directory containing the extension
                '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), ext_dir),
                # Other intermediate static libraries are placed in a temporary build directory instead
                '-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), self.build_temp),
            ]

            # We can handle some platform-specific settings at our discretion
            if platform.system() == 'Windows':
                plat = ('x64' if platform.architecture()[0] == '64bit' else 'Win32')

                # When we use CMAKE_BUILD_TYPE=DEBUG we need python3x_d.lib libraries for linking, but Windows often
                # doesn't include those debug libraries. To prevent issues, force build type to Release
                cfg = 'Release'

                cmake_args += [
                    # These options are likely to be needed under Windows
                    '-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE',
                    '-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), ext_dir),
                ]
                # Assuming that Visual Studio and MinGW are supported compilers
                if self.compiler.compiler_type == 'msvc':
                    cmake_args += [
                        '-DCMAKE_GENERATOR_PLATFORM=%s' % plat,
                    ]
                else:
                    cmake_args += [
                        '-G', 'MinGW Makefiles',
                    ]

            cmake_args += cmake_cmd_args

            if not os.path.exists(self.build_temp):
                os.makedirs(self.build_temp)

            # Config
            subprocess.check_call(
                ['cmake', ext.cmake_lists_dir, '-DBOARD:STRING=python',
                 '-DPROJECT:STRING=oos_openwsn'] + cmake_args,
                cwd=self.build_temp)

            if enable_configure:
                if platform.system() == 'Windows':
                    print('ON WINDOWS')
                    subprocess.check_call(['cmake-gui', ext.cmake_lists_dir], cwd=self.build_temp)
                else:
                    subprocess.check_call(['ccmake', ext.cmake_lists_dir], cwd=self.build_temp)

            # Build
            try:
                subprocess.check_call(['cmake', '--build', '.', '--config', cfg], cwd=self.build_temp)
            except subprocess.CalledProcessError:
                if colors:
                    print(c.Fore.RED + "Build process failed" + c.Fore.RESET)
                else:
                    print("Build process failed")
            else:
                if colors:
                    print(c.Fore.GREEN + "Build process succeeded" + c.Fore.RESET)
                else:
                    print("Build process succeeded")


class CleanCommand(Command):
    """Custom clean command to tidy up the project root."""
    CLEAN_FILES = './build ./dist ./*.pyc ./*.tgz ./*.egg-info'.split(' ')

    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        global here

        for path_spec in self.CLEAN_FILES:
            # Make paths absolute and relative to this path
            abs_paths = glob.glob(os.path.normpath(os.path.join(here, path_spec)))
            for path in [str(p) for p in abs_paths]:
                if not path.startswith(here):
                    # Die if path in CLEAN_FILES is absolute + outside this directory
                    raise ValueError("%s is not a path inside %s" % (path, here))
                print('removing %s' % os.path.relpath(path))
                shutil.rmtree(path)


setuptools.setup(name='openwsn',
                 packages=setuptools.find_packages(where=os.path.join('build', 'openwsn')),
                 version=1.0,
                 python_requires='>=3.6',
                 description='An instance of an OpenWSN mote',
                 author='Timothy Claeys',
                 author_email='timothy.claeys@gmail.com',
                 ext_modules=[CMakeExtension(c_module_name)],
                 cmdclass={
                     'build_ext': CMakeBuild,
                     'clean': CleanCommand
                 },
                 classifiers=[
                     "Programming Language :: Python :: 3.6+",
                     "License :: OSI Approved :: MIT License",
                     "Operating System :: OS Independent",
                 ],
                 zip_safe=False)
