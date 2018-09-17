/*
  main.c - An embedded CNC Controller with rs274/ngc (g-code) support
  Part of Grbl
  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud
      Updated For PSOC BJD 2017

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <FS.h>
#include <string.h>
#include <Global.h>
#include "controller.h"

#include "grbl.h"

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow usage of the floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif



/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  The main function performs the following actions:
*   1. Waits until VBUS becomes valid and starts the USBFS component which is
*      enumerated as virtual Com port.
*   2. Waits until the device is enumerated by the host.
*   3. Waits for data coming from the hyper terminal and sends it back.
*   4. PSoC3/PSoC5LP: the LCD shows the line settings.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/

#include "grbl.h"

// Declare system global variable structure
system_t sys; 

int main()
{    
    
    CyGlobalIntEnable;       
    serial_init();
    settings_init(); // Load Grbl settings from EEPROM
    stepper_init();
    system_init();
    hyatt_init();
    
    memset(sys_position,0,sizeof(sys_position)); // Clear machine position
    //memset(&sys, 0, sizeof(sys));  // Clear all system variables
    //sys.abort = true;   // Set abort to complete initialization
    
    // Initialize system state.
    #ifdef FORCE_INITIALIZATION_ALARM
      // Force Grbl into an ALARM state upon a power-cycle or hard reset.
      sys.state = STATE_ALARM;
    #else
      sys.state = STATE_IDLE;
    #endif
    
    // Check for power-up and set system alarm if homing is enabled to force homing cycle
    // by setting Grbl's alarm state. Alarm locks out all g-code commands, including the
    // startup scripts, but allows access to settings and internal commands. Only a homing
    // cycle '$H' or kill alarm locks '$X' will disable the alarm.
    // NOTE: The startup script will run after successful completion of the homing cycle, but
    // not after disabling the alarm locks. Prevents motion startup blocks from crashing into
    // things uncontrollably. Very bad.
    #ifdef HOMING_INIT_LOCK
      if (bit_istrue(settings.flags,BITFLAG_HOMING_ENABLE)) { sys.state = STATE_ALARM; }
    #endif
    
    for(;;)
    {
        // Reset system variables.
        uint8_t prior_state = sys.state;
        memset(&sys, 0, sizeof(system_t)); // Clear system struct variable.
        sys.state = prior_state;
        sys.f_override = DEFAULT_FEED_OVERRIDE;  // Set to 100%
        sys.r_override = DEFAULT_RAPID_OVERRIDE; // Set to 100%
        sys.spindle_speed_ovr = DEFAULT_SPINDLE_SPEED_OVERRIDE; // Set to 100%
    		memset(sys_probe_position,0,sizeof(sys_probe_position)); // Clear probe position.
        sys_probe_state = 0;
        sys_rt_exec_state = 0;
        sys_rt_exec_alarm = 0;
        sys_rt_exec_motion_override = 0;
        sys_rt_exec_accessory_override = 0;
      
      
        // Reset Grbl primary systems.
        serial_reset_read_buffer(); // Clear serial read buffer
        gc_init(); // Set g-code parser to default state
        spindle_init();
        coolant_init();
        limits_init();
        probe_init();
        plan_reset(); // Clear block buffer and planner variables
        st_reset(); // Clear stepper subsystem variables.
    
        // Sync cleared gcode and planner positions to current system position.
        plan_sync_position();    
        gc_sync_position();

        // Print welcome message. Indicates an initialization has occured at power-up or with a reset.
        report_init_message();
        
        // Start Grbl main loop. Processes program inputs and executes them.
        protocol_main_loop();
        //usb_uart_check();
    }
}


/* [] END OF FILE */
