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

#include "common.h"
#include "debug.h"
#include "memory/paddr.h"
#include <stddef.h>
#include <stdint.h>
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_BCD_NUMBER, TK_HEX_NUMBER, TK_NEG_NUM,
  TK_NEXT_LINE, TK_REG, TK_PTR,
  TK_CMP_LESS, TK_CMP_MORE, 
  TK_LOGIC_OR, TK_LOGIC_AND, TK_LOGIC_NOT,
  TK_SHIFT_LEFT, TK_SHIFT_RIGHT,
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {")", ')'},
  {"0x[0-9,a-f,A-F]+", TK_HEX_NUMBER}, // hex number
  {"[0-9]+", TK_BCD_NUMBER},   // bcd number
  {"<<", TK_SHIFT_LEFT},
  {">>", TK_SHIFT_RIGHT},
  {"\\$\\w*", TK_REG},
  {">", TK_CMP_MORE},
  {"<", TK_CMP_LESS},
  {"&&", TK_LOGIC_AND},
  {"\\|\\|", TK_LOGIC_OR},
  {"!", TK_LOGIC_NOT},
  {"\\\n", TK_NEXT_LINE},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;
static int token_index __attribute__((used)) = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        switch (rules[i].token_type) {
          case TK_NOTYPE:
          case TK_NEXT_LINE:
            break;
          case TK_HEX_NUMBER:
            tokens[token_index].type = rules[i].token_type;
            uint64_t base = 1;
            uint64_t hex_src = 0;
            for (substr_len -= 1; substr_len > 1; --substr_len) {
              int tmp = 0;
              if (substr_start[substr_len] >= '0' &&
                  substr_start[substr_len] <= '9') {
                tmp = (substr_start[substr_len] - '0') * base;
              } else if (substr_start[substr_len] >= 'a' &&
                         substr_start[substr_len] <= 'f') {
                tmp = (substr_start[substr_len] - 'a' + 10) * base;
              } else if (substr_start[substr_len] >= 'A' &&
                         substr_start[substr_len] <= 'F') {
                tmp = (substr_start[substr_len] - 'A' + 10) * base;
              } else {
                Assert(0,
                       "make_token: TK_HEX_NUMBER error, %%c: %c %%d: %d "
                       "substr_len: %d\n",
                       substr_start[substr_len], substr_start[substr_len],
                       substr_len);
              }
              base *=  16;
              hex_src += tmp;
            }
            int len = 1;
            for (int index = hex_src; index /= 10 != 0; ++len);
            Assert(len < sizeof(tokens[0].str) , "tokens overflow, type: %d, str: %s\n",
                   rules[i].token_type, tokens[token_index].str);
            snprintf(tokens[token_index].str, sizeof(tokens[0].str) , "%ld", hex_src);
            ++token_index;
            break;
          default:
            tokens[token_index].type = rules[i].token_type;
            Assert(substr_len < sizeof(tokens[0].str) , "tokens overflow, type: %d, str: %s\n",
                   rules[i].token_type, tokens[token_index].str);
            for (int index = 0; index < sizeof(tokens[0].str) && index < substr_len; ++index) {
              tokens[token_index].str[index] = substr_start[index];
            }
            ++token_index;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int expr_core(const int prev, const int next);
bool check_parentheses(const int prev, const int next);
bool check_negative(const int prev, const int next);
bool check_register(const int prev, const int next);

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  int len = 0;
  for (; tokens[len].type != 0; len += 1) {
    if (tokens[len].type == '-' &&
        (len == 0 || tokens[len - 1].type == '+' ||
         tokens[len - 1].type == '-' || tokens[len - 1].type == '*' ||
         tokens[len - 1].type == '/' || tokens[len - 1].type == TK_NEG_NUM)) {
      tokens[len].type = TK_NEG_NUM;
    }

    if (tokens[len].type == '*' &&
        (len == 0 || (tokens[len - 1].type != TK_BCD_NUMBER &&
                      tokens[len - 1].type != TK_HEX_NUMBER))) {
      tokens[len].type = TK_PTR;
    }
  }

  int rs = expr_core(0, len - 1);
  memset((char*)tokens, '\0', sizeof(tokens));
  token_index = 0;

  return rs;
}

