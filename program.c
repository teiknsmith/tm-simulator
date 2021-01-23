#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tree_node {
  char *name;
  struct tree_node *left;
  struct tree_node *right;
  unsigned int idx;
};
struct tree_node *new_tree_node(char *name) {
  struct tree_node *res = malloc(sizeof(struct tree_node));
  res->name = name;
  res->left = NULL;
  res->right = NULL;
  res->idx = 0;
  return res;
}
void free_tree(struct tree_node *root) {
  if (root->left) free_tree(root->left);
  if (root->right) free_tree(root->right);
  free(root);
}

// void print_tree(struct tree_node *head, unsigned int depth) {
//   if (!head) return;
//   //   printf(" tree_node \"%s\"[%d] < %p > %p\n", head->name, head->idx,
//   //   head->left,
//   //          head->right);
//   for (unsigned int i = 0; i < depth; ++i) {
//     printf("\t");
//   }
//   printf("%s[%d]\n", head->name, head->idx);
//   print_tree(head->right, depth + 1);
//   print_tree(head->left, depth + 1);
// }
void tree_add(struct tree_node *head, char *new_name) {
  int cmp = strcmp(head->name, new_name);
  if (!cmp) return;
  struct tree_node **child = cmp < 0 ? &head->left : &head->right;
  if (!*child) {
    *child = new_tree_node(new_name);
  } else {
    tree_add(*child, new_name);
  }
}

void tree_idxify_r(struct tree_node *head, unsigned int *idx) {
  if (!head) return;
  head->idx = (*idx)++;
  tree_idxify_r(head->left, idx);
  tree_idxify_r(head->right, idx);
}
unsigned int tree_idxify(struct tree_node *head) {
  unsigned int idx = 0;
  tree_idxify_r(head, &idx);
  return idx;
}

void fill_symbol_arr_from_tree(struct tree_node *head, char *arr) {
  if (!head) return;
  arr[head->idx] = *head->name;
  fill_symbol_arr_from_tree(head->left, arr);
  fill_symbol_arr_from_tree(head->right, arr);
}
void fill_state_arr_names_from_tree(struct tree_node *head, state_t *arr) {
  if (!head) return;
  if (!arr[head->idx].name) {
    size_t name_len = strlen(head->name);
    arr[head->idx].name = malloc(name_len + 1);
    strcpy(arr[head->idx].name, head->name);
    arr[head->idx].is_halt = !strncmp("halt", head->name, 4);
  }
  fill_state_arr_names_from_tree(head->left, arr);
  fill_state_arr_names_from_tree(head->right, arr);
}
unsigned int symbol_idx_in_tree(struct tree_node *tree, char find_me) {
  if (!tree) return -1;
  if (*tree->name == find_me) return tree->idx;
  return symbol_idx_in_tree(*tree->name < find_me ? tree->left : tree->right,
                            find_me);
}
unsigned int state_name_idx_in_tree(struct tree_node *tree, char *find_me) {
  if (!tree) return -1;
  int cmp = strcmp(tree->name, find_me);
  if (!cmp) return tree->idx;
  return state_name_idx_in_tree(cmp < 0 ? tree->left : tree->right, find_me);
}
void populate_transitions(state_t *arr, unsigned int *symbol_rlookup,
                          struct tree_node *state_name_tree,
                          rule_node *rule_head) {
  rule_node *curr_rule_node = rule_head;
  rule_str_t *curr_rule;
  while (curr_rule_node) {
    curr_rule = &curr_rule_node->rule;
    unsigned int from_state_idx =
        state_name_idx_in_tree(state_name_tree, curr_rule->from_state);
    unsigned int from_symbol_idx = symbol_rlookup[*curr_rule->from_symbol];
    transition_t *transition =
        &arr[from_state_idx].transitions[from_symbol_idx];

    unsigned int to_state_idx =
        state_name_idx_in_tree(state_name_tree, curr_rule->to_state);
    unsigned int to_symbol_idx = symbol_rlookup[*curr_rule->to_symbol];
    transition->to_symbol = to_symbol_idx;
    transition->to_state = to_state_idx;
    transition->direction = curr_rule->direction;
    transition->valid = 1;

    curr_rule_node = curr_rule_node->next;
  }
}
char max_symbol_in_tree(struct tree_node *tree) {
  return tree->left ? max_symbol_in_tree(tree->left) : *tree->name;
}

program_t make_program(rule_node *rule_head) {
  struct tree_node *state_head = new_tree_node("*");
  struct tree_node *symbol_head = new_tree_node("*");
  tree_add(symbol_head, "_");

  rule_node *curr_rule_node = rule_head;
  while (curr_rule_node) {
    tree_add(state_head, curr_rule_node->rule.from_state);
    tree_add(state_head, curr_rule_node->rule.to_state);
    tree_add(symbol_head, curr_rule_node->rule.from_symbol);
    tree_add(symbol_head, curr_rule_node->rule.to_symbol);

    curr_rule_node = curr_rule_node->next;
  }

  unsigned int num_states = tree_idxify(state_head);
  unsigned int num_symbols = tree_idxify(symbol_head);

  //   print_tree(symbol_head, 0);

  char *symbols = malloc(num_symbols);
  fill_symbol_arr_from_tree(symbol_head, symbols);
  unsigned int *symbol_rlookup =
      calloc(1 + max_symbol_in_tree(symbol_head), sizeof(unsigned int));
  for (unsigned int i = 0; i < num_symbols; ++i) {
    symbol_rlookup[symbols[i]] = symbol_idx_in_tree(symbol_head, symbols[i]);
  }
  state_t *states = malloc(num_states * sizeof(state_t));
  for (unsigned int i = 0; i < num_states; ++i) {
    states[i].name = NULL;
    states[i].transitions = malloc(num_symbols * sizeof(transition_t));
    for (unsigned int j = 0; j < num_symbols; ++j) {
      states[i].transitions[j].valid = 0;
    }
  }
  fill_state_arr_names_from_tree(state_head, states);
  populate_transitions(states, symbol_rlookup, state_head, rule_head);

  free_tree(symbol_head);
  free_tree(state_head);

  unsigned int blank_symbol = symbol_rlookup['_'];

  program_t res = {states,      num_states,     symbols,
                   num_symbols, symbol_rlookup, blank_symbol};
  return res;
}

void free_program(program_t *program) {
  for (unsigned int i = 0; i < program->num_states; ++i) {
    free(program->states[i].transitions);
    free(program->states[i].name);
  }
  free(program->states);
  free(program->symbols);
  free(program->symbol_idx_lookup);
}