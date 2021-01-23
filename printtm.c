#include "printtm.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct winsize screen_win_size;

fancy_data_t init_fancy_data(program_state_t *program_state) {
  int lowest_used_idx = -program_state->tape->neg_tape.num_used_cells;
  int tape_idx = program_state->tape_idx;
  ioctl(0, TIOCGWINSZ, &screen_win_size);
  int screen_width = screen_win_size.ws_col;

  int left_index;
  if (tape_idx <= lowest_used_idx) {
    left_index = tape_idx;
  } else {
    int min_left_index = tape_idx - (screen_width / 2);
    left_index =
        min_left_index > lowest_used_idx ? min_left_index : lowest_used_idx;
  }
  fancy_data_t res = {left_index, tape_idx, screen_width};
  return res;
}

fancy_data_t update_fancy_data(program_state_t *program_state,
                               fancy_data_t old_data) {
  const int MIN_CHARS_TO_LEFT = 3;
  fancy_data_t new_data = init_fancy_data(program_state);
  if (new_data.tape_idx >= old_data.left_idx) {
    if (new_data.tape_idx < old_data.left_idx + MIN_CHARS_TO_LEFT &&
        new_data.tape_idx < old_data.tape_idx) {
      new_data.left_idx =
          old_data.left_idx - (old_data.tape_idx - new_data.tape_idx);
    } else if (new_data.tape_idx <
               old_data.left_idx + (new_data.screen_width / 2)) {
      new_data.left_idx = old_data.left_idx;
    }
  }
  return new_data;
}

fancy_printer_t *new_fancy_printer(program_state_t *program_state) {
  fancy_printer_t *res = malloc(sizeof(fancy_printer_t));
  res->program_state = program_state;
  res->last_data = init_fancy_data(program_state);
  return res;
}
void free_fancy_printer(fancy_printer_t *printer) {
  free(printer);
  printf("\e[?25h");  // unhide the cursor
}
char printing_symbol_at(fancy_printer_t *printer, int idx) {
  return printer->program_state->program
      ->symbols[tape_at(printer->program_state->tape, idx,
                        printer->program_state->program->blank_symbol)];
}
void fancy_print(fancy_printer_t *printer) {
  fancy_data_t new_data =
      update_fancy_data(printer->program_state, printer->last_data);
  // screen coordinates start at 1 apparently
  int tape_head_cursor_idx = new_data.tape_idx - new_data.left_idx + 1;
  char *state_name =
      printer->program_state->program->states[printer->program_state->state]
          .name;
  printf(
      "\e[?25l"                          // hide the cursor
      "\e[2J\e[1;1H"                     // clear and move to top left
      "Steps: %d\n"                      // num steps
      "State: \"%s\"\n"                  // state
      "\e[%dGv\n"                        // tape head
      ,                                  // ^^ fstrs ^^ vv data vv
      printer->program_state->step_num,  // num steps
      state_name,                        // state name
      tape_head_cursor_idx               // tape head
  );
  for (int i = new_data.left_idx; i < new_data.left_idx + new_data.screen_width;
       ++i) {
    printf("%c", printing_symbol_at(printer, i));
  }
  printf("\n\e[%dG^\n", tape_head_cursor_idx);
  printer->last_data = new_data;
}
