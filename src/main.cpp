#include <iostream>
#include "alu.h"
#include "csu.h"
#include "loadstorequeue.h"
#include "memory.h"
#include "registerfile.h"
#include "reservestation.h"
#include "tools.h"
int main(int argc, char **argv) {
  dark::CPU cpu;
  ZYM::CentralScheduleUnit csu;
  ZYM::Memory memory;
  cpu.add_module(&csu);
  cpu.add_module(&memory);
  cpu.halt_signal.assign([&]() -> auto & { return csu.halt_signal; });
  memory.reset = [&]() { return cpu.GetResetSignal(); };
  memory.LoadProgram(std::cin);
  std::cout << cpu.run(0, true) << std::endl;
  return 0;
}