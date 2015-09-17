#ifndef __DELAY__
#define __DELAY__

#define DECLARE_DELAY(type, len)                                        \
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

#define DELAY(type, len) delay_##type##_##len##_t
#define DELAY_READ(delay, type, len, idx) delay_##type##_##len##_read(delay, idx)
#define DELAY_WRITE(delay, type, len, val) delay_##type##_##len##_write(delay, val)
#define DELAY_FILL(delay, type, len, val) delay_##type##_##len##_fill(delay, val)

#endif
