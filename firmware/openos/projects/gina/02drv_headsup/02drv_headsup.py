#!/usr/bin/python

import serial
import struct
import threading
import Tkinter
import math
import winsound
import time
import os
if os.name=='nt':
   import _winreg as winreg

class DataHandler(object):
    
    current_x = None
    current_y = None
    current_z = None
    
    perfect_x = None
    perfect_y = None
    perfect_z = None
    
    front_x   = None
    front_y   = None
    front_z   = None
    
    side_x    = None
    side_y    = None
    side_z    = None
    
    #======================== public ==========================================
    
    def handleData(self,rawData):
        if len(rawData)==10:
            (raw_x, raw_y, raw_z, _,_) = struct.unpack(">HHHHH",rawData)
            self.current_x  = self.rawToAccel(raw_x)
            self.current_y  = self.rawToAccel(raw_y)
            self.current_z  = self.rawToAccel(raw_z)
            self.lastUpdate = time.time()
        else:
            print "ERROR did not receive 10 bytes"
    
    def getData(self):
        if self.current_x and self.perfect_x and self.front_x and self.side_x:
            
            self._printDebug('')
            
            v_current   = (self.current_x,self.current_y,self.current_z)
            v_perfect   = (self.perfect_x,self.perfect_y,self.perfect_z)
            v_front     = (self.front_x,  self.front_y,  self.front_z)
            v_side      = (self.side_x,   self.side_y,   self.side_z)
            
            self._printDebug('v_current:   '+str(v_current))
            self._printDebug('v_perfect:   '+str(v_perfect))
            self._printDebug('v_front:     '+str(v_front))
            self._printDebug('v_side:      '+str(v_side))
            
            norm_front  = self._crossProduct(v_perfect,v_front)
            norm_side   = self._crossProduct(v_perfect,v_side)
            
            self._printDebug('norm_front:  '+str(norm_front))
            self._printDebug('norm_side:   '+str(norm_side))
            
            angle_front = self._anglePlane(v_current,norm_side)
            angle_side  = self._anglePlane(v_current,norm_front)
            angle_abs   = self._angleAbsolute(v_current,v_perfect)
            
            self._printDebug('angle_front: '+str(angle_front))
            self._printDebug('angle_side:  '+str(angle_side))
            self._printDebug('angle_abs:   '+str(angle_abs))
            
            return(angle_side,angle_front,angle_abs,self.lastUpdate)
        else:
            return None
    
    def calibratePerfect(self):
        self.perfect_x = self.current_x
        self.perfect_y = self.current_y
        self.perfect_z = self.current_z
    
    def calibrateFront(self):
        self.front_x   = self.current_x
        self.front_y   = self.current_y
        self.front_z   = self.current_z
    
    def calibrateSide(self):
        self.side_x    = self.current_x
        self.side_y    = self.current_y
        self.side_z    = self.current_z
        
    #======================== helpers =========================================
    
    def _crossProduct(self,v1,v2):
       (a1, a2, a3) = v1
       (b1, b2, b3) = v2
       
       return (a2*b3-b3*b2,
               a3*b1-b1*b3,
               a1*b2-b2*b1)
    
    def _normVector(self,v1,v2):
        return [a/(self._length(v1) * self._length(v2)) for a in self._crossProduct(v1,v2)]
    
    def _dotproduct(self,v1,v2):
        return float(sum((a*b) for a, b in zip(v1, v2)))
    
    def _length(self,v):
        return float(math.sqrt(self._dotproduct(v, v)))
    
    def _angleAbsolute(self,v1,v2):
        return math.acos(self._angleInternal(v1,v2))
    
    def _anglePlane(self,v1,v2):
        return math.asin(self._angleInternal(v1,v2))
    
    def _angleInternal(self,v1,v2):
        temp = self._dotproduct(v1, v2) / (self._length(v1) * self._length(v2))
        if temp>1:
            print 'clipping from '+str(temp)+' to 1'
            temp=1
        if temp<-1:
            print 'clipping from '+str(temp)+' to -1'
            temp=-1
        return temp
    
    def _printDebug(self,stringToPrint):
        stringToPrint
        #print stringToPrint
    
    def rawToAccel(self,raw):
        volt   = (float(raw)/2**12)*3.0
        accel  = -(volt-1.5)/0.6
        return accel

class MoteHandler(threading.Thread):
    
    def __init__(self,dataHandler):
        
        # record params
        self.dataHandler = dataHandler
        
        # initialize the parent class
        threading.Thread.__init__(self)
    
    #======================== public ==========================================
    
    def run(self):
        serialHandler = serial.Serial(self._findSerialPortsNames()[0],baudrate=115200)
        state         = "WAIT_HEADER"
        numdelimiter  = 0
        while True:
            char = serialHandler.read(1)
            if state == "WAIT_HEADER":
                if char == '^':
                    numdelimiter = numdelimiter + 1
                else:
                    numdelimiter = 0
                if numdelimiter==3:
                    state = "RECEIVING_COMMAND"
                    input = ""
                    numdelimiter = 0
            else:
                if state == "RECEIVING_COMMAND":
                    input=input+char
                    if char == '$':
                        numdelimiter = numdelimiter + 1
                    else:
                        numdelimiter = 0
                    if numdelimiter==3:
                        state        = "WAIT_HEADER"
                        numdelimiter = 0
                        input = input.rstrip('$')
                        self.dataHandler.handleData(input)
    
    #======================== helpers =========================================
    
    def _findSerialPortsNames(self):
        serialport_names = []
        if (os.name=='nt' or os.name=='posix'):
            path = 'HARDWARE\\DEVICEMAP\\SERIALCOMM'
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path)
            for i in range(winreg.QueryInfoKey(key)[1]):
                try:
                    val = winreg.EnumValue(key,i)
                except:
                    pass
                else:
                    if (val[0].find('USBSER')>-1):
                        serialport_names.append(str(val[1]))
        elif os.name=='posix':
            serialport_names = glob.glob('/dev/ttyUSB*')
        serialport_names.sort()
        return serialport_names

