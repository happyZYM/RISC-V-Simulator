#pragma once
#include "concept.h"
#ifndef REGISTERFILE_H
#include <array>
#include "tools.h"
namespace ZYM {
const static size_t kTotalRegisters = 32;
struct RegisterFile_Input {
  // receive control signal from CSU
  dark::Wire<1> reset;
  // dark::Wire<1> force_clear_receiver;
  dark::Wire<1> is_issuing;
  dark::Wire<1> issue_type;
  dark::Wire<5> issue_ROB_index;
  dark::Wire<7 + 3 + 1> full_ins_id;
  dark::Wire<32> full_ins;
  dark::Wire<5> decoded_rd;
  dark::Wire<1> has_decoded_rd;
  dark::Wire<5> decoded_rs1;
  dark::Wire<1> has_decoded_rs1;
  dark::Wire<5> decoded_rs2;
  dark::Wire<1> has_decoded_rs2;
  dark::Wire<1> is_committing;
  dark::Wire<1> has_resulting_register;
  dark::Wire<5> commit_ins_ROB_index;
  dark::Wire<5> commit_reg_index;
  dark::Wire<32> commit_reg_value;
};
struct RegisterFile_Output {
  dark::Register<1> rs1_nodep;
  dark::Register<5> rs1_deps;
  dark::Register<32> rs1_value;
  dark::Register<1> rs2_nodep;
  dark::Register<5> rs2_deps;
  dark::Register<32> rs2_value;
};
struct RegisterFile_Private {
  std::array<dark::Register<32>, kTotalRegisters> registers;
  std::array<dark::Register<5>, kTotalRegisters> register_deps;
  std::array<dark::Register<1>, kTotalRegisters> register_nodep;
};
struct RegisterFile : public dark::Module<RegisterFile_Input, RegisterFile_Output, RegisterFile_Private> {
  RegisterFile() {
    // Constructor
  }
  void work() {
    if (bool(reset)) {
      for (auto &reg : registers) {
        reg <= 0;
      }
      for (auto &reg : register_deps) {
        reg <= 0;
      }
      for (auto &reg : register_nodep) {
        reg <= 1;
      }
      return;
    }
    if (bool(is_committing)) {
      if (bool(has_resulting_register)) {
        registers[static_cast<max_size_t>(commit_reg_index)] <= commit_reg_value;
        if (register_deps[static_cast<max_size_t>(commit_reg_index)] == commit_ins_ROB_index) {
          register_nodep[static_cast<max_size_t>(commit_reg_index)] <= 1;
        }
      }
    }
    if (bool(is_issuing)) {
      if (bool(has_decoded_rs1)) {
        if ((!bool(is_committing)) || (commit_reg_index != decoded_rs1)) {
          rs1_deps <= register_deps[static_cast<max_size_t>(decoded_rs1)].peek();
          rs1_value <= registers[static_cast<max_size_t>(decoded_rs1)].peek();
          rs1_nodep <= register_nodep[static_cast<max_size_t>(decoded_rs1)].peek();
        } else {
          rs1_deps <= 0;
          rs1_value <= commit_reg_value;
          rs1_nodep <= 1;
        }
      }
      if (bool(has_decoded_rs2)) {
        if ((!bool(is_committing)) || (commit_reg_index != decoded_rs2)) {
          rs2_deps <= register_deps[static_cast<max_size_t>(decoded_rs2)].peek();
          rs2_value <= registers[static_cast<max_size_t>(decoded_rs2)].peek();
          rs2_nodep <= register_nodep[static_cast<max_size_t>(decoded_rs2)].peek();
        } else {
          rs2_deps <= 0;
          rs2_value <= commit_reg_value;
          rs2_nodep <= 1;
        }
      }
      if (bool(has_decoded_rd)) {
        register_deps[static_cast<max_size_t>(decoded_rd)] <= static_cast<max_size_t>(issue_ROB_index);
        register_nodep[static_cast<max_size_t>(decoded_rd)] <= 0;
      }
    }
  }
};
}  // namespace ZYM
#endif