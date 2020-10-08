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

def degree_range(n):
    start = np.linspace(0, 180, n+1, endpoint=True)[0:-1]
    end = np.linspace(0, 180, n+1, endpoint=True)[1::]
    mid_points = start + ((end-start)/2.)
    return np.c_[start, end], mid_points


def rot_text(ang):
    rotation = np.degrees(np.radians(ang) * np.pi / np.pi - np.radians(90))
    return rotation

def gauge(colors='jet_r', title=''):

    labels = [i for i in range(180)]
    N = 180

    # if colors is a colormap
    cmap = cm.get_cmap(colors, N)
    cmap = cmap(np.arange(N))
    colors = cmap[::-1, :].tolist()

    ang_range, mid_points = degree_range(N)
    labels = labels[::-1]

    patches = []
    for ang, c in zip(ang_range, colors):
        # sectors
        patches.append(Wedge((0., 0.), .4, *ang, facecolor='w', lw=2))
        # arcs
        patches.append(Wedge((0., 0.), .4, *ang, width=0.10,
                             facecolor=c, lw=2, alpha=0.5))

    [ax.add_patch(p) for p in patches]

    # for mid, lab in zip(mid_points, labels):
        # ax.text(0.35 * np.cos(np.radians(mid)), 0.35 * np.sin(np.radians(mid)),
                # lab, ha='center', va='center', fontsize=14, fontweight='bold',
                # rotation=rot_text(mid))

    r = Rectangle((-0.4, -0.1), 0.8, 0.1, facecolor='w', lw=2)
    ax.add_patch(r)
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
    
def animate(i, data_source):
    
    ang_range, mid_points = degree_range(180)
    
    num_lines = 0
    if data_source == '1':
        with open(sample_file, 'r') as f:
            for line in f:
                num_lines += 1
    else:
        num_lines = i
                
    angle = get_angle_to_pkt(num_lines)
    if angle:
        pos = mid_points[abs(int(angle)-179)]
        ax.clear()
        ax.arrow(
            0, 0, 0.225 * np.cos(np.radians(pos)), 0.225 *
            np.sin(np.radians(pos)), width=0.01, head_width=0.02,
            head_length=0.1, fc='k', ec='k'
        )
        gauge()
        
if __name__ == '__main__':

    data_source = raw_input("read from file (0) or serial (1)?")
    
    print type(data_source)
    
    if data_source == '1':
        print "start serial reading..."
        serial_thread = Thread( target = start_read)
        serial_thread.start()
        
        while os.path.exists(sample_file) == False:
            pass
    
    labels = [i for i in range(180)]
    fig, ax = plt.subplots()
    anim = animation.FuncAnimation(fig, animate, interval=100, fargs=(data_source,), save_count=0)
    plt.show()