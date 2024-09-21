#include "Vkeyboard.h"
#include "nvboard.h"
#include "verilated_vcd_c.h"
#include <assert.h>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void single_cycle(Vkeyboard *top) {
  top->clk = 0;
  top->eval();
  top->clk = 1;
  top->eval();
}

int main(int argc, char *argv[]) {
  const std::uint64_t sim_time = 200000000;

  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vkeyboard *top = new Vkeyboard{contextp};

  nvboard_bind_pin(&top->code, 14, SEG1G, SEG1F, SEG1E, SEG1D, SEG1C, SEG1B,
                   SEG1A, SEG0G, SEG0F, SEG0E, SEG0D, SEG0C, SEG0B, SEG0A);
  nvboard_bind_pin(&top->ascii_code, 28, SEG5G, SEG5F, SEG5E, SEG5D, SEG5C,
                   SEG5B, SEG5A, SEG4G, SEG4F, SEG4E, SEG4D, SEG4C, SEG4B,
                   SEG4A, SEG3G, SEG3F, SEG3E, SEG3D, SEG3C, SEG3B, SEG3A,
                   SEG2G, SEG2F, SEG2E, SEG2D, SEG2C, SEG2B, SEG2A);
  nvboard_bind_pin(&top->total, 14, SEG7G, SEG7F, SEG7E, SEG7D, SEG7C, SEG7B,
                   SEG7A, SEG6G, SEG6F, SEG6E, SEG6D, SEG6C, SEG6B, SEG6A);
  nvboard_bind_pin(&top->ps2_clk, 1, PS2_CLK);
  nvboard_bind_pin(&top->ps2_data, 1, PS2_DAT);

  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC();
  top->trace(m_trace, 99);
  m_trace->open("waveform.vcd");
  nvboard_init();
  top->clrn = 1;

  for (; contextp->time() < sim_time && !contextp->gotFinish();) {
    contextp->timeInc(1);
    single_cycle(top);
    m_trace->dump(contextp->time());
    // for (volatile int i = 0;i < 2000000000; ++i);
    // if (top->ascii_src != 0) {
    //   printf("top->ascii_src: %x\n", top->ascii_src);
    // }
    nvboard_update();
  }

  m_trace->close();
  nvboard_quit();
}
