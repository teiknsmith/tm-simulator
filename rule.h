#ifndef RULE_H
#define RULE_H

#include <stdio.h>

typedef enum { LEFT = -1, STAY = 0, RIGHT = 1 } direction_t;

typedef struct {
  char *from_state;
  char *from_symbol;
  char *to_symbol;
  direction_t direction;
  char *to_state;
} rule_str_t;

typedef struct rule_node {
  rule_str_t rule;
  struct rule_node *next;
} rule_node;

rule_node *read_rules(FILE *input);
void free_rule_nodes(rule_node *head);

#endif