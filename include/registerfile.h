#pragma once
#ifndef REGISTERFILE_H
#include "tools.h"
namespace ZYM {
struct RegisterFile_Input {
  // receive control signal from CSU
  dark::Wire<1> reset;
  dark::Wire<1> force_clear_receiver;
  dark::Wire<1> is_issuing;
  dark::Wire<1> issue_type;
  dark::Wire<5> issue_ROB_index;
  dark::Wire<7+3+1> full_ins_id;
  dark::Wire<32> full_ins;
  dark::Wire<5> decoded_rd;
  dark::Wire<1> has_decoded_rd;
  dark::Wire<5> decoded_rs1;
  dark::Wire<1> has_decoded_rs1;
  dark::Wire<5> decoded_rs2;
  dark::Wire<1> has_decoded_rs2;
};
struct RegisterFile_Output {
  dark::Register<1> rs1_nodep;
  dark::Register<5> rs1_deps;
  dark::Register<32> rs1_value;
  dark::Register<1> rs2_nodep;
  dark::Register<5> rs2_deps;
  dark::Register<32> rs2_value;
};
struct RegisterFile : public dark::Module<RegisterFile_Input, RegisterFile_Output> {
    RegisterFile() {
        // Constructor
    }
    void work() {
        // Update function
    }
};
}
#endif