#!/usr/bin/env python3

from matplotlib import pyplot

import numpy as np
import struct
import sys
import math

def main(name: str, frame_idx: int) -> None:
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
    frame = []
    (endiness,) = struct.unpack(pre + 'b', f.read(1))
    (length,) = struct.unpack('N', f.read(8))
    for i in range(0, length):
        pyplot.plot([float(x) for x in struct.unpack('d'*length, f.read(8*i))])
        pyplot.show()

    f.close()

    ax.clear()
    pyplot.plot(frame[0])
    pyplot.show()

if __name__ == '__main__':
    main(sys.argv[1], int(sys.argv[2]))
