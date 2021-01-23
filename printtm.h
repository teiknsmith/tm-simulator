#ifndef PRINTTM_H
#define PRINTTM_H

#include "simulator.h"

typedef struct {
  int left_idx;
  int tape_idx;
  int screen_width;
} fancy_data_t;

typedef struct {
  program_state_t *program_state;
  fancy_data_t last_data;
} fancy_printer_t;

fancy_printer_t *new_fancy_printer(program_state_t *program_state);
void free_fancy_printer(fancy_printer_t *printer);
void fancy_print(fancy_printer_t *printer);

#endif