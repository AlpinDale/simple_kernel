#ifndef INPUT_H
#define INPUT_H

#include "types.h"

typedef enum {
  INPUT_EVENT_CHAR = 0,
  INPUT_EVENT_UP,
  INPUT_EVENT_DOWN,
  INPUT_EVENT_LEFT,
  INPUT_EVENT_RIGHT,
} input_event_type_t;

typedef struct {
  input_event_type_t type;
  char ch;
} input_event_t;

input_event_t input_getevent(void);
char input_getchar(void);

#endif
