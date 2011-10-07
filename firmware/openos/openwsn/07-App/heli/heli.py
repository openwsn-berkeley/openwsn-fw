import socket
import binascii
import time
import os
import Tkinter
import sys

MOTOR_MAX        = 800
MOTOR_MIN        =   0
MOTOR_STEP       =  10

myAddress        = ''    # means 'any suitable interface'
myPort           = 2158
hisAddress       = '2001:470:1f05:dff:1415:9209:22b:51'
hisPort          = 2192

motor1_command = 0
motor2_command = 0

print "Testing heli..."

def heli_up():
   global motor1_command, motor2_command
   if (motor1_command+MOTOR_STEP<=MOTOR_MAX and motor2_command+MOTOR_STEP<=MOTOR_MAX):
      motor1_command += MOTOR_STEP
      motor2_command += MOTOR_STEP
      sendCommand()

def heli_down():
   global motor1_command, motor2_command
   if (motor1_command-MOTOR_STEP>=MOTOR_MIN and motor2_command-MOTOR_STEP>=MOTOR_MIN):
      motor1_command -= MOTOR_STEP
      motor2_command -= MOTOR_STEP
      sendCommand()

def heli_left():
   global motor1_command
   if (motor1_command+MOTOR_STEP<=MOTOR_MAX):
      motor1_command += MOTOR_STEP
      sendCommand()

def heli_right():
   global motor1_command
   if (motor1_command-MOTOR_STEP>=MOTOR_MIN):
      motor1_command -= MOTOR_STEP
      sendCommand()

def heli_stop():
   global motor1_command, motor2_command
   motor1_command = 0
   motor2_command = 0
   sendCommand()

def heli_takeoff():
   global motor1_command, motor2_command
   motor1_command = 0x0260
   motor2_command = 0x0260
   sendCommand()

def heli_land():
   global motor1_command, motor2_command
   motor1_command = 0x0200
   motor2_command = 0x0200
   sendCommand()

def key_pressed(event):
   if (event.char=='i'):
      heli_up()
   if (event.char=='j'):
      heli_left()
   if (event.char=='l'):
      heli_right()
   if (event.char=='k'):
      heli_down()
   if (event.char==' '):
      heli_stop()
   if (event.char=='a'):
      heli_takeoff()
   if (event.char=='q'):
      heli_land()

def gui_clicked(event):
   global motor1_command, motor2_command
   if (event.x>100 and event.x<200 and event.y>0   and event.y<100):
      heli_up()
   if (event.x>0   and event.x<100 and event.y>100 and event.y<200):
      heli_left()
   if (event.x>200 and event.x<300 and event.y>100 and event.y<200):
      heli_right()
   if (event.x>100 and event.x<200 and event.y>200 and event.y<300):
      heli_down()
   if (event.x>100 and event.x<200 and event.y>100 and event.y<200):
      heli_stop()
   if (event.x>200 and event.x<300 and event.y>0   and event.y<100):
      heli_takeoff()
   if (event.x>200 and event.x<300 and event.y>200 and event.y<300):
      heli_land()
   if (event.y>310 and event.y<340):
      motor1_command = event.x/300.0*800.0
      sendCommand()
   if (event.y>340 and event.y<360):
      motor_diff = motor1_command-motor2_command
      motor1_command = (event.x/300.0*800.0)+(motor_diff/2.0)
      if motor1_command>800:
         motor1_command=800
      motor2_command = (event.x/300.0*800.0)-(motor_diff/2.0)
      if motor2_command>800:
         motor2_command=800
      sendCommand()
   if (event.y>360 and event.y<390):
      motor2_command = event.x/300.0*800.0
      sendCommand()

def sendCommand():
   #update the sliders
   buttonCanvas.delete("temp_slider")
   buttonCanvas.create_rectangle(0  ,310,(motor1_command/800.0)*300.0,340,fill="yellow",tag="temp_slider")
   buttonCanvas.create_rectangle(0  ,360,(motor2_command/800.0)*300.0,390,fill="yellow",tag="temp_slider")
   #send the command over UDP
   request = []
   request.append(chr(int(motor1_command/256)))
   request.append(chr(int(motor1_command%256)))
   request.append(chr(int(motor2_command/256)))
   request.append(chr(int(motor2_command%256)))
   request = ''.join(request)
   hisAddress = addressText.get(1.0,Tkinter.END)
   hisAddress = hisAddress[0:len(hisAddress)-1]
   try:
      socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
      socket_handler.settimeout(5)
      socket_handler.bind((myAddress,myPort))
      socket_handler.sendto(request,(hisAddress,hisPort))
   except:
      addressText.config(background="red")
   else:
      addressText.config(background="green")

#================================= GUI functions ====================

def releaseAndQuit():
   root.quit()
   sys.exit()

#================================= GUI definition ===================

root=Tkinter.Tk()
root.title("OpenHelicopter")
root.protocol("WM_DELETE_WINDOW",releaseAndQuit)
root.resizable(0,0)

root.bind("<Key>",key_pressed)

addressText = Tkinter.Text(root,width=50,height=1)
#displayElements[motePort]["cellTable"]["TextField"].delete("1.0",Tkinter.END)
addressText.insert(Tkinter.END,'2001:470:1f05:dff:1415:9209:22b:51')
addressText.grid(row=0,column=0)

buttonCanvas=Tkinter.Canvas(root,width=300,height=400)
buttonCanvas.bind("<Button-1>",gui_clicked)
buttonCanvas.bind("<Button-2>",gui_clicked)
buttonCanvas.bind("<Button-3>",gui_clicked)
buttonCanvas.create_rectangle(100,  0,200,100,fill="blue")
buttonCanvas.create_text(150,50,text="up <i>")
buttonCanvas.create_rectangle(  0,100,100,200,fill="blue")
buttonCanvas.create_text(50,150,text="left <j>")
buttonCanvas.create_rectangle(200,100,300,200,fill="blue")
buttonCanvas.create_text(250,150,text="right <l>")
buttonCanvas.create_rectangle(100,200,200,300,fill="blue")
buttonCanvas.create_text(150,250,text="down <k>")
buttonCanvas.create_oval(120,120,180,180,fill="red")
buttonCanvas.create_text(150,150,text="stop!\n<space>")
buttonCanvas.create_oval(220, 20,280, 80,fill="green")
buttonCanvas.create_text(250, 50,text="takeoff <a>")
buttonCanvas.create_oval(220,220,280,280,fill="green")
buttonCanvas.create_text(250,250,text="land <q>")

buttonCanvas.create_rectangle(0  ,310,300,340)
buttonCanvas.create_text(150,325,text="motor 1")
buttonCanvas.create_rectangle(0  ,360,300,390)
buttonCanvas.create_text(150,375,text="motor 2")
buttonCanvas.grid(row=1,column=0)

#================================= main =============================

root.mainloop()
