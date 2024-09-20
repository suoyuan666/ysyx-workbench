#include "Vshift.h"
#include "nvboard.h"
#include "verilated_vcd_c.h"
#include <assert.h>
// #include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  const std::uint64_t sim_time = 200000000;
  
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vshift *top = new Vshift{contextp};
  
  nvboard_bind_pin(&top->source, 8, SW7, SW6, SW5, SW4, SW3, SW2, SW1, SW0);
  nvboard_bind_pin(&top->rs, 8, SW15, SW14, SW13, SW12, SW11, SW10, SW9, SW8);
  
  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC();
  top->trace(m_trace, 99);
  m_trace->open("waveform.vcd");
  nvboard_init();

  top->source = 0b00000001;

  for (;contextp->time() < sim_time && !contextp->gotFinish();) {
    contextp->timeInc(1);
    top->eval();
    m_trace->dump(contextp->time());
    // for (volatile int i = 0;i < 2000000000; ++i);
    nvboard_update();
  }

  m_trace->close();
  nvboard_quit();
}