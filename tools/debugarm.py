#!/usr/bin/python

import os
import subprocess
import sys
import getopt
import time

from distutils.spawn import find_executable

class DebugLauncher(): 

    def __init__(self):
        
        self.current_dir = "."
        self.home_dir = ".."
        
        self.platform = "OpenMote-CC2538"
        self.compiler = "armgcc"
        self.target = "03oos_openwsn_prog"
        
        self.current_dir = os.path.realpath(__file__)
        d, f = os.path.split(self.current_dir)
        home = os.path.join(d, self.home_dir)
        os.chdir(home)
        self.current_dir = os.path.realpath(home)
        
        self.cmd_dir = os.path.join(self.current_dir, 'projects', self.platform, self.platform + ".gdb")
        self.exec_file = os.path.join(self.current_dir, 'build', self.platform+"_"+self.compiler, 'projects','common',self.target)
                  
        self.gdbclient_bin = "/usr/bin/arm-none-eabi-gdb"
        self.gdbclient_params = ["--batch", "--command="+self.cmd_dir, self.exec_file]
        

        self.debugger_bin = "/usr/bin/nemiver"
        self.debugger_params = ["--remote=localhost:2331", "--gdb-binary="+self.gdbclient_bin, self.exec_file]
        
                     
        #arm-none-eabi-gdb --batch --command=/home/xvilajosana/Development/workspaces/workspace_qt_openwsn/openwsn-fw/projects/OpenMote-CC2538/cc2538_gdb.gdb /home/xvilajosana/Development/workspaces/workspace_qt_openwsn/openwsn-fw/build/OpenMote-CC2538_armgcc/projects/common/03oos_openwsn_prog
        #nemiver --remote=localhost:2331 --gdb-binary=/usr/bin/arm-none-eabi-gdb /home/xvilajosana/Development/workspaces/workspace_qt_openwsn/openwsn-fw/build/OpenMote-CC2538_armgcc/projects/common/03oos_openwsn_prog

         
    #Run gdb client
    def gdbclient_run(self):
                   
        command = self.gdbclient_bin+ " " + " ".join(c for c in self.gdbclient_params)
        print command
        if (find_executable(self.gdbclient_bin)):
            print("Running GDB Client..."),
            proc = subprocess.Popen(command, shell=True, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            #proc = subprocess.Popen(qt, shell=True, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print proc
            print("ok!")
        else:
            print("Error: GDB client not found!")
          
   
    #Run gdb client
    def debugger_run(self):
        #qt = [self.debugger_bin] + self.debugger_params
        command = self.debugger_bin+ " " + " ".join(c for c in self.debugger_params)

        print command        
        if (find_executable(self.debugger_bin)):
            print("Running Debugger Client...")
            try:
                print [self.debugger_bin]+self.debugger_params
                
                proc = subprocess.call([self.debugger_bin]+self.debugger_params)
            except:
                print("Unexpected error:", sys.exc_info()[0])
                raise
            #proc = subprocess.Popen(command, shell=True, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            
            print("ok!")
        else:
            print("Error: Debugger not found!")
            
            
   
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
               print 'debugarm.py '
               sys.exit()
            elif opt in ("-b", "--board"):
               platform = arg
         
        print 'Board is "', self.platform
        
        
    def initialize(self,pt,cp,tg):
        #self.platform=pt
        self.platform=pt
        self.compiler=cp
        self.target=tg
        
        self.gdbclient_run()
        time.sleep(1)
        self.debugger_run()
     
    # Main function
    def start(self,args):
        
        self.parseParams(args)
        self.initialize(self.platform, self.compiler, self.target)
      
if __name__ == "__main__":
    dbg = DebugLauncher()
    dbg.start(sys.argv[1:])
