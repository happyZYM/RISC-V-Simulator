#pragma once
#include <iomanip>
#include "concept.h"
#ifndef REGISTERFILE_H
#include <array>
#include "tools.h"
namespace ZYM {
const static size_t kTotalRegisters = 32;
struct RegisterFile_Input {
  // receive control signal from CSU
  dark::Wire<1> reset;
  dark::Wire<1> force_clear_receiver;
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
  uint8_t ReturnExitCodeImmediately() {
    std::cerr << "Register File: CSU is collecting exit code" << std::endl;
    std::cerr << "Sent " << std::dec << (registers[10].peek() & 0xff) << std::endl;
    return registers[10].peek() & 0xff;
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
    bool dependency_cleared = false;
    if (bool(is_committing)) {
      std::cerr << "register file found CSU is committing commit_ins_ROB_index=" << std::dec
                << static_cast<max_size_t>(commit_ins_ROB_index) << std::endl;
      if (bool(has_resulting_register)) {
        registers[static_cast<max_size_t>(commit_reg_index)] <= commit_reg_value;
        if (register_deps[static_cast<max_size_t>(commit_reg_index)] == commit_ins_ROB_index) {
          std::cerr << "The dependency is cleared" << std::endl;
          if (!(bool(is_issuing) && bool(has_decoded_rd) &&
                (static_cast<max_size_t>(decoded_rd) == static_cast<max_size_t>(commit_reg_index))))
            register_nodep[static_cast<max_size_t>(commit_reg_index)] <= 1;
          dependency_cleared = true;
        }
      }
    }
    if (bool(force_clear_receiver)) {
      for (auto &reg : register_deps) {
        reg <= 0;
      }
      for (auto &reg : register_nodep) {
        reg <= 1;
      }
      return;
    }
    if (bool(is_issuing)) {
      std::cerr << "Register File Found CSU is issuing" << std::endl;
      if (bool(has_decoded_rs1)) {
        if (static_cast<max_size_t>(decoded_rs1) == 0) {
          rs1_deps <= 0;
          rs1_value <= 0;
          rs1_nodep <= 1;
        } else if ((!bool(is_committing)) || (commit_reg_index != decoded_rs1) || (!dependency_cleared)) {
          rs1_deps <= register_deps[static_cast<max_size_t>(decoded_rs1)].peek();
          rs1_value <= registers[static_cast<max_size_t>(decoded_rs1)].peek();
          rs1_nodep <= register_nodep[static_cast<max_size_t>(decoded_rs1)].peek();
        } else {
          rs1_deps <= 0;
          rs1_value <= commit_reg_value;
          rs1_nodep <= 1;
        }
        std::cerr << std::dec << "rs1_deps=" << rs1_deps.peek() << std::endl;
        std::cerr << std::hex << std::setw(8) << std::setfill('0') << "rs1_value=" << rs1_value.peek() << std::endl;
        std::cerr << "rs1_nodep=" << rs1_nodep.peek() << std::endl;
      }
      if (bool(has_decoded_rs2)) {
        if (static_cast<max_size_t>(decoded_rs2) == 0) {
          rs2_deps <= 0;
          rs2_value <= 0;
          rs2_nodep <= 1;
        } else if ((!bool(is_committing)) || (commit_reg_index != decoded_rs2) || (!dependency_cleared)) {
          rs2_deps <= register_deps[static_cast<max_size_t>(decoded_rs2)].peek();
          rs2_value <= registers[static_cast<max_size_t>(decoded_rs2)].peek();
          rs2_nodep <= register_nodep[static_cast<max_size_t>(decoded_rs2)].peek();
        } else {
          rs2_deps <= 0;
          rs2_value <= commit_reg_value;
          rs2_nodep <= 1;
        }
        std::cerr << std::dec << "rs2_deps=" << rs2_deps.peek() << std::endl;
        std::cerr << std::hex << std::setw(8) << std::setfill('0') << "rs2_value=" << rs2_value.peek() << std::endl;
        std::cerr << "rs2_nodep=" << rs2_nodep.peek() << std::endl;
      }
      if (bool(has_decoded_rd)) {
        std::cerr << "RF: setting rd dependency" << std::endl;
        std::cerr << "\tdecoded_rd=" << std::dec << static_cast<max_size_t>(decoded_rd) << std::endl;
        std::cerr << "\tissue_ROB_index=" << std::dec << static_cast<max_size_t>(issue_ROB_index) << std::endl;
        register_deps[static_cast<max_size_t>(decoded_rd)] <= static_cast<max_size_t>(issue_ROB_index);
        register_nodep[static_cast<max_size_t>(decoded_rd)] <= 0;
      }
    }
  }
};
}  // namespace ZYM
#endif