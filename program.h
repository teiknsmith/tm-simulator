#ifndef PROGRAM_H
#define PROGRAM_H

#include "rule.h"

typedef struct {
  int valid;
  unsigned int to_symbol;
  direction_t direction;
  unsigned int to_state;
} transition_t;

typedef struct {
  char *name;
  transition_t *transitions;
  int is_halt;
} state_t;

typedef struct {
  state_t *states;
  unsigned int num_states;
  char *symbols;
  unsigned int num_symbols;
  unsigned int *symbol_idx_lookup;
  unsigned int blank_symbol;
} program_t;

program_t make_program(rule_node *rule_head);
void free_program(program_t *program);

#endif