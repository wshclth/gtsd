#!/usr/bin/env python3

from matplotlib import pyplot

import numpy as np
import struct
import sys
import math

def main(name: str, frame_idx_start:int, frame_idx_end: int) -> None:
    f = open(name + '.ts', 'rb')
    (endiness,) = struct.unpack('b', f.read(1))


    pre = '@'
    if endiness != 1:
        print('warning: swapping byteorder, system endiness ', sys.byteorder)

    (num_entries,) = struct.unpack(pre + 'N', f.read(8))
    data = struct.unpack(pre + 'd'*num_entries, f.read(8*num_entries))
    f.close()

    fig, axs = pyplot.subplots(1, 2)

    axs[0].plot(data)
    ax = axs[1]

    frame_number = 0
    frame = []

    f = open(name, 'rb')
    (endiness,) = struct.unpack(pre + 'b', f.read(1))
    (length,) = struct.unpack('N', f.read(8))

    print(length)
    frame = []
    
    for i in range(0, frame_idx_end):
        row = [0 for _ in range(i)] + [float(x) for x in struct.unpack(pre + 'd'*(length-i), f.read(8*(length-i)))] 
        if i >= frame_idx_start:
            row = np.array(row)
            where_are_NaNs = np.isnan(row)
            row[where_are_NaNs] = 0 
            frame.append(row)

    f.close()
    ax.clear()
   
    frame = np.array(frame)
    frame = frame.transpose()
    ax.imshow(frame, aspect='auto')
    pyplot.show()

if __name__ == '__main__':
    main(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))
