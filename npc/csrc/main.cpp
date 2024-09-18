#include "Vselect.h"
#include "nvboard.h"
#include "verilated_vcd_c.h"
#include <assert.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  const std::uint64_t sim_time = 400000000;
  
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Vselect *top = new Vselect{contextp};
  
  nvboard_bind_pin(&top->yi, 2, SW1, SW0);
  nvboard_bind_pin(&top->Xi, 8, SW9, SW8, SW7, SW6, SW5, SW4, SW3, SW2, SW1);
  nvboard_bind_pin(&top->Fo, 2, LD1, LD0);
  
  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC();
  top->trace(m_trace, 99);       // 顶层类设置测试波形参数
  m_trace->open("waveform.vcd"); // 设置波形写入的文件
  nvboard_init();

  int i = 0;
  top->Xi = 0b10110001;

  for (;contextp->time() < sim_time && !contextp->gotFinish();) {
    contextp->timeInc(1);
    top->yi = i;
    top->eval();
    m_trace->dump(contextp->time());
    nvboard_update();
    if (++i > 3) {
      i = 0;
    }
  }

  m_trace->close();
  nvboard_quit();
}