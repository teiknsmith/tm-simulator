#ifndef TAPE_H
#define TAPE_H

#define TAPE_BLOCK_SIZE 256

typedef struct {
  unsigned int *cells;
  unsigned int num_available_cells;
  unsigned int num_used_cells;
} tape_ray_t;

typedef struct {
  tape_ray_t neg_tape;
  tape_ray_t pos_tape;
} tape_t;

tape_t *new_tape();
void free_tape(tape_t *tape);
unsigned int tape_at(tape_t *tape, int idx, unsigned int blank_val);
void tape_set(tape_t *tape, int idx, unsigned int val);

#endif