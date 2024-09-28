/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  uint32_t old_rs;
  char exp[128];
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp() {
  WP* tmp = NULL;
  if (free_ != NULL) {
    tmp = head;
    if (tmp == NULL) {
      tmp = (WP*)wp_pool;
      head = tmp;
    } else {
      while (tmp->next != NULL);
      tmp->next = (WP*)wp_pool;
    }
    *wp_pool = *(((WP*)wp_pool)->next);
  }
  return tmp;
}

void free_wp(WP *wp) {
  if (wp != NULL) {
    WP *tmp = wp_pool;
    for (WP *hd = head; hd != NULL; hd = hd->next) {
      if (hd->next == wp) {
        hd->next = wp->next;
      }
    }
    while(tmp->next != NULL);
    tmp->next = wp;
  }
}

void print_wp() {
  for (WP *p = head; p != NULL && p->exp[0] != '\0'; p = p->next) {
    printf("num: %d, exp: %s, old_rs: %d\n", p->NO, p->exp, p->old_rs);
  }
}

 WP* check_diff(int *new_rs) {
  for (WP* p = head; p != NULL; p = p->next) {
    bool rs = true;
    *new_rs = expr(p->exp, &rs);
    if (p->old_rs != *new_rs) {
      return p;
    }
  }
  return NULL;
}
/* TODO: Implement the functionality of watchpoint */

