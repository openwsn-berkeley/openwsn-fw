import struct
import numpy                as np
import matplotlib.pyplot    as plt
from matplotlib             import  rcParams

# direction finding related
from config     import *
from calc_angle import *

rcParams['font.size'] = 20

data = [
        {
            'array': 1,
            'angle': []
        },
        {
            'array': 2,
            'angle':[]
        }
    ]

with open('samples_nordic.txt','r') as f:
    for line in f:
        data_entry = eval(line)
        data[data_entry['array']-1]['angle'].append(data_entry['angle'])

if __name__ == '__main__':
    fig, ax = plt.subplots(figsize=(20,4))
    for array_data in data:
        ax.plot(array_data['angle'], label='antenna array {0}'.format(array_data['array']))
    
    ax.legend()
    plt.show()