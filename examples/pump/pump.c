/* Copyright 2016 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include <msp430g2231.h>
#include "dtask.h"

#ifndef DTASK_GEN
#include "pump_tasks.h"
#endif

// state for all pump tasks
static pump_tasks_state_t state = DTASK_STATE(pump_tasks, 0, 0);

// bit mask of initial tasks to run (triggered externally e.g. interrupt)
static dtask_set_t initial = 0;

static uint16_t clock = 0;

// GPIO lines
#define LED1 0x01
#define LED2 0x40
#define SWITCH 0x08
#define PUMP_GPIO 0x80

void set(uint8_t x, bool on) {
  P1OUT = (P1OUT & ~x) | (on ? x : 0);
}

// interrupts
__attribute__((interrupt(TIMERA0_VECTOR)))
void timer_isr() {
  P1IES = (P1IES & ~SWITCH) | (P1IN & SWITCH); // make sure button interrupts on the right edge
  clock++;
  initial |= TIMER; // mark timer task to run
  LPM0_EXIT; // exit low power mode
}

__attribute__((interrupt(PORT1_VECTOR)))
void button_isr() {
  P1IES ^= SWITCH;  // catch the other edge
  P1IFG &= ~SWITCH; // clear the interrupt flag
  initial |= BUTTON_GPIO; // mark button_gpio task to run
  LPM0_EXIT; // exit low power mode
}

// initialization
void init() {

  // disable watchdog
  WDTCTL = WDTPW | WDTHOLD;

  // LEDs, switch, and pump
  P1DIR = LED1 | LED2 | PUMP_GPIO;
  P1OUT = SWITCH | PUMP_GPIO;
  P1REN = ~P1DIR;
  P1IES = SWITCH;
  P1IE = SWITCH;
  P1IFG = 0;

  // timer, ~8.5Hz, ~512 ticks per minute
  TACCTL0 = CCIE;
  TACTL = MC_1 | ID_3 | TASSEL_2;
  TACCR0 = 16845; /* DCO = ~1.15Mhz */

  __eint(); // enable interrupts
}

void main() {
  init();

  // enable the outputs
  dtask_enable((dtask_state_t *)&state, PUMP | STATUS_LED | UI_LED);

  for(;;) {

    // atomically get the initial set
    __dint();
    dtask_set_t run_initial = initial;
    initial = 0;
    __eint();

    if(run_initial == 0) {

      // enter low power mode
      LPM0;

    } else {

      // external inputs
      state.button_gpio = !(P1IN & SWITCH);
      state.timer = clock;

      // run tasks
      dtask_run((dtask_state_t *)&state, run_initial);

      // external outputs
      set(PUMP_GPIO, state.pump);
      set(LED2, state.status_led);
      set(LED1, state.ui_led);
    }
  }
}
