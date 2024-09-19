#include "Valu.h"
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
  Valu *top = new Valu{contextp};
  
  nvboard_bind_pin(&top->a, 4, SW3, SW2, SW1, SW0);
  nvboard_bind_pin(&top->b, 4, SW7, SW6, SW5, SW4);
  nvboard_bind_pin(&top->mod, 3, SW15, SW14, SW13);
  nvboard_bind_pin(&top->out, 4, LD3, LD2, LD1, LD0);
  nvboard_bind_pin(&top->ZF, 1, LD14);
  nvboard_bind_pin(&top->CF, 1, LD15);
  
  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC();
  top->trace(m_trace, 99);
  m_trace->open("waveform.vcd");
  nvboard_init();

  /*
  000     加法        A+B
  001     减法        A-B
  010     取反       Not A
  011      与       A and B
  100      或       A or B
  101    异或       A xor B
  110    比较大小   If A<B then out=1; else out=0;
  111   判断相等    If A==B then out=1; else out=0;
  */
  top->mod = 0b001;

  top->a = 0b1010;
  top->b = 0b1010;

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