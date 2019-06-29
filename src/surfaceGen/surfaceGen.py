#!/usr/bin/env python3

import sys, math

l = len(sys.argv)

xwidth      = 500.0
ywidth      = 500.0
zwidth      = 0.5
bitdiameter = 25.4
bitstepover = 20.0
speed       = 1000

passes      = 3

bitradius   = bitdiameter / 2.0
yend        = ywidth - bitradius
zdiff       = zwidth / float(passes-1)

xend        = xwidth - bitradius
columns     = (xwidth - bitdiameter) / bitstepover
columns     = math.ceil((xwidth - bitdiameter) / bitstepover)
xdiff       = (xwidth - bitdiameter) / (columns) 

x = bitradius
y = bitradius
print("(surfaceGen %01.f x %0.1f x %0.1f with %0.1f@%0.1f)"%(xwidth,ywidth,zwidth,bitdiameter,bitstepover))
print("G90 G94")
print("G17")
print("G21")
print("G28 G91 Z0")
print("G90")
print("T9 M6")
print("G54")
print("G10L20P1X%0.4fY%0.4f"%(x,y))
print("M3")
print("G4 P4.0")

print("\n(start)")
print("G1F%d"%speed)

z = 0.0
for p in range(passes):
    print("Z%0.4f"%z)
    z -= zdiff

    for column in range(0,columns+1):
        print("X%0.4f"%x)
        print("Y%0.4f"%yend)
        if column != columns:
            print("Y%0.4f"%bitradius)
        x += xdiff 

    x = bitradius
    print("X%0.4f"%x)                               # moves to NW
    print("Y%0.4f"%bitradius)                       # moves to SW
    print("X%0.4fY%0.4f"%(bitradius/2,bitradius/2)) # 0,0 knockout
    print("X%0.4fY%0.4f"%(x,bitradius))             # move to SW

    print()

x = bitradius
print("X%0.4f"%x)


print("G28 G91 Z0")
print("G90")
print("M30")


