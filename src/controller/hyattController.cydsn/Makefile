hex:
	cd ..;/mnt/c/Program\ Files\ \(x86\)/Cypress/PSoC\ Creator/4.2/PSoC\ Creator/bin/cyprjmgr.exe -w "hyattController.cywrk" -build

clean:
	cd ..;/mnt/c/Program\ Files\ \(x86\)/Cypress/PSoC\ Creator/4.2/PSoC\ Creator/bin/cyprjmgr.exe -w "hyattController.cywrk" -clean
flash: 
	cd ..;/mnt/c/Program\ Files\ \(x86\)/Cypress/Programmer/PSoCProgrammer.exe c:/tmp/hyatt/src/controller/hyattController.cydsn/ARM_GCC_541/Debug/hyattController.hex 
con: 
	cd ..;rm -rf /var/lock/LCK..ttyS4;microcom -p /dev/ttyS4 -s 115200


# flash:
# 	/mnt/c/Program\ Files\ \(x86\)/Cypress/PSoC\ Creator/4.2/PSoC\ Creator/bin/cyprjmgr.exe -w "hyattController.cywrk" -build
# 	hyattController.cydsn/ARM_GCC_541/Debug/hyattController.hex
# 
# OpenPort KitProg/06100A18001E6400 .
# SetAcquireMode "Reset" 
# SetProtocol 8
# SetProtocolClock 224  ??
# SetPowerVoltage 5.0
# PowerOn
# DAP_Acquire
# HEX_ReadFile hyattController.cydsn/ARM_GCC_541/Debug/hyattController.hex
# 

