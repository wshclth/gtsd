#!/usr/bin/env python3

from matplotlib import pyplot

import numpy as np
import struct
import sys
import math

def main(name: str) -> None:
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
    while True:
        try:
            f = open(name + ".frame%d" % (frame_number), 'rb')
            frame = []
            (endiness,) = struct.unpack(pre + 'b', f.read(1))
            (num_features,) = struct.unpack('N', f.read(8))
            for i in range(0, num_features):
                print(i, '/', num_features)
                row = [x for x in struct.unpack('d'*(i+1), f.read(8*(i+1)))]
                for j in range(num_features - i - 1):
                    row.append(0)
                row = np.where(np.isnan(row), 0, row)
                frame.append(row)
            f.close()

            ax.clear()
            ax.imshow(frame, interpolation='none',
                      aspect = 'auto')
            pyplot.pause(1e-14)
            frame_number += 1
        except FileNotFoundError:
            break

    ax.clear()
    im = ax.imshow(frame, interpolation='none',
                   aspect = 'auto')
    fig.colorbar(im)
    pyplot.show()

if __name__ == '__main__':
    main(sys.argv[1])
