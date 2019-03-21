( https://nraynaud.github.io/webgcode/      )
( 1/4-20, M6 flush threaded insert          )
( 23/64" insert     0.3594"/2, r=0.1797"    )
( 1/4" endmill      0.2500"/2, r=0.1250"    )
(                   0.1797-0.1250 = 0.0547" ) 
( for 0.75" MDF                             )

M3  S1000
G20 G91 F200 (inches, relative)
G1  Z0.5
G1  Y0.0547

G1  Z-0.6
G02 X0 Y-0.1094 R0.0547
G02 X0 Y0.1094 R0.0547

G1  Z-0.1
G02 X0 Y-0.1094 R0.0547
G02 X0 Y0.1094 R0.0547

G1  Z-0.1
G02 X0 Y-0.1094 R0.0547
G02 X0 Y0.1094 R0.0547

G1  Z-0.1
G02 X0 Y-0.1094 R0.0547
G02 X0 Y0.1094 R0.0547

G1  Z-0.1
G02 X0 Y-0.1094 R0.0547
G02 X0 Y0.1094 R0.0547

G1  Z-0.1
G02 X0 Y-0.1094 R0.0547
G02 X0 Y0.1094 R0.0547

G1  Z-0.1
G02 X0 Y-0.1094 R0.0547
G02 X0 Y0.1094 R0.0547

G1 Z1.0
M5
M30
