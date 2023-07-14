import pygame
import serial
import struct
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *

long  = 1
width = 0.8
hight = 0.2

# Cube vertices and faces
vertices = [
    [long, -width, -hight], [long, width, -hight], [-long, width, -hight], [-long, -width, -hight],
    [long, -width, hight], [long, width, hight], [-long, width, hight], [-long, -width, hight]
]

face_edges = (
    (0, 1, 2, 3),
    (4, 5, 6, 7),
    (0, 4, 7, 3),
    (1, 5, 6, 2),
    (0, 1, 5, 4),
    (3, 2, 6, 7)
)

line_edges = [
    [0, 1], [1, 2], [2, 3], [3, 0],
    [0, 4], [1, 5], [2, 6], [3, 7],
    [4, 5], [5, 6], [6, 7], [7, 4]
]

blue_gray   = (0.68, 0.84, 1)
line_white  = (1,1,1)

face_colors = [blue_gray for i in range(6)]

def draw_cube():
    glBegin(GL_QUADS)
    for face in range(len(face_edges)):
        glColor3fv(face_colors[face])
        for vertex in face_edges[face]:
            glVertex3fv(vertices[vertex])
    glEnd()
    
    glColor3f(line_white[0], line_white[1], line_white[2])
    glBegin(GL_LINES)
    for edge in line_edges:
        for vertex in edge:
            glVertex3fv(vertices[vertex])
    glEnd()

def main():
    pygame.init()
    display = (800, 600)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL)

    gluPerspective(45, (display[0] / display[1]), 0.1, 50.0)
    glTranslatef(0.0, 0.0, -5)

    # Connect to the serial port
    ser = serial.Serial('COM14', 115200)  # Replace 'COM3' with your serial port and baud rate

    rawFrame = []
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                quit()

        # Read gyroscope measurements from the serial port

        byte  = ser.read(1)        
        rawFrame += byte
        
        if rawFrame[-2:]==[13, 10]:
                    
            if len(rawFrame) == 8:
                            
                (x_gyro, y_gyro, z_gyro) = struct.unpack('>hhh', bytes(rawFrame[:-2]))
                        
                # debug info
                output = 'gyr_x={0:<6} gyr_y={1:<6} gyr_z={2:<6}'.format(
                    x_gyro,
                    y_gyro,
                    z_gyro
                )
                
            rawFrame         = []
            
            # GYRO_RESOLUATION_0 = 16.4
            GYRO_RESOLUATION_1 = 32.8
            # GYRO_RESOLUATION_2 = 65.6
            # GYRO_RESOLUATION_3 = 131.2
            
            # GYRO_RESOLUATION_4 = 264.4
            gyro_reso = GYRO_RESOLUATION_1
            DATA_INTERVAL = 0.0625
            
            x_gyro = DATA_INTERVAL*float(x_gyro)/float(gyro_reso)
            y_gyro = DATA_INTERVAL*float(y_gyro)/float(gyro_reso)
            z_gyro = DATA_INTERVAL*float(z_gyro)/float(gyro_reso)

            # Rotate the cube based on gyroscope measurements
            glRotatef(x_gyro, 1, 0, 0)
            glRotatef(y_gyro, 0, 1, 0)
            glRotatef(z_gyro, 0, 0, 1)
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            draw_cube()
            pygame.display.flip()
            pygame.time.wait(10)

    # Close the serial port when finished
    ser.close()

if __name__ == '__main__':
    main()
