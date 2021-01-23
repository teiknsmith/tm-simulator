#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "program.h"
#include "tape.h"

typedef struct {
  program_t *program;
  unsigned int state;
  tape_t *tape;
  int tape_idx;
  unsigned int step_num;
} program_state_t;

program_state_t *start_program(program_t *program, char *init_tape,
                               char *init_state_name);
void free_program_state(program_state_t *program_state);
void step(program_state_t *program_state);
int is_halted(program_state_t *program_state);

#endif