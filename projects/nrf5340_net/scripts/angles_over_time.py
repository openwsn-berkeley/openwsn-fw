import struct
import numpy                as np
import matplotlib.pyplot    as plt
from matplotlib             import  rcParams
import os

# direction finding related
from config     import *
from calc_angle import *

horizental_folder = 'placed_horizontally'
vertical_folder   = 'placed_vertically'

rcParams['font.size'] = 12

raw_data = {
        horizental_folder: {}, 
                
        vertical_folder:   {}
}

ref_angles  = {
    vertical_folder : {
        1: {
            '0.0m' : 90.00,
            '0.3m' : 90.00,
            '0.6m' : 90.00,
            '0.9m' : 90.00,
            '1.2m' : 90.00,
            '1.5m' : 90.00,
            '1.8m' : 90.00, 
            '2.1m' : 90.00,
        },
        2: {
            '0.0m' : 45.868250787968954,
            '0.3m' : 55.819583777115064,
            '0.6m' : 68.79090838883613,
            '0.9m' : 84.45883355636084,
            '1.2m' : 100.98057542761217,
            '1.5m' : 115.8766900608275,
            '1.8m' : 127.81553941619688, 
            '2.1m' : 136.86074205679654,
        }
    },
    
    horizental_folder : {
        2: {
            '2.1m' : 99.54600803453542,
            '1.8m' : 101.04619741453838,
            '1.5m' : 102.60438264837917,
            '1.2m' : 103.77414699802674,
            '0.9m' : 103.96931853613768,
            '0.6m' : 103.06803026277223,
            '0.3m' : 101.57456760711261,
            '0.0m' : 100.02498786207578,
        },
        1: {
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
}

for key, item in raw_data.items(): 
    for file in os.listdir("./{0}".format(key)):
        print("processing {0}/{1}...".format(key, file))
        distance = file.split('_')[-1][:-4]
        item[distance] = {1: [], 2: []}
        with open("./{0}/{1}".format(key, file),'r') as f:
            if file.endswith('.txt'):
                for line in f:
                    sample = eval(line)
                    if sample['angle'] != 254:
                        item[distance][sample['array']].append(sample['angle'] - ref_angles[key][sample['array']][distance])

                        # get the phase data
                        # magPhase  = {'mag_data': [], 'phase_data': []}
                        # for entry in sample['samples']:
                            # magPhase['mag_data'].append(entry['magnitude'])
                            # magPhase['phase_data'].append(entry['phase'])
                            
                        # _, angle = aoa_angle_calculation(magPhase['phase_data'], 0, sample['array'])
                        # item[distance][sample['array']].append(angle - ref_angles[key][distance])


if __name__ == '__main__':
    fig, ax = plt.subplots()
    
    for key, item in raw_data.items():
        data    = {1:[], 2:[]}
        xlabel  = {1:[], 2:[]}
        ref_angle_text   = {1:[], 2:[]}
        for pos in sorted(item.keys()):
            data[1].append(item[pos][1])
            data[2].append(item[pos][2])
            
            xlabel[1].append('{0}\n({1:.1f})'.format( pos, ref_angles[key][1][pos] ))
            xlabel[2].append('{0}\n({1:.1f})'.format( pos, ref_angles[key][2][pos] ))
        
        for ant, angles in data.items():
            parts = ax.violinplot(
                angles, 
                showmeans=True,
                showmedians=False
            )
            
            parts['cmeans'].set_color('black')
            parts['cmins'].set_color('black')
            parts['cmaxes'].set_color('black')
            parts['cbars'].set_color('black')
            for pc in parts['bodies']:
                pc.set_facecolor('red')
                pc.set_edgecolor('red')
                pc.set_alpha(1)
                
            # for i in range(8):
                # ax.annotate('{0}'.format(ref_angle_text[ant][i]),
                    # xy=(i+1, max(angles[i])),
                    # xytext=(0, 3),  # 3 points vertical offset
                    # textcoords="offset points",
                    # ha='center', va='bottom')
                    
            print([sum(angle)/len(angle) for angle in angles])
            
            ax.set_xticks([i+1 for i in range(8)])
            ax.set_xticklabels(xlabel[ant], rotation = 0)
            # ax.set_title(key+' (array_{0})'.format(ant))
            
            ax.set_xlabel('positions\n(groundtruth angles)')
            ax.set_ylabel('errors')
            
            ax.set_ylim(-110, 110)
            ax.grid(True)
            fig.tight_layout()
            fig.savefig("{0}_array_{1}.png".format(key, ant))
            fig.savefig("{0}_array_{1}.eps".format(key, ant))
            ax.clear()