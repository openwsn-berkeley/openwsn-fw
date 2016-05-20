#!/usr/bin/python

import os
import subprocess
import sys
import getopt

from distutils.spawn import find_executable

current_dir = "."
home_dir = ".."

ignore_folders = ['.git', 'build', 'tools', 'docs', 'site_scons']

src_extensions = ['.c', '.cpp', '.h', '.hpp', '.py', '.env']
hdr_extensions = ['.h', '.hpp']

platform_dir = ''
common_platform_dir = ''
board_dir = ''
bootloader_dir = ''
projects_dir = ''
common_projects_dir = ''
bootloader_platform = ''
projects_platform = ''
drivers_dir = ''
drivers_platform = ''
common_drivers_dir = ''
drivers_dir = ''
drivers_platform = ''
    


qtcreator_bin = "qtcreator"
qtcreator_project = "tools/qtcreator"
qtcreator_creator = "OpenWSN.creator"
qtcreator_files = "OpenWSN.files"
qtcreator_includes = "OpenWSN.includes"
qtcreator_default = ["-noload", "Welcome", "-noload", "QmlDesigner", "-noload", "QmlProfiler"]

platform = "OpenMote-CC2538"
# platform = "gina"
 
# Determines valid folders
def is_valid_folder(path):
    result = False

    if path not in ignore_folders:
        result = True

    return result

# Returns all folders in the current path
def get_all_folders(path):
    result = []
    folders = next(os.walk(path))

    for f in folders[1]:
        if is_valid_folder(f):
            f = os.path.abspath(f)
            result.append(f)
    return result

# Determines if it is a valid source file
def is_valid_source_file(path):
    name, extension = os.path.splitext(path)
    if extension in src_extensions:
        return True
    return False
    
# Determines if it is a valid header directory
def is_valid_header_dir(path):
    name, extension = os.path.splitext(path)
    if extension in hdr_extensions:
        return True
    return False

# Gets all files from a path
def get_all_files(path):
    global board_dir, bootloader_dir, projects_dir, common_platform_dir, platform_dir, bootloader_dir, bootloader_platform, projects_platform
    global common_drivers_dir, drivers_dir, drivers_platform
    
    result = []
    boardfolder = False
    bootloaderfolder = False
    projectsfolder = False
    driverfolder = False
               
    for path, subdirs, files in os.walk(path):
        if board_dir in path:
            boardfolder = True
        else:
            boardfolder = False
       
        if bootloader_dir in path:
            bootloaderfolder = True
        else:
            bootloaderfolder = False
        
        if projects_dir in path:
            projectsfolder = True
        else:
            projectsfolder = False
        
        if drivers_dir in path:
            driverfolder = True
        else:
            driverfolder = False
                
        for f in files:
            if is_valid_source_file(f):
                if (boardfolder and (platform_dir in path or path == common_platform_dir or path == board_dir)): 
                    r = os.path.join(path, f)
                    r = os.path.abspath(r)
                    result.append(r)
                # bootloader    
                if (bootloaderfolder and path == bootloader_platform):
                    r = os.path.join(path, f)
                    r = os.path.abspath(r)
                    result.append(r)
                
                if (projectsfolder and (projects_platform in path or common_projects_dir in path)):
                    r = os.path.join(path, f)
                    r = os.path.abspath(r)
                    result.append(r)
                    
                if (driverfolder and (drivers_platform in path or common_drivers_dir in path)):
                    r = os.path.join(path, f)
                    r = os.path.abspath(r)
                    result.append(r)
         
                if (not boardfolder and not bootloaderfolder and not projectsfolder and not driverfolder):
                    r = os.path.join(path, f)
                    r = os.path.abspath(r)
                    result.append(r)
                    
                   
    
    return result

# Gets source files from a path
def get_src_files(path):
    src_files = []
    
    dirs = get_all_folders(path)
    
    for d in dirs:
        files = get_all_files(d)
        for f in files:
            if (f):
                src_files.append(f)
    src_files.sort()
    return src_files

# Gets includes folders from a path
def get_inc_dirs(path):
    inc_dirs = []
    
    dirs = get_all_folders(path)

    for d in dirs:
        files = get_all_files(d)
        for f in files:
            if (f):
                d, f = os.path.split(f)
                if d not in inc_dirs:
                    inc_dirs.append(d)
    inc_dirs.sort()
    return inc_dirs

# Writes output to file
def write_qtcreator(file_name, inc_dirs):
    fn = open(file_name, 'w')
    for inc_dir in inc_dirs:
        fn.write(inc_dir + "\n")
    fn.close()

# Prepares the QtCreator files
def qtcreator_prepare():
    global current_dir
    
    print("Preparing QtCreator...")
    inc_dirs = get_inc_dirs(current_dir)
    src_files = get_src_files(current_dir)
    
    path = os.path.join(qtcreator_project, qtcreator_files)
    write_qtcreator(path, src_files)    
    
    path = os.path.join(qtcreator_project, qtcreator_includes)
    write_qtcreator(path, inc_dirs)

    print("ok!")

# Runs QtCreator
def qtcreator_run():
    qt = [qtcreator_bin, qtcreator_project] + qtcreator_default
    
    if (find_executable(qtcreator_bin)):
        print("Running QtCreator..."),
        proc = subprocess.Popen(qt, shell=False, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print("ok!")
    else:
        print("Error: QtCreator not found!")

# Go to the home project directory
def change_dir():
    global current_dir, board_dir, platform_dir, common_platform_dir, bootloader_dir, projects_dir, common_projects_dir, bootloader_platform, projects_platform
    global common_drivers_dir, drivers_dir, drivers_platform
    
    current_dir = os.path.realpath(__file__)
    d, f = os.path.split(current_dir)
    home = os.path.join(d, home_dir)
    os.chdir(home)
    current_dir = os.path.realpath(home)
    
    board_dir = os.path.join(current_dir, 'bsp', 'boards')
    platform_dir = os.path.join(board_dir, platform)
    common_platform_dir = os.path.join(board_dir, 'common')
    
    bootloader_platform = os.path.join(current_dir, 'bootloader', platform)
    bootloader_dir = os.path.join(current_dir, 'bootloader')
    
    projects_dir = os.path.join(current_dir, 'projects')
    projects_platform = os.path.join(current_dir, 'projects', platform)
    common_projects_dir = os.path.join(current_dir, 'projects', 'common')
    
    common_drivers_dir = os.path.join(current_dir, 'drivers', 'common')
    drivers_dir = os.path.join(current_dir, 'drivers')
    drivers_platform = os.path.join(current_dir, 'drivers', platform)
    

def parseParams(args):
    global platform
    try:
        opts, args = getopt.getopt(args, "b:", ["board="])
    except getopt.GetoptError as err:
        print err
        print 'Wrong params. Use: qtcreator.py -b <board name>'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
           print 'qtcreator.py -b <board name>'
           sys.exit()
        elif opt in ("-b", "--board"):
           platform = arg
     
    print 'Board is "', platform
    

# Main function
def main(args):
    
    parseParams(args)
    
    change_dir()
   
    qtcreator_prepare()
    qtcreator_run()

if __name__ == "__main__":
    main(sys.argv[1:])