class GuiHandler(object):
    
    GUI_SIZE         = 500
    UPDATE_PERIOD    = 200
    OVAL_RADIUS      = 10
    INFLATION_FACTOR = ((2*GUI_SIZE)/(2*3.14))
    INFLATION_FACTOR = 300
    
    def __init__(self,dataHandler):
    
        # record params
        self.dataHandler = dataHandler
        
        # define GUI
        self.window = Tkinter.Tk()
        self.canvas = Tkinter.Canvas(self.window,width=self.GUI_SIZE,
                                            height=self.GUI_SIZE,
                                            bg='black')
        self.canvas.after(self.UPDATE_PERIOD,self._updateCanvas)
        self.canvas.grid(row=0,columnspan=2)
        
        self.canvas.create_line(self.GUI_SIZE/2, 0,
                                self.GUI_SIZE/2, self.GUI_SIZE,
                                fill="white")
        self.canvas.create_line(0,               self.GUI_SIZE/2,
                                self.GUI_SIZE,   self.GUI_SIZE/2, 
                                fill="white")
        self.canvas.create_line(0,               self.GUI_SIZE/2,
                                self.GUI_SIZE,   self.GUI_SIZE/2, 
                                fill="white")
        self.thePoint = self.canvas.create_oval(0,0,self.OVAL_RADIUS,self.OVAL_RADIUS,fill='green')
        
        self.buttonFront = Tkinter.Button(self.window,text='front',command=self._calibrateFront)
        self.buttonFront.grid(row=1,column=0)
        
        self.buttonPerfect = Tkinter.Button(self.window,text='perfect',command=self._calibratePerfect)
        self.buttonPerfect.grid(row=2,column=0)
        
        self.buttonSide = Tkinter.Button(self.window,text='side',command=self._calibrateSide)
        self.buttonSide.grid(row=2,column=1)
    
    #======================== public ==========================================
    
    def start(self):
        self.window.mainloop()
    
    #======================== private =========================================
    
    def _rawToCoordinates(self,raw):
        return (raw*self.INFLATION_FACTOR)+(self.GUI_SIZE/2)
    
    def _updateCanvas(self):
        temp = self.dataHandler.getData()
        
        if temp:
            (rawX,rawY,_,_) = temp
            
            newcoords = (self._rawToCoordinates(rawX)-self.OVAL_RADIUS,
                         self._rawToCoordinates(rawY)-self.OVAL_RADIUS,
                         self._rawToCoordinates(rawX)+self.OVAL_RADIUS,
                         self._rawToCoordinates(rawY)+self.OVAL_RADIUS,)
            
            self.canvas.coords(self.thePoint,newcoords)
        
        self.canvas.after(self.UPDATE_PERIOD,self._updateCanvas)
    
    def _calibrateFront(self):
        self.dataHandler.calibrateFront()
        self.buttonFront.configure(bg='green')
    
    def _calibratePerfect(self):
        self.dataHandler.calibratePerfect()
        self.buttonPerfect.configure(bg='green')
    
    def _calibrateSide(self):
        self.dataHandler.calibrateSide()
        self.buttonSide.configure(bg='green')

class AngleWatchdog(threading.Thread):
    
    THRES_UPDATE = 5
    THRES_ANGLE  = 0.10
    
    def __init__(self,dataHandler):
        
        # record params
        self.dataHandler = dataHandler
        
        # variables
        self.numAngleErrors = 0
        
        # initialize the parent class
        threading.Thread.__init__(self)
    
    def run(self):
        
        while True:
            # wait before getting data
            time.sleep(1)
            
            # get data
            temp = self.dataHandler.getData()
        
            if temp:
            
                (_,_,angleError,lastUpdate) = temp
            
                if (time.time()-lastUpdate>self.THRES_UPDATE):
                    print 'NO UPDATES!!!'
                    winsound.PlaySound("SystemExclamation", winsound.SND_ALIAS)
                
                if angleError>self.THRES_ANGLE:
                    self.numAngleErrors += 1
                    if self.numAngleErrors>3:
                        print 'ANGLE!!!'
                        winsound.PlaySound("SystemExclamation", winsound.SND_ALIAS)
                else:
                    self.numAngleErrors=0

#======================== main ================================================

def main():
    dataHandler = DataHandler()
    moteHandler = MoteHandler(dataHandler)
    guiHandler  = GuiHandler(dataHandler)
    watchdog    = AngleWatchdog(dataHandler)
    
    moteHandler.start()
    watchdog.start()
    guiHandler.start()

if __name__=='__main__':
    main()
