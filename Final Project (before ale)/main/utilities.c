#include <xc.h>
#include "utilities.h"

static volatile enum mode_t mode = IDLE;

void set_mode(enum mode_t new_mode) {
  mode = new_mode;
}

enum mode_t get_mode(void) {
  return mode;
}
