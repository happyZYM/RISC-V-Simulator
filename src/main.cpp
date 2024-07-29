#include "tools.h"
#include "csu.h"
int main(int argc, char **argv) {
  dark::CPU cpu;
  ZYM::Memory memory;
  cpu.add_module(&memory);
  memory.reset = [&]() { return cpu.GetResetSignal(); };
  memory.LoadProgram(std::cin);
  return 0;
}