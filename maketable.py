#!/usr/bin/python

import struct

array = [None]*65537

print 'Parsing tubecable_huffman.c ...'
file = open('tubecable_huffman.c')

for line in file:
	if line.startswith('array',1):
		num = int(line[7:13])
		#len = int(line[25:27])
		str = line.split('"')[1]
		#assert(len == len(str))
		array[num+32768] = str

file.close()

print 'Writing tubecable_huffman.bin ...'
binfile = open('tubecable_huffman.bin','wb')

for i in range(65537):
	code = array[i]
	bincode = 0
	codelen = len(code)
	for j,bit in enumerate(code):
		if (bit == '1'):
			bincode = bincode | (1 << j)
	binfile.write(struct.pack('!BI',codelen,bincode))

binfile.close()
