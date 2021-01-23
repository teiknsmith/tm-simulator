#include "tape.h"

#include <stdio.h>
#include <stdlib.h>

#include "exit_codes.h"

tape_ray_t new_tape_ray() {
  tape_ray_t res;
  res.cells = malloc(TAPE_BLOCK_SIZE * sizeof(unsigned int));
  res.num_available_cells = TAPE_BLOCK_SIZE;
  res.num_used_cells = 0;
  return res;
}
tape_t *new_tape() {
  tape_t *res = malloc(sizeof(tape_t));
  res->neg_tape = new_tape_ray();
  res->pos_tape = new_tape_ray();
  return res;
}
void free_tape(tape_t *tape) {
  free(tape->neg_tape.cells);
  free(tape->pos_tape.cells);
  free(tape);
}

void check_idx(tape_ray_t *tape, unsigned int idx) {
  if (idx >= tape->num_available_cells) {
    if (!(tape->cells = realloc(tape->cells,
                                (tape->num_available_cells + TAPE_BLOCK_SIZE) *
                                    sizeof(unsigned int)))) {
      fprintf(stderr, "We ran out of memory... :(\n");
      exit(EXIT_RUNTIME_ERR);
    }
    tape->num_available_cells += TAPE_BLOCK_SIZE;
  }
}
unsigned int tape_ray_at(tape_ray_t *tape, unsigned int idx,
                         unsigned int blank_val) {
  check_idx(tape, idx);
  if (idx == tape->num_used_cells) {
    tape->cells[idx] = blank_val;
    tape->num_used_cells++;
    return blank_val;
  } else if (idx > tape->num_used_cells) {
    fprintf(stderr,
            "Uh-oh, somebody skipped a cell.\nThat shouldn't happen.\n");
    exit(EXIT_INTERNAL_LOGIC_ERR);
  }
  return tape->cells[idx];
}
void tape_ray_set(tape_ray_t *tape, unsigned int idx, unsigned int val) {
  check_idx(tape, idx);
  if (idx == tape->num_used_cells) {
    tape->num_used_cells++;
  } else if (idx > tape->num_used_cells) {
    fprintf(stderr,
            "Uh-oh, somebody skipped a cell.\nThat shouldn't happen.\n");
    exit(EXIT_INTERNAL_LOGIC_ERR);
  }
  tape->cells[idx] = val;
}

unsigned int tape_at(tape_t *tape, int idx, unsigned int blank_val) {
  return idx < 0 ? tape_ray_at(&tape->neg_tape, -idx - 1, blank_val)
                 : tape_ray_at(&tape->pos_tape, idx, blank_val);
}
void tape_set(tape_t *tape, int idx, unsigned int val) {
  if (idx < 0)
    tape_ray_set(&tape->neg_tape, -idx - 1, val);
  else
    tape_ray_set(&tape->pos_tape, idx, val);
}