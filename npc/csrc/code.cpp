#include "Vcode_top.h"
#include "nvboard.h"
#include "verilated_vcd_c.h"
#include <assert.h>
#include <cstdint>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  const std::uint64_t sim_time = 400000000;
  
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vcode_top *top = new Vcode_top{contextp};
  
  nvboard_bind_pin(&top->code, 8, SW8, SW7, SW6, SW5, SW4, SW3, SW2, SW1, SW0);
  nvboard_bind_pin(&top->out, 3, LD2, LD1, LD0);
  nvboard_bind_pin(&top->sout, 7, SEG0G,SEG0F, SEG0E, SEG0D, SEG0C, SEG0B, SEG0A);
  
  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC();
  top->trace(m_trace, 99);
  m_trace->open("waveform.vcd");
  nvboard_init();
  top->en = 1;
  top->code = 0b11111111;


  for (;contextp->time() < sim_time && !contextp->gotFinish();) {
    contextp->timeInc(1);
    top->eval();
    m_trace->dump(contextp->time());
    printf("out: %d\n", top->out);
    top->code = top->code >> 1;
    if (top->code == 0) {
      top->code = 0b11111111;
    }
    for (volatile int i = 0;i < 2000000000; ++i);
    nvboard_update();
  }

  m_trace->close();
  nvboard_quit();
}