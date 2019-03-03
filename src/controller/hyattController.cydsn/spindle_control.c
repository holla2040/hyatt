/*
  spindle_control.c - spindle control methods
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
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

#include "grbl.h"

extern parser_block_t gc_block;

#ifdef VARIABLE_SPINDLE
  static float pwm_gradient; // Precalulated value to speed up rpm to PWM conversions.
#endif

// PSoc Rewrite
void spindle_init()
{
  #ifdef VARIABLE_SPINDLE    
    pwm_gradient = SPINDLE_PWM_RANGE/(settings.rpm_max-settings.rpm_min);
    PWM_Spindle_Start();
  #else

    // Configure no variable spindle and only enable pin.
    // TO DO Unsupported PSoC option right now

  #endif
  spindle_stop();
}


uint8_t spindle_get_state()
{
	#ifdef VARIABLE_SPINDLE
        #ifdef USE_SPINDLE_DIR_AS_ENABLE_PIN
              // Not Supported in PSoC Yet
        #else
            #ifdef INVERT_SPINDLE_ENABLE_PIN
              if (!SPINDLE_ENABLE_OUT_Read()) 
            #else
              if (SPINDLE_ENABLE_OUT_Read()) 
            #endif
              { // Check if PWM is enabled.
                if (SPINDLE_DIRECTION_OUT_Read())
                  { return(SPINDLE_STATE_CCW); }
                else 
                  { return(SPINDLE_STATE_CW); }
              }
        #endif
	#else
		// Not Supported in PSoC Yet
	#endif
	return(SPINDLE_STATE_DISABLE);
}


// Disables the spindle and sets PWM output to zero when PWM variable spindle speed is enabled.
// Called by various main program and ISR routines. Keep routine small, fast, and efficient.
// Called by spindle_init(), spindle_set_speed(), spindle_set_state(), and mc_reset().
// PSoC rewrite
void spindle_stop() {
  #ifdef VARIABLE_SPINDLE
    PWM_Spindle_WriteCompare(0);
  #else
    // TO DO Unsupported in PSoC
  #endif
  #ifdef INVERT_SPINDLE_ENABLE_PIN
    SPINDLE_ENABLE_OUT_Write(1);  
  #else
    SPINDLE_ENABLE_OUT_Write(0); 
  #endif
}

#ifdef VARIABLE_SPINDLE
  // Sets spindle speed PWM output and enable pin, if configured. Called by spindle_set_state()
  // and stepper ISR. Keep routine small and efficient.
  // PSoC Rewrite
  void spindle_set_speed(uint8_t pwm_value)  {   
    PWM_Spindle_WriteCompare(pwm_value);
    if (gc_state.modal.spindle != SPINDLE_DISABLE) {
      #ifdef INVERT_SPINDLE_ENABLE_PIN
        SPINDLE_ENABLE_OUT_Write(0);  
      #else
        SPINDLE_ENABLE_OUT_Write(1); 
      #endif
    }
  }

// Called by spindle_set_state() and step segment generator. Keep routine small and efficient.
  int spindle_compute_pwm_value(int rpm) // 328p PWM register is 8-bit.
  {
    int pwm_value;   
    
    rpm *= (0.010*sys.spindle_speed_ovr); // Scale by spindle speed override value.
    
    // PSoC removed a bunch of stuff
    
    rpm = constrain(rpm, settings.rpm_min, settings.rpm_max);
    
    //printPgmString(PSTR("ok"));
    //print_uint8_base10(pwm_value); 
    
    pwm_value = map(rpm, settings.rpm_min, settings.rpm_max, 0, SPINDLE_PWM_MAX_VALUE);
    
    
    return(pwm_value);
  }
#endif


// Immediately sets spindle running state with direction and spindle rpm via PWM, if enabled.
// Called by g-code parser spindle_sync(), parking retract and restore, g-code program end,
// sleep, and spindle stop override.
#ifdef VARIABLE_SPINDLE
  void spindle_set_state(uint8_t state, float rpm)
#else
  void _spindle_set_state(uint8_t state)
#endif
{
  if (sys.abort) { return; } // Block during abort.
  if (state == SPINDLE_DISABLE) { // Halt or set spindle direction and rpm.
  
    #ifdef VARIABLE_SPINDLE
      sys.spindle_speed = 0.0;
    #endif
    spindle_stop();
  
  } else {  
    
    #ifndef USE_SPINDLE_DIR_AS_ENABLE_PIN
      SPINDLE_DIRECTION_OUT_Write(state == SPINDLE_ENABLE_CW);  // PSoC Rewrite       
    #else
      #ifdef INVERT_SPINDLE_ENABLE_PIN
        SPINDLE_ENABLE_OUT_Write(0); 
      #else
        SPINDLE_ENABLE_OUT_Write(1);
      #endif
    #endif
  
    #ifdef VARIABLE_SPINDLE
      // NOTE: Assumes all calls to this function is when Grbl is not moving or must remain off.
      if (settings.flags & BITFLAG_LASER_MODE) { 
        if (state == SPINDLE_ENABLE_CCW) { rpm = 0.0; } // TODO: May need to be rpm_min*(100/MAX_SPINDLE_SPEED_OVERRIDE);
      }
      spindle_set_speed(spindle_compute_pwm_value(rpm));
    #endif
    #if defined(USE_SPINDLE_DIR_AS_ENABLE_PIN) || !defined(VARIABLE_SPINDLE)
      // NOTE: Without variable spindle, the enable bit should just turn on or off, regardless
      // if the spindle speed value is zero, as its ignored anyhow.
      #ifdef INVERT_SPINDLE_ENABLE_PIN
        SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT);   // this 328p code does nothing on PSoC
      #else
        SPINDLE_ENABLE_PORT |= (1<<SPINDLE_ENABLE_BIT);
      #endif    
    #endif
  
  }
  
  sys.report_ovr_counter = 0; // Set to report change immediately
}


// G-code parser entry-point for setting spindle state. Forces a planner buffer sync and bails 
// if an abort or check-mode is active.
#ifdef VARIABLE_SPINDLE
  void spindle_sync(uint8_t state, float rpm)
  {
    if (sys.state == STATE_CHECK_MODE) { return; }
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    spindle_set_state(state,rpm);
  }
#else
  void _spindle_sync(uint8_t state)
  {
    if (sys.state == STATE_CHECK_MODE) { return; }
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    _spindle_set_state(state);
  }
#endif
