#include "Vcode.h"
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
  Vcode *top = new Vcode{contextp};
  
  nvboard_bind_pin(&top->cod, 8, SW8, SW7, SW6, SW5, SW4, SW3, SW2, SW1, SW0);
  nvboard_bind_pin(&top->out, 3, LD2, LD1, LD0);
  nvboard_bind_pin(&top->sout, 7, SEG0G,SEG0F, SEG0E, SEG0D, SEG0C, SEG0B, SEG0A);
  
  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC();
  top->trace(m_trace, 99);
  m_trace->open("waveform.vcd");
  nvboard_init();
  top->en = 1;
  top->cod = 0b01001100;


  for (;contextp->time() < sim_time && !contextp->gotFinish();) {
    contextp->timeInc(1);
    top->eval();
    m_trace->dump(contextp->time());
    printf("out: %d\n", top->out);
    nvboard_update();
  }

  m_trace->close();
  nvboard_quit();
}