int expr_core(const int prev,const int next) {
  if (prev > next) {
    return 0;
  } else if (prev == next) {
    if (tokens[prev].type == TK_REG) {
      bool flag = false;
      int rs = isa_reg_str2val(tokens[prev].str+1, &flag);
      Assert(flag, "expr_core: check_register() -> isa_reg_str2val() error, rs: %d\n", rs);
      return rs;
    }
    return atoi(tokens[prev].str);
  } else if (check_parentheses(prev, next) == true) {
    return expr_core(prev + 1, next - 1);
  } else if (check_negative(prev, next) == true) {
    if ((next - prev) >= 2 && (next - prev) % 2 == 0) {
      return atoi(tokens[next].str);
    } else {
      int rs = atoi(tokens[next].str);
      return -rs;
    }
  } else {
    struct {
      int index;
      char kind; // 1: 一元运算符, 0: 二元运算符
    } op = {0, 0};

    bool fetch_blcok = false;
    bool fetch_imul = false;
    bool fetch_add_sub = false;
    bool fetch_shift = false;
    bool fetch_compare = false;
    bool fetch_compre_eq = false;
    bool fetch_logic_and = false;
    bool fetch_logic_or = false;

    for (int i = prev; i <= next; ++i) {
      if (tokens[i].type == '(') {
        fetch_blcok = true;
      } else if (tokens[i].type == ')') {
        fetch_blcok = false;
      } else if (!fetch_blcok && tokens[i].type == TK_LOGIC_OR) {
        op.index = i;
        fetch_logic_or = true;
      } else if (!fetch_blcok && !fetch_logic_or &&
                 tokens[i].type == TK_LOGIC_AND) {
        op.index = i;
        fetch_logic_and = true;
      } else if (!fetch_blcok && !fetch_logic_or && !fetch_logic_and &&
                 (tokens[i].type == TK_CMP_LESS ||
                  tokens[i].type == TK_CMP_MORE)) {
        op.index = i;
        fetch_compare = true;
      } else if (!fetch_blcok && !fetch_logic_or && !fetch_logic_and &&
                 tokens[i].type == TK_EQ) {
        op.index = i;
        fetch_compre_eq = true;
      } else if (!fetch_blcok && !fetch_logic_or && !fetch_logic_and &&
                 !fetch_compare && fetch_compre_eq &&
                 (tokens[i].type == TK_SHIFT_LEFT ||
                  tokens[i].type == TK_SHIFT_RIGHT)) {
        op.index = i;
        fetch_shift = true;
      } else if (!fetch_blcok && !fetch_logic_or && !fetch_logic_and &&
                 !fetch_compare && fetch_compre_eq && !fetch_shift &&
                 (tokens[i].type == '+' || tokens[i].type == '-')) {
        op.index = i;
        fetch_add_sub = true;
      } else if (!fetch_blcok && !fetch_logic_or && !fetch_logic_and &&
                 !fetch_compare && fetch_compre_eq && !fetch_shift &&
                 !fetch_add_sub &&
                 (tokens[i].type == '*' || tokens[i].type == '/')) {
        op.index = i;
        fetch_imul = true;
      } else if (!fetch_blcok && !fetch_logic_or && !fetch_logic_and &&
                 !fetch_compare && fetch_compre_eq && !fetch_shift &&
                 !fetch_add_sub && !fetch_imul &&
                 (tokens[i].type == TK_LOGIC_NOT || tokens[i].type == TK_PTR)) {
        op.index = i;
        op.kind = 1;
      }
    }

    int val1 = 0;
    int val2 = 0;

    if (op.kind == 0) {
      val1 = expr_core(prev, op.index - 1);
      val2 = expr_core(op.index + 1, next);
    } else {
      val2 = expr_core(op.index + 1, next);
    }

    switch (tokens[op.index].type) {
      case '+':
        return val1 + val2;
        break;
      case '-':
        return val1 - val2;
        break;
      case '*':
        return val1 * val2;
        break;
      case '/':
        return val1 / val2;
        break;
      case TK_CMP_LESS:
        return (val1 < val2) ? 1 : 0;
      case TK_CMP_MORE:
        return (val1 > val2) ? 1 : 0;
      case TK_EQ:
        return (val1 == val2) ? 1 : 0;
      case TK_LOGIC_AND:
        return (val1 && val2) ? 1 : 0;
      case TK_LOGIC_OR:
        return (val1 || val2) ? 1 : 0;
      case TK_LOGIC_NOT:
        return (val2 == 0) ? 1 : 0;
      case TK_SHIFT_LEFT:
        return val1 << val2;
      case TK_SHIFT_RIGHT:
        return val1 >> val2;
      case TK_PTR:
        return paddr_read(val2, 1);
      default:
        Assert(
            0,
            "expr_core: swith case a wrong type: %d: %c, prev: %d, next: %d\n",
            op.index, tokens[op.index].type, prev, next);
        break;
    }
  }
}

bool check_parentheses(const int prev, const int next) {
  if (tokens[prev].str[0] == '(' && tokens[next].str[0] == ')') {
    if (prev + 1 != next && next - 1 > 0) {
      for (int i = prev + 1; i < next; ++i) {
        if (tokens[i].str[0] == ')') {
          return false;
        }
      }
    }
    return true;
  }
  return false;
}

bool check_negative(const int prev, const int next) {
  for (int i = prev; i <= next; ++i) {
    if (tokens[i].type != TK_NEG_NUM && tokens[i].type != TK_BCD_NUMBER &&
        tokens[i].type != TK_HEX_NUMBER) {
      return false;
    }
  }
  return true;
}