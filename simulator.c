#include "simulator.h"

#include <stdlib.h>
#include <string.h>

#include "exit_codes.h"

program_state_t *start_program(program_t *program, char *init_tape,
                               char *init_state_name) {
  program_state_t *res = malloc(sizeof(program_state_t));

  res->program = program;

  unsigned int start_state = 0;
  for (; start_state < program->num_states; ++start_state) {
    if (!strcmp(init_state_name, program->states[start_state].name)) break;
  }
  if (start_state == program->num_states) {
    fprintf(stderr, "Start state \"%s\" not found in any rules\n",
            init_state_name);
    exit(EXIT_USER_LOGIC_ERR);
  }
  res->state = start_state;

  res->tape_idx = 0;

  res->tape = new_tape();
  for (unsigned int idx = 0; init_tape[idx]; ++idx) {
    if (init_tape[idx] == '*') {
      res->tape_idx = idx;
    } else {
      unsigned int symbol_idx = program->symbol_idx_lookup[init_tape[idx]];
      if (!symbol_idx) {
        fprintf(stderr,
                "Unknown symbol '%c' on input tape (no rules involving this "
                "symbol declared)\n",
                init_tape[idx]);
        exit(EXIT_USER_LOGIC_ERR);
      }
      tape_set(res->tape, idx, symbol_idx);
    }
  }

  res->step_num = 0;

  return res;
}
void free_program_state(program_state_t *program_state) {
  free_tape(program_state->tape);
  free(program_state);
}
void step(program_state_t *program_state) {
  program_state->step_num++;
  unsigned int read_symbol =
      tape_at(program_state->tape, program_state->tape_idx,
              program_state->program->blank_symbol);
  transition_t transition = program_state->program->states[program_state->state]
                                .transitions[read_symbol];
  if (!transition.valid)
    transition =
        program_state->program->states[program_state->state].transitions[0];
  if (!transition.valid) {
    fprintf(stderr, "No rule found for state:\"%s\" reading symbol:'%c'\n",
            program_state->program->states[program_state->state].name,
            program_state->program->symbols[read_symbol]);
    exit(EXIT_USER_LOGIC_ERR);
  }

  if (transition.to_symbol)
    tape_set(program_state->tape, program_state->tape_idx,
             transition.to_symbol);

  if (transition.to_state) program_state->state = transition.to_state;

  switch (transition.direction) {
    case LEFT:
      program_state->tape_idx--;
      break;
    case STAY:
      break;
    case RIGHT:
      program_state->tape_idx++;
      break;

    default:
      fprintf(stderr, "During step, unknown direction encountered\n");
      exit(EXIT_INTERNAL_LOGIC_ERR);
  }
}
int is_halted(program_state_t *program_state) {
  return program_state->program->states[program_state->state].is_halt;
}
