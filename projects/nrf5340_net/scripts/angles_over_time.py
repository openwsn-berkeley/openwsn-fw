import struct
import numpy                as np
import matplotlib.pyplot    as plt
from matplotlib             import  rcParams
import os

# direction finding related
from config     import *
from calc_angle import *

horizental_folder = 'antenna_array_1(horizon)'
vertical_folder   = 'antenna_array_2(vertical)'

rcParams['font.size'] = 20

raw_data = {
        horizental_folder: {}, 
                
        vertical_folder:   {}
}

ref_angles  = {
    vertical_folder : {
        '0.0m' : 45.868250787968954,
        '0.3m' : 55.819583777115064,
        '0.6m' : 68.79090838883613,
        '0.9m' : 84.45883355636084,
        '1.2m' : 100.98057542761217,
        '1.5m' : 115.8766900608275,
        '1.8m' : 127.81553941619688, 
        '2.1m' : 136.86074205679654,
    },
    
    horizental_folder : {
        '2.1m' : 45.868250787968954,
        '1.8m' : 55.819583777115064,
        '1.5m' : 68.79090838883613,
        '1.2m' : 84.45883355636084,
        '0.9m' : 100.98057542761217,
        '0.6m' : 115.8766900608275,
        '0.3m' : 127.81553941619688, 
        '0.0m' : 136.86074205679654,
    }
}

for key, item in raw_data.items(): 
    for file in os.listdir("./{0}".format(key)):
        print("processing {0}/{1}...".format(key, file))
        distance = file.split('_')[-1][:-4]
        item[distance] = {1: [], 2: []}
        with open("./{0}/{1}".format(key, file),'r') as f:
            for line in f:
                sample = eval(line)
                if sample['angle'] != 254:
                    item[distance][sample['array']].append(sample['angle'] - ref_angles[key][distance])


if __name__ == '__main__':
    fig, ax = plt.subplots()
    
    for key, item in raw_data.items():
        data    = []
        xlabel  = []
        if key == horizental_folder:
            for pos in sorted(item.keys()):
                data.append(item[pos][1])
                xlabel.append(pos)
        elif key == vertical_folder:
            for pos in sorted(item.keys()):
                data.append(item[pos][2])
                xlabel.append(pos)
                
        ax.violinplot(  
                        data, 
                        showmeans=False,
                        showmedians=True
        )
        ax.set_xticks([i for i in range(8)])
        ax.set_xticklabels(xlabel, rotation = 45)
        ax.set_title(key)
        ax.legend()
        fig.tight_layout()
        fig.savefig("./{0}.png".format(key))
        ax.clear()