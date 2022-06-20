import matplotlib.pyplot    as plt
from matplotlib             import  rcParams
import os
import datetime

n_bins = 20
rcParams['font.size'] = 10

result = {
    1 : {
        'angles':       [],
        'timestamp':    [],
        'relativeTime': []
    },
    2 : {
        'angles':       [],
        'timestamp':    [],
        'relativeTime': []
    }
}

isFirstSample = True
firstSample   = None
with open("samples.txt", 'r') as f:
    for line in f:
        entry = eval(line)
       
        
        if entry["angle"] != 254:
        
            if isFirstSample:
                firstSample = entry
                isFirstSample = False
        
            result[entry["array"]]['angles'].append(entry["angle"])
            result[entry["array"]]['timestamp'].append(entry["timestamp"])
            
            # 2022-06-20 09:19:41.162236
            timestamp       = datetime.datetime.strptime(entry["timestamp"],        "%Y-%m-%d %H:%M:%S.%f")
            firstTimestamp  = datetime.datetime.strptime(firstSample['timestamp'],  "%Y-%m-%d %H:%M:%S.%f")
            
            relativeTime    = (timestamp-firstTimestamp).total_seconds()
            # print((firstTimestamp, timestamp, round(relativeTime,2)))
            result[entry["array"]]['relativeTime'].append(round(relativeTime,2))

if __name__ == '__main__':
    fig, axes = plt.subplots(2, 2, sharey=True, tight_layout=True)
    
    print(axes)
    
    for array, data in result.items():
        
        axes[0][array-1].hist(data['angles'], n_bins)
        axes[0][array-1].grid(True)
        axes[0][array-1].set_title("array {0}".format(array))
        
    for array, data in result.items():
        
        axes[1][array-1].plot(data['relativeTime'], data['angles'], '-^')
        axes[1][array-1].grid(True)
        axes[1][array-1].set_title("array {0}".format(array))

    plt.show()
    fig.savefig("singleSample.png")