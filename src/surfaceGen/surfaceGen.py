#!/usr/bin/env python3

import sys

l = len(sys.argv)

xwidth      = 300.0
ywidth      = 400.0
zwidth      = 0.5
bitwidth    = 25.4
bitstepover = 20.0



print("(surfaceGen %01.f x %0.1f x %0.1f with %0.1f@%0.1f)"%(xwidth,ywidth,zwidth,bitwidth,bitstepover))
print("G90 G94")
print("G17")
print("G21")
print("G28 G91 Z0")
print("G90")
print("G58")
print("G1L20P1X%0.4fY%0.4f"%(bitwidth/2.0,bitwidth/2.0))
print("M3")
print("G4 P4.0")


