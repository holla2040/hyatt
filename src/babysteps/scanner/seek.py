#!/usr/bin/env python2.7
import sys

if len(sys.argv) == 3:
    seek = int(sys.argv[2])
    fn = sys.argv[1]
else:
    seek = 130 + 1
    fn = "solder.nc"

print fn,seek
print "----------------------"

f = open(fn,"r")

f.seek(seek)

for i in range(10):
    line = f.readline()
    print line.strip()

