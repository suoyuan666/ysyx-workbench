#include <stdint.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  uint32_t old_rs;
  char exp[128];
  /* TODO: Add more members if necessary */

} WP;

WP* new_wp();
void free_wp(WP *wp);
void print_wp();
