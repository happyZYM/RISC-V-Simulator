#pragma once
#ifndef ALU_H
#include "tools.h"
namespace ZYM {
struct ALU_Input {
  dark::Wire<7 + 3 + 1> request_full_id;
  dark::Wire<32> operand1;
  dark::Wire<32> operand2;
  dark::Wire<5> request_ROB_index;
};
struct ALU_Output {
  dark::Register<2> alu_status;
  dark::Register<5> result_ROB_index;
  dark::Register<32> result;
  dark::Register<32> completed_alu_resulting_PC;
};
struct ALU : public dark::Module<ALU_Input, ALU_Output> {
  ALU() {
    // Constructor
  }
  void work() {
    // Update function
  }
};
}
#endif