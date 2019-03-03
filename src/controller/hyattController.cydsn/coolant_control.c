/*
  coolant_control.c - coolant control methods
  Part of Grbl

  Copyright (c) 2012-2015 Sungeun K. Jeon
      Updated For PSoC 2017-2018 Barton Dring buildlog.net

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

#include "grbl.h"


void coolant_init()
{ 
  coolant_set_state(COOLANT_DISABLE);
}

// Returns current coolant output state. Overrides may alter it from programmed state.
uint8_t coolant_get_state()
{
  uint8_t cl_state = COOLANT_STATE_DISABLE;
  
  if (MIST_ENABLE_OUT_Read()) 
  {  
#ifdef INVERT_COOLANT_MIST_PIN
  cl_state &= ~COOLANT_STATE_MIST;
#else
  cl_state |= COOLANT_STATE_MIST;
#endif
  }

  if (FLOOD_ENABLE_OUT_Read()) 
  {  
#ifdef INVERT_FLOOD_MIST_PIN
  cl_state &= ~COOLANT_STATE_FLOOD;
#else
  cl_state |= COOLANT_STATE_FLOOD;
#endif
  }
  
  return(cl_state);
}


void coolant_stop()
{
#ifdef INVERT_COOLANT_MIST_PIN
    MIST_ENABLE_OUT_Write(1);
#else
    MIST_ENABLE_OUT_Write(0);
#endif
#ifdef INVERT_COOLANT_FLOOD_PIN
    FLOOD_ENABLE_OUT_Write(1);
#else
    FLOOD_ENABLE_OUT_Write(0);
#endif
}


void coolant_set_state(uint8_t mode)
{
  if (sys.abort) { return; } // Block during abort.
  
  if (mode == COOLANT_DISABLE) // M9
  {  
    coolant_stop();
  }
  else
  {
    if (mode & COOLANT_MIST_ENABLE) // M7
    {
        #ifdef INVERT_COOLANT_MIST_PIN
            MIST_ENABLE_OUT_Write(0);
        #else
            MIST_ENABLE_OUT_Write(1);
        #endif
    }    
    if (mode & COOLANT_FLOOD_ENABLE) 
    {
        #ifdef INVERT_FLOOD_MIST_PIN
            FLOOD_ENABLE_OUT_Write(0);
        #else
            FLOOD_ENABLE_OUT_Write(1);
        #endif
    }    
  }
  sys.report_ovr_counter = 0; // Set to report change immediately
 
}


// G-code parser entry-point for setting coolant state. Forces a planner buffer sync and bails 
// if an abort or check-mode is active.
void coolant_sync(uint8_t mode)
{
  if (sys.state == STATE_CHECK_MODE) { return; }
  protocol_buffer_synchronize(); // Ensure coolant turns on when specified in program.
  coolant_set_state(mode);
}
