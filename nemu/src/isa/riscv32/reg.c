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

#include <string.h>
#include <stdio.h>
#include <isa.h>
#include "local-include/reg.h"
#include "common.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  printf("$pc: %x\n", cpu.pc);
  for (int i = 0; i < 32; i+=4) {
    printf("%s: %x, %s: %x, %s: %x, %s: %x\n", 
          regs[i], cpu.gpr[i], 
          regs[i + 1], cpu.gpr[i + 1],
          regs[i + 2], cpu.gpr[i + 2], 
          regs[i + 3], cpu.gpr[i + 3]);
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int index = 0;
  int rs = 0;
  for (index = 0; index < 32; ++index) {
    if (strcmp(regs[index], s) == 0) {
      *success = true;
      break;
    }
  }
  rs = cpu.gpr[index];
  if (strcmp(s, "pc") == 0){
    rs = cpu.pc;
    *success = true;
  }
  
  return rs;
}
