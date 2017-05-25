#!/usr/bin/python

import os
import subprocess
import sys
import getopt

from distutils.spawn import find_executable

class QtCreatorManager(): 

    def __init__(self):
        
        self.current_dir = "."
        self.home_dir = ".."
        
        self.ignore_folders = ['.git', 'build', 'tools', 'docs', 'site_scons']
        
        self.src_extensions = ['.c', '.cpp', '.h', '.hpp', '.py', '.env']
        self.hdr_extensions = ['.h', '.hpp']
        
        self.platform_dir = ''
        self.common_platform_dir = ''
        self.board_dir = ''
        self.bootloader_dir = ''
        self.projects_dir = ''
        self.common_projects_dir = ''
        self.bootloader_platform = ''
        self.projects_platform = ''
        self.drivers_dir = ''
        self.drivers_platform = ''
        self.common_drivers_dir = ''
        self.drivers_dir = ''
        self.drivers_platform = ''
            
        self.qtcreator_bin = "qtcreator"
        self.qtcreator_project = "tools/qtcreator"
        self.qtcreator_creator = "OpenWSN.creator"
        self.qtcreator_files = "OpenWSN.files"
        self.qtcreator_includes = "OpenWSN.includes"
        self.qtcreator_default = ["-noload", "Welcome", "-noload", "QmlDesigner", "-noload", "QmlProfiler"]
        
        self.platform = "OpenMote-CC2538"
        
    
     
    # Determines valid folders
    def is_valid_folder(self,path):
        result = False
    
        if path not in self.ignore_folders:
            result = True
    
        return result
    
    # Returns all folders in the current path
    def get_all_folders(self,path):
        result = []
        folders = next(os.walk(path))
    
        for f in folders[1]:
            if self.is_valid_folder(f):
                f = os.path.abspath(f)
                result.append(f)
        return result
    
    # Determines if it is a valid source file
    def is_valid_source_file(self,path):
        name, extension = os.path.splitext(path)
        if extension in self.src_extensions:
            return True
        return False
        
    # Determines if it is a valid header directory
    def is_valid_header_dir(self,path):
        name, extension = os.path.splitext(path)
        if extension in hdr_extensions:
            return True
        return False
    
    # Gets all files from a path
    def get_all_files(self,path):
        
        result = []
        boardfolder = False
        bootloaderfolder = False
        projectsfolder = False
        driverfolder = False
                   
        for path, subdirs, files in os.walk(path):
            if self.board_dir in path:
                boardfolder = True
            else:
                boardfolder = False
           
            if self.bootloader_dir in path:
                bootloaderfolder = True
            else:
                bootloaderfolder = False
            
            if self.projects_dir in path:
                projectsfolder = True
            else:
                projectsfolder = False
            
            if self.drivers_dir in path:
                driverfolder = True
            else:
                driverfolder = False
                    
            for f in files:
                if self.is_valid_source_file(f):
                    
                    if ((boardfolder and (self.platform_dir in path or path == self.common_platform_dir or path == self.board_dir))
                        or ((bootloaderfolder and path == self.bootloader_platform))
                        or (projectsfolder and (self.projects_platform in path or self.common_projects_dir in path))
                        or (driverfolder and (self.drivers_platform in path or self.common_drivers_dir in path))):
                         
                        r = os.path.join(path, f)
                        r = os.path.abspath(r)
                        result.append(r)
                  
                    if (not boardfolder and not bootloaderfolder and not projectsfolder and not driverfolder):
                        r = os.path.join(path, f)
                        r = os.path.abspath(r)
                        result.append(r)
                        
        return result
    
    # Gets source files from a path
    def get_src_files(self,path):
        src_files = []
        
        dirs = self.get_all_folders(path)
        
        for d in dirs:
            files = self.get_all_files(d)
            for f in files:
                if (f):
                    src_files.append(f)
        src_files.sort()
        return src_files
    
    # Gets includes folders from a path
    def get_inc_dirs(self,path):
        inc_dirs = []
        
        dirs = self.get_all_folders(path)
    
        for d in dirs:
            files = self.get_all_files(d)
            for f in files:
                if (f):
                    d, f = os.path.split(f)
                    if d not in inc_dirs:
                        inc_dirs.append(d)
        inc_dirs.sort()
        return inc_dirs
    
    # Writes output to file
    def write_qtcreator(self,file_name, inc_dirs):
        fn = open(file_name, 'w')
        for inc_dir in inc_dirs:
            fn.write(inc_dir + "\n")
        fn.close()
    
    # Prepares the QtCreator files
    def qtcreator_prepare(self):
        
        print("Preparing QtCreator...")
        inc_dirs = self.get_inc_dirs(self.current_dir)
        src_files = self.get_src_files(self.current_dir)
        
        path = os.path.join(self.qtcreator_project, self.qtcreator_files)
        self.write_qtcreator(path, src_files)    
        
        path = os.path.join(self.qtcreator_project, self.qtcreator_includes)
        self.write_qtcreator(path, inc_dirs)
    
        print("ok!")
    
    # Runs QtCreator
    def qtcreator_run(self):
        qt = [self.qtcreator_bin, self.qtcreator_project] + self.qtcreator_default
        
        if (find_executable(self.qtcreator_bin)):
            print("Running QtCreator..."),
            proc = subprocess.Popen(qt, shell=False, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print("ok!")
        else:
            print("Error: QtCreator not found!")
    
    # Go to the home project directory
    def change_dir(self):
        
        self.current_dir = os.path.realpath(__file__)
        d, f = os.path.split(self.current_dir)
        home = os.path.join(d, self.home_dir)
        os.chdir(home)
        self.current_dir = os.path.realpath(home)
        
        self.board_dir = os.path.join(self.current_dir, 'bsp', 'boards')
        self.platform_dir = os.path.join(self.board_dir, self.platform)
        self.common_platform_dir = os.path.join(self.board_dir, 'common')
        
        self.bootloader_platform = os.path.join(self.current_dir, 'bootloader', self.platform)
        self.bootloader_dir = os.path.join(self.current_dir, 'bootloader')
        
        self.projects_dir = os.path.join(self.current_dir, 'projects')
        self.projects_platform = os.path.join(self.current_dir, 'projects', self.platform)
        self.common_projects_dir = os.path.join(self.current_dir, 'projects', 'common')
        
        self.common_drivers_dir = os.path.join(self.current_dir, 'drivers', 'common')
        self.drivers_dir = os.path.join(self.current_dir, 'drivers')
        self.drivers_platform = os.path.join(self.current_dir, 'drivers', self.platform)
        
    
    def parseParams(self,args):
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
         
        print 'Board is "', self.platform
        
    def initialize(self,pt):
        self.platform=pt
        self.change_dir()
       
        self.qtcreator_prepare()
        self.qtcreator_run()
     
    # Main function
    def start(self,args):
        
        self.parseParams(args)
        
        self.change_dir()
       
        self.qtcreator_prepare()
        self.qtcreator_run()
    
if __name__ == "__main__":
    qtcreator = QtCreatorManager()
    qtcreator.start(sys.argv[1:])
