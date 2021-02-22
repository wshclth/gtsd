#!/usr/bin/env python3

from matplotlib import pyplot
import struct

def main():
    f = open('/hdd/scratch/test1.ts', 'rb')
    (endiness,) = struct.unpack('b', f.read(1))
    (num_entries,) = struct.unpack('N', f.read(8))
    data = struct.unpack('d' * num_entries, f.read(8*num_entries))
    f.close()

    pyplot.plot(data)
    pyplot.show()

    fig, ax = pyplot.subplots(1,1)
    for i in range(0, 1000):
        f = open("/hdd/scratch/test1.frame%d" % (i+1), 'rb')
        (endiness,) = struct.unpack('b', f.read(1))
        print(endiness)
        (num_features,) = struct.unpack('N', f.read(8))

        frame = []
        for i in range(0, num_features):
            row = [float(x) for x in struct.unpack('d'*(i+1), f.read(8*(i+1)))]
            for j in range(num_features - i - 1):
                row.append(0)
            frame.append(row)
        f.close()

        ax.imshow(frame)
        pyplot.pause(0.001)
        ax.clear()
    pass

if __name__ == '__main__':
    main()
    pass
