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

#ifndef __DELAY__
#define __DELAY__

#define DECLARE_DELAY(type, len) _DECLARE_DELAY(type, len)
#define _DECLARE_DELAY(type, len)                                       \
  typedef struct {                                                      \
    unsigned int t;                                                     \
    type z[len];                                                        \
  } delay_##type##_##len##_t;                                           \
  static const type *delay_##type##_##len##_read(const delay_##type##_##len##_t *delay, unsigned int idx) \
  {                                                                     \
    return &delay->z[(delay->t + (len - idx)) % len];                   \
  }                                                                     \
  static void delay_##type##_##len##_write(delay_##type##_##len##_t *delay, const type *val) \
  {                                                                     \
    if(++delay->t >= len) delay->t = 0;                                 \
    delay->z[delay->t] = *val;                                          \
  }                                                                     \
  static void delay_##type##_##len##_fill(delay_##type##_##len##_t *delay, const type *val) \
  {                                                                     \
    for(unsigned int i = 0; i < len; i++)                               \
    {                                                                   \
      delay->z[i] = *val;                                               \
    }                                                                   \
    delay->t = 0;                                                       \
  }

#define DELAY(type, len) _DELAY(type, len)
#define _DELAY(type, len) delay_##type##_##len##_t
#define DELAY_READ(delay, type, len, idx) _DELAY_READ(delay, type, len, idx)
#define _DELAY_READ(delay, type, len, idx) delay_##type##_##len##_read(delay, idx)
#define DELAY_WRITE(delay, type, len, val) _DELAY_WRITE(delay, type, len, val)
#define _DELAY_WRITE(delay, type, len, val) delay_##type##_##len##_write(delay, val)
#define DELAY_FILL(delay, type, len, val) _DELAY_FILL(delay, type, len, val)
#define _DELAY_FILL(delay, type, len, val) delay_##type##_##len##_fill(delay, val)

#endif
