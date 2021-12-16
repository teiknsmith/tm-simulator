#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "exit_codes.h"
#include "printtm.h"
#include "program.h"
#include "rule.h"
#include "simulator.h"

#define READING 0
#define RUNNING 1
#define STOPPED 2

int status = READING;

void sigint_handler(int _) {
  printf("In signal handler\n");
  if (status == READING) {
    exit(EXIT_INTERRUPT);
  } else {
    status = STOPPED;
  }
}

int main(int argc, char **argv) {
  signal(SIGINT, sigint_handler);
  if (argc < 4) {
    fprintf(
        stderr,
        "Usage: %s rules_file tape_file|tape init_state [microseconds_pause]\n",
        argv[0]);
    exit(EXIT_USAGE_ERR);
  }
  FILE *rules_in;
  if (!(rules_in = fopen(argv[1], "r"))) {
    fprintf(stderr, "Could not open %s for reading\n", argv[1]);
    exit(EXIT_USAGE_ERR);
  }

  rule_node *rule_llist = read_rules(rules_in);
  fclose(rules_in);
  if (!rule_llist) {
    fprintf(stderr, "No valid rules found\n");
    exit(EXIT_PARSE_ERR);
  }

  program_t program = make_program(rule_llist);

  free_rule_nodes(rule_llist);

  // printf("\n\nALL THE RULES:\n");
  // for (unsigned int i = 0; i < program.num_states; ++i) {
  //   printf("From state %d(\"%s\"):\n", i, program.states[i].name);
  //   for (unsigned int j = 0; j < program.num_symbols; ++j) {
  //     transition_t transition = program.states[i].transitions[j];
  //     printf("\ton %d('%c'):", j, program.symbols[j]);
  //     if (transition.valid) {
  //       printf(" write %d('%c') move %d enter %d(\"%s\")\n",
  //              transition.to_symbol, program.symbols[transition.to_symbol],
  //              transition.direction, transition.to_state,
  //              program.states[transition.to_state].name);
  //     } else {
  //       printf(" no such rule\n");
  //     }
  //   }
  // }
  // printf("Num:: states:%d, symbols:%d\n", program.num_states,
  //        program.num_symbols);

  FILE *tape_in;
  char *init_tape;
  if (tape_in = fopen(argv[2], "r")) {
    fseek(tape_in, 0, SEEK_END);
    long tape_max_len = ftell(tape_in);
    fseek(tape_in, 0, SEEK_SET);
    init_tape = malloc(tape_max_len + 1);
    fread(init_tape, tape_max_len, 1, tape_in);
    fclose(tape_in);

    init_tape[tape_max_len] = 0;
    for (unsigned int i = 0; i < tape_max_len; ++i) {
      if (init_tape[i] == '\n') {
        init_tape[i] = 0;
      }
    }
  } else {
    // just use the command line argument as the tape
    init_tape = argv[2];
  }

  char *init_state_name = argv[3];

  program_state_t *running_program =
      start_program(&program, init_tape, init_state_name);
  if (init_tape != argv[2]) free(init_tape);

  int micros_to_sleep = 1000;
  if (argc >= 5) {
    micros_to_sleep = atoi(argv[4]);
  }

  status = RUNNING;
  fancy_printer_t *printer = new_fancy_printer(running_program);
  while (!is_halted(running_program) && status != STOPPED) {
    // if (!(running_program->step_num % 500)) {
    //   fancy_print(printer);
    //   usleep(10);
    // }
    if (micros_to_sleep) {
      fancy_print(printer);
      usleep(micros_to_sleep);
    }
    step(running_program);
  }
  fancy_print(printer);

  free_fancy_printer(printer);
  free_program_state(running_program);
  free_program(&program);

  printf("\n");

  return EXIT_AOK;
}