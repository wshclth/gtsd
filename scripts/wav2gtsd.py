#!/usr/bin/env python3

from scipy.io import wavfile
from struct import pack

import sys

# Read the audio file
samplerate, data = wavfile.read(sys.argv[1])

print('meta data: samplerate=', samplerate, ', data_len=', len(data))

# Transpose the array
data = data.transpose()

for i in range(len(data)):
    print("processing channel ", i, "of ", sys.argv[1],)
    f = open(sys.argv[1] + '%d.gtsd' % i, 'wb')

    # Write endineess byte
    f.write(pack('?', 1))
    f.write(pack('Q', len(data[i])))

    for sample in data[i]:
        f.write(pack('d', sample))
    f.close()
