#!/usr/bin/env python3

from matplotlib import pyplot

import numpy as np
import struct
import sys

def main(name: str) -> None:
    f = open(name + '.ts', 'rb')
    (endiness,) = struct.unpack('b', f.read(1))


    pre = '@'
    if endiness != 1:
        print('warning: swapping byteorder, system endiness ', sys.byteorder)

    (num_entries,) = struct.unpack(pre + 'N', f.read(8))
    data = struct.unpack(pre + 'd'*num_entries, f.read(8*num_entries))
    f.close()

    pyplot.plot(data)
    pyplot.show()

    fig, ax = pyplot.subplots(1,1)
    frame_number = 1
    frame = []
    while True:
        try:
            f = open(name + ".frame%d" % (frame_number), 'rb')
            frame = []
            (endiness,) = struct.unpack(pre + 'b', f.read(1))
            (num_features,) = struct.unpack('N', f.read(8))
            for i in range(0, num_features):
                row = [float(x) for x in struct.unpack('d'*(i+1), f.read(8*(i+1)))]
                for j in range(num_features - i - 1):
                    row.append(0)
                row = np.where(np.isnan(row), 0, row)
                frame.append(row)
            f.close()

            ax.clear()
            ax.imshow(frame, cmap=pyplot.cm.jet, interpolation='none')
            pyplot.pause(1e-14)
            frame_number += 1
        except FileNotFoundError:
            break
   
    ax.clear()
    im = ax.imshow(frame, cmap=pyplot.cm.jet, interpolation='none')
    fig.colorbar(im)
    pyplot.show()

if __name__ == '__main__':
    main(sys.argv[1])
