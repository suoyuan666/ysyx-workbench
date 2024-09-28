/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[16] = {};
// static char code_buf[512 + 128] = {}; // a little larger than `buf`
// static char *code_format =
// "#include <stdio.h>\n"
// "int main() { "
// "  unsigned result = %s; "
// "  printf(\"%%u\", result); "
// "  return 0; "
// "}";

const static char bcd_num[10] = {'0', '1', '2', '3', '4',
                                 '5', '6', '7', '8', '9'};
const static char op_list[4] = {'+', '-', '*', '/'};

int buf_index = 0;
int block_num = 0;

int get_random(int max) {
  FILE *fp = fopen("/dev/random", "r");
  if (fp == NULL) {
    perror("Failed to open /dev/random");
    return EXIT_FAILURE;
  }
  unsigned int randomValue;
  size_t readBytes = fread(&randomValue, sizeof(randomValue), 1, fp);
  if (readBytes != 1) {
    perror("Failed to read from /dev/urandom");
    fclose(fp);
    return EXIT_FAILURE;
  }
  fclose(fp);
  return randomValue % max;
}

static void gen_rand_expr() {
  switch (get_random(3)) {
  case 0:
    num:
    if (buf_index + block_num + 1 < sizeof(buf)) {
      buf[buf_index] = bcd_num[get_random(sizeof(bcd_num))];
      if (buf_index != 0 && buf[buf_index - 1] == '/') {
        while (buf[buf_index] == '0') {
          buf[buf_index] = bcd_num[get_random(sizeof(bcd_num))];
        }
      }
      buf_index += 1;
    } else if (buf[buf_index - 1] == '+' || buf[buf_index - 1] == '-' ||
               buf[buf_index - 1] == '*' || buf[buf_index - 1] == '/') {
      buf[buf_index - 1] = '\0';
      buf_index -= 1;
    }
    break;
  case 1:
    if (buf_index + block_num + 1 < sizeof(buf) &&
        (buf_index == 0 || buf[buf_index - 1] != '(')) {
      buf[buf_index] = '(';
      buf_index += 1;
      block_num += 1;
    } else if (buf[buf_index - 1] == '+' || buf[buf_index - 1] == '-' ||
               buf[buf_index - 1] == '*' || buf[buf_index - 1] == '/') {
      buf[buf_index - 1] = '\0';
      buf_index -= 1;
      return;
    } else if (buf[buf_index - 1] == '(') {
      goto num;
      return;
    }

    gen_rand_expr();

    if (buf_index + block_num + 1 < sizeof(buf)) {
      buf[buf_index] = ')';
      buf_index += 1;
      block_num -= 1;
    } else {
      for (int j = 0; j < block_num; ++j) {
        int fetch_block = 0;
        for (int i = buf_index; i >= 0; --i) {
          if (buf[i] == ')') {
            fetch_block += 1;
          }
          if (buf[i] == '(') {
            if (fetch_block == 1) {
              if (i != 0 && (buf[i - 1] == '+' || buf[i - 1] == '-' ||
                             buf[i - 1] == '*' || buf[i - 1] == '/')) {
                buf[i - 1] = '\0';
                buf_index = i - 1;
              } else {
                buf_index = i;
              }
              buf[i] = '\0';
              break;
            } else {
              fetch_block -= 1;
            }
          }
          buf[i] = '\0';
        }
      }
      block_num = 0;
      return;
    }
    break;
  default:
    gen_rand_expr();

    if (buf_index + block_num + 2 < sizeof(buf)) {
      buf[buf_index] = op_list[get_random(sizeof(op_list))];
      buf_index += 1;
    } else {
      return;
    }

    gen_rand_expr();
    break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
    if (buf[0] == '\0') {
      continue;
    }
    printf("%s\n", buf);

    // sprintf(code_buf, code_format, buf);

    // FILE *fp = fopen("/tmp/.code.c", "w");
    // assert(fp != NULL);
    // fputs(code_buf, fp);
    // fclose(fp);

    // int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    // if (ret != 0) continue;

    // fp = popen("/tmp/.expr", "r");
    // assert(fp != NULL);

    // int result;
    // ret = fscanf(fp, "%d", &result);
    // pclose(fp);

    // printf("%u %s\n", result, buf);
    memset(buf, '\0', sizeof(buf));
    buf_index = 0;
    block_num = 0;
  }
  return 0;
}
