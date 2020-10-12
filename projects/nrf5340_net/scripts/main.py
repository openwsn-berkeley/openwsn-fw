import matplotlib
from matplotlib import cm
from matplotlib import pyplot as plt
import numpy as np

import matplotlib.animation as animation
from matplotlib.patches import Circle, Wedge, Rectangle

from sample_parser_from_file import *
from sample_reader_from_serial import *

from threading import Thread
import os
import gc
import sys

def degree_range(n):
    start = np.linspace(0, 360, n+1, endpoint=True)[0:-1]
    end = np.linspace(0, 360, n+1, endpoint=True)[1::]
    mid_points = start + ((end-start)/2.)
    return np.c_[start, end], mid_points


def rot_text(ang):
    rotation = np.degrees(np.radians(ang) * np.pi / np.pi - np.radians(90))
    return rotation

def gauge(colors='jet_r', title=''):

    labels = [i for i in range(360)]
    N = 36

    # if colors is a colormap
    cmap = cm.get_cmap(colors, N)
    cmap = cmap(np.arange(N))
    colors = cmap[::-1, :].tolist()

    ang_range, mid_points = degree_range(N)
    labels = labels[::-1]

    patches = []
    for ang, c in zip(ang_range, colors):
        # sectors
        # patches.append(Wedge((0., 0.), .4, *ang, facecolor='w', lw=2))
        # arcs
        patches.append(Wedge((0., 0.), .4, *ang, width=0.10,
                             facecolor=c, lw=2, alpha=0.5))

    [ax.add_patch(p) for p in patches]

    labels = mid_points = [0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330]
    
    for mid, lab in zip(mid_points, labels):
    
        ax.text(0.42 * np.cos(np.radians(mid)), 0.42 * np.sin(np.radians(mid)),
                lab, ha='center', va='center', fontsize=14, fontweight='bold',
                rotation=rot_text(mid))

    # r = Rectangle((-0.4, -0.1), 0.8, 0.1, facecolor='w', lw=2)
    # ax.add_patch(r)
    ax.text(0, -0.05, title, horizontalalignment='center',
            verticalalignment='center', fontsize=22, fontweight='bold')

    # pos = mid_points[abs(i - 180)]
    # ax.arrow(0, 0, 0.225 * np.cos(np.radians(pos)), 0.225 *
             # np.sin(np.radians(pos)), width=0.01, head_width=0.02,
             # head_length=0.1, fc='k', ec='k')

    ax.add_patch(Circle((0, 0), radius=0.02, facecolor='k'))
    ax.add_patch(Circle((0, 0), radius=0.01, facecolor='w', zorder=11))

    ax.set_frame_on(False)
    ax.axes.set_xticks([])
    ax.axes.set_yticks([])
    ax.axis('equal')
    plt.tight_layout()
    
def animate(i, data_source, on_board_calculation):
    
    ang_range, mid_points = degree_range(360)
    
    angles = [None, None]
    
    if data_source == 'r':
    
        num_lines = 0
        with open(sample_file, 'r') as f:
            for line in f:
                num_lines += 1
        pkt_id = num_lines
    
        pkt_id = i
        sys.stdout.write("{0}/{1}\r".format(pkt_id, num_lines))
        sys.stdout.flush()
        
        angle, array = get_angle_to_pkt(pkt_id, on_board_calculation)

        if angle:
        
            if array == 1:
                another_array = 2
            else:
                another_array = 1
            
            j = 1
            another_angle = None
            while (another_array != array or another_angle == None) and pkt_id-j > 0:
                another_angle, another_array = get_angle_to_pkt(pkt_id-j, on_board_calculation)
                j += 1
                
            if j == pkt_id:
                # didn't find the previous angle
                another_angle = None
            else:
                debug_print("angother angle {0}and array {1}".format(another_angle, another_array))
                
            if angle or another_angle:
                ax.clear()
        
            
            if array == 1:
                angles[0] = angle
                angles[1] = another_angle
            else:
                angles[0] = another_angle
                angles[1] = angle
    else :    
        angles = latest_angle
        
    if angles[0] != None and angles[1] != None:
    
        ax.clear()  
    
        if angles[1] > 90:
            angle = angles[0]
        else:
            angle = 360 - angles[0]
            
        angle += 45
            
        if angle >= 360:
            angle -= 360
            
        pos = mid_points[int(angle)]
        ax.arrow(
            0, 0, 0.225 * np.cos(np.radians(pos)), 0.225 *
            np.sin(np.radians(pos)), width=0.01, head_width=0.02,
            head_length=0.1, fc='k', ec='k'
        )
    
        gauge()
        gc.collect()
    return
        
if __name__ == '__main__':

    data_source             = raw_input("Replay from file (r) or real-time from serial (s)?")
    on_board_calculation    = raw_input("on board angle calculation ? (y/n)")
    
    if on_board_calculation == 'n':
        on_board_calculation = False
    else:
        on_board_calculation = True
    
    if data_source == 's':
        
        print "start serial reading..."
        serial_thread = Thread( target = start_read)
        serial_thread.start()
    
    labels = [i for i in range(180)]
    fig, ax = plt.subplots()
    anim = animation.FuncAnimation(fig, animate, interval=100, fargs=(data_source, on_board_calculation), save_count=0)
    plt.show()