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

#ifndef DTASK_GEN
#include "pump_tasks.h"
#endif

#define BUTTON_RESET_TIME 2
#define LONG_PRESS_TIME 16
#define PUMP_PERIOD 5
#define ENABLED_TIME 64
#define MAX_ANIM 64

DTASK_GROUP(pump_tasks)

// button event types
enum button_event {
  BTN_PRESS = 0,   // normal press
  BTN_RELEASE,     // release after press
  BTN_LONG_PRESS,  // long press
  BTN_LONG_RELEASE // release after long press
};

// pass though external button GPIO state
DTASK(gpio_button, bool) {
  return true;
}

// pass through external timer
DTASK(timer, uint16_t) {
  return true;
}

// converts raw GPIO to button events
DTASK(button, struct { int event; uint16_t press_time; } ) {

  // dependencies
  bool gpio = *DREF(gpio_button);
  uint16_t time = *DREF(timer);
  int event = DREF(button)->event;

  // on GPIO change
  if(DTASK_AND(GPIO_BUTTON)) {
    // button presses are rate limited
    if(gpio && time - DREF(button)->press_time > BUTTON_RESET_TIME) {
      event = BTN_PRESS;
      DREF(button)->press_time = time;
    } else if(!gpio) {
      if(event == BTN_LONG_PRESS) {
        event = BTN_LONG_RELEASE;
      } else {
        event = BTN_RELEASE;
      }
    }
  }

  // promote press to a long press
  if(gpio &&
     event == BTN_PRESS &&
     time - DREF(button)->press_time > LONG_PRESS_TIME) {
    event = BTN_LONG_PRESS;
  }

  if(event == DREF(button)->event) {
    return false;
  } else {
    DREF(button)->event = event;
    return true;
  }
}

// animation types
enum animation {
  ANIM_NONE = 0,
  ANIM_START,
  ANIM_CHANGE,
  ANIM_END,
  ANIM_READ
};

// use button events to adjust the time setting
DTASK(time_setting, struct {int on_time; uint16_t change_enable_ts; bool change_enabled; int animation;}) {
  time_setting_t *s = DREF(time_setting);
  uint16_t time = *DREF(timer);
  int event = DREF(button)->event;
  bool change = false;

  // default animation
  s->animation = ANIM_NONE;

  // on button event
  if(DTASK_AND(BUTTON)) {
    if(event == BTN_LONG_PRESS) {
      // enable changes to settings with a long press
      s->change_enable_ts = time;
      s->change_enabled = true;
      s->animation = ANIM_START;
      change = true;
    } else if(event == BTN_RELEASE) {
      if(s->change_enabled) {
        // cycle through settings after a button press
        s->change_enable_ts = time;
        s->on_time = ++s->on_time % PUMP_PERIOD;
        s->animation = ANIM_CHANGE;
      } else {
        // changes are not enabled, so just blink back the current setting
        s->animation = ANIM_READ;
      }
      change = true;
    }
  }

  // disable changes after idle
  if(s->change_enabled && time - s->change_enable_ts > ENABLED_TIME) {
    s->change_enabled = false;
    s->animation = ANIM_END;
    change = true;
  }

  return change;
}

// controls a pump
DTASK(pump, bool) {
  uint16_t time = *DREF(timer);
  time_setting_t *s = DREF(time_setting);

  // enable pump with set duty cycle
  bool pump_state = (time >> 9) % PUMP_PERIOD <= s->on_time;

  if(pump_state != *DREF(pump)) {
    *DREF(pump) = pump_state;
    return true;
  } else {
    return false;
  }
}

// status led, indicates if the pump is running
DTASK(status_led, bool) {
  uint16_t time = *DREF(timer);
  bool pump = *DREF(pump);

  // indicate pump status
  // - solid = running
  // - blink = idle
  *DREF(status_led) = pump || (time & 31) == 0;
  return true;
}

// UI led, used to read and change the time setting
DTASK(ui, struct { bool led; uint16_t start; int anim; } ) {
  time_setting_t *s = DREF(time_setting);
  uint16_t time = *DREF(timer);

  if(DTASK_AND(TIME_SETTING)) {
    // start animation indicated from time_setting
    DREF(ui)->start = time;
    DREF(ui)->anim = s->animation;
  }

  uint16_t t = time - DREF(ui)->start;

  // reset animation after maximum length
  if(t > MAX_ANIM) DREF(ui)->anim = ANIM_NONE;

  switch(DREF(ui)->anim) {
  case ANIM_NONE:
    // off
    DREF(ui)->led = false;
    break;
  case ANIM_START:
    // single long blink
    DREF(ui)->led = t < 16;
    break;
  case ANIM_CHANGE:
  case ANIM_READ:
    // blink back time setting slowly
    DREF(ui)->led = ((t & 4) != 0) && (t >> 3 <= s->on_time);
    break;
  case ANIM_END:
    // quick flashes
    DREF(ui)->led = ((t & 1) != 0) && (t < 8);
    break;
  default:
    break;
  }
  return true;
}
