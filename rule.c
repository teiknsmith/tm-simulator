#include "rule.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "exit_codes.h"

void printrule(const rule_str_t *rule) {
  printf("Rule:: In %s, read %s: write %s, move %d, enter %s\n",
         rule->from_state, rule->from_symbol, rule->to_symbol, rule->direction,
         rule->to_state);
}
void free_rule(const rule_str_t *rule) {
  if (rule->from_state) free(rule->from_state);
  if (rule->from_symbol) free(rule->from_symbol);
  if (rule->to_symbol) free(rule->to_symbol);
  if (rule->to_state) free(rule->to_state);
}

char *next_piece(char *s, char **lscan, char **rscan) {
  while (isspace(**lscan)) (*lscan)++;
  if (!**lscan) return NULL;
  *rscan = *lscan;
  while (!isspace(*++*rscan))
    if (!**rscan) break;
  int piece_len = *rscan - *lscan;
  char *res = malloc(piece_len + 1);
  memcpy(res, *lscan, piece_len);
  res[piece_len] = 0;
  *lscan = *rscan;
  // printf("\tHave a piece! [%s]\n", res);
  return res;
}

int is_empty(char *s) {
  if (*s == ';') return 1;  // comments are 'empty'
  if (!isspace(*s)) return 0;

  char *scan = s;
  while (*++scan)
    if (!isspace(*scan)) return *scan == ';'; // allow comments with spaces before the `;`
  return 1;
}

int has_gte_n_fields(char *s, unsigned int n) {
  unsigned int num_fields = 0;
  char prev_c = ' ';
  char *scan = s;
  while (*scan) {
    if (isspace(*scan) && !isspace(prev_c)) num_fields++;
    if (num_fields == n) return 1;
    prev_c = *(scan++);
  }
  return 0;
}

enum rule_parse_result {
  PARSE_RULE_AOK,
  PARSE_RULE_EMPTY,
  PARSE_ERR_MULTICHAR_SYMBOL,
  PARSE_ERR_EARLY_EOL,
  PARSE_ERR_BAD_DIR
};

enum rule_parse_result storule(char *s, rule_str_t *rule) {
  // printf("Parse this!: %s", s);
  if (is_empty(s)) return PARSE_RULE_EMPTY;
  if (!has_gte_n_fields(s, 5)) return PARSE_ERR_EARLY_EOL;
  // printf("...okay...\n");
  char *lscan = s;
  char *rscan = s;
  char *piece;

  piece = next_piece(s, &lscan, &rscan);
  rule->from_state = piece;

  piece = next_piece(s, &lscan, &rscan);
  rule->from_symbol = piece;
  if (piece[1]) return PARSE_ERR_MULTICHAR_SYMBOL;

  piece = next_piece(s, &lscan, &rscan);
  rule->to_symbol = piece;
  if (piece[1]) return PARSE_ERR_MULTICHAR_SYMBOL;

  piece = next_piece(s, &lscan, &rscan);
  char dir_char = *piece;
  char second_char = piece[1];
  free(piece);
  if (second_char) return PARSE_ERR_BAD_DIR;
  switch (dir_char) {
    case 'l':
      rule->direction = LEFT;
      break;
    case '*':
      rule->direction = STAY;
      break;
    case 'r':
      rule->direction = RIGHT;
      break;
    default:
      return PARSE_ERR_BAD_DIR;
  }

  piece = next_piece(s, &lscan, &rscan);
  rule->to_state = piece;

  // printf("Done parsing: %s", s);
  return PARSE_RULE_AOK;
}

const char *rule_parse_err_string(enum rule_parse_result code) {
  switch (code) {
    case PARSE_RULE_AOK:
      return "No error";
    case PARSE_ERR_MULTICHAR_SYMBOL:
      return "Attempted symbol has multiple characters";
    case PARSE_ERR_EARLY_EOL:
      return "Reached end of line early";
    case PARSE_ERR_BAD_DIR:
      return "Attempted direction is not in \"lr*\"";

    default:
      return "FIXME: No string for that code";
  }
}

rule_node *new_rule_node() {
  rule_node *node = malloc(sizeof(rule_node));
  node->next = NULL;
  node->rule.from_state = NULL;
  node->rule.from_symbol = NULL;
  node->rule.to_symbol = NULL;
  node->rule.to_state = NULL;
}
void free_rule_nodes(rule_node *head) {
  if (head->next) free_rule_nodes(head->next);
  free_rule(&head->rule);
  free(head);
}

const int LINE_BUF_SIZE = 255;

rule_node *read_rules(FILE *input) {
  rule_str_t rule;
  enum rule_parse_result parse_result;
  rule_node *rules_head = new_rule_node();
  rule_node *curr_node = rules_head, *end_node = NULL;

  unsigned int linenum = 0;
  char *line_buf = malloc(LINE_BUF_SIZE);
  while (fgets(line_buf, LINE_BUF_SIZE, input)) {
    linenum++;
    parse_result = storule(line_buf, &(curr_node->rule));
    if (parse_result == PARSE_RULE_AOK) {
      curr_node->next = new_rule_node();
      end_node = curr_node;
      curr_node = curr_node->next;
    } else if (parse_result == PARSE_RULE_EMPTY) {
      continue;
    } else {
      printf("Error: %s\n\ton line %d: %s\n",
             rule_parse_err_string(parse_result), linenum, line_buf);
      exit(EXIT_PARSE_ERR);
    }
  }
  free(line_buf);

  free_rule_nodes(curr_node);
  if (end_node) {
    end_node->next = NULL;
  } else {
    rules_head = NULL;
  }
  return rules_head;
}