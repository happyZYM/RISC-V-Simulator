#pragma once
#ifndef RESERVATIONSTATION_H
#include <array>
#include "tools.h"
namespace ZYM {
struct ReserveStation_Input {
  // receive control signal from CSU
  dark::Wire<1> reset;
  dark::Wire<1> force_clear_receiver;
  dark::Wire<1> is_issuing;
  dark::Wire<1> issue_type;
  dark::Wire<5> issue_ROB_index;
  dark::Wire<7 + 3 + 1> full_ins_id;
  dark::Wire<32> full_ins;
  dark::Wire<32> issuing_PC;
  dark::Wire<5> decoded_rd;
  dark::Wire<1> has_decoded_rd;
  dark::Wire<5> decoded_rs1;
  dark::Wire<1> has_decoded_rs1;
  dark::Wire<1> rs1_is_in_ROB;
  dark::Wire<32> rs1_in_ROB_value;
  dark::Wire<5> decoded_rs2;
  dark::Wire<1> has_decoded_rs2;
  dark::Wire<1> rs2_is_in_ROB;
  dark::Wire<32> rs2_in_ROB_value;
  dark::Wire<32> decoded_imm;
  dark::Wire<6> decoded_shamt;
  // receive data from register file
  dark::Wire<1> rs1_nodep;
  dark::Wire<5> rs1_deps;
  dark::Wire<32> rs1_value;
  dark::Wire<1> rs2_nodep;
  dark::Wire<5> rs2_deps;
  dark::Wire<32> rs2_value;
  // data from alu
  dark::Wire<2> alu_status_receiver;
  dark::Wire<5> completed_aluins_ROB_index;
  dark::Wire<32> completed_aluins_result;
  // data from Memory
  dark::Wire<2> mem_status_receiver;
  dark::Wire<5> completed_memins_ROB_index;
  dark::Wire<32> completed_memins_read_data;
  // receive status signal from L0 cache(data from Memory)
  dark::Wire<1> cache_hit;
  dark::Wire<5> cache_hit_ROB_index;
  dark::Wire<32> cache_hit_data;
};
struct ReserveStation_Output {
  // alu will listen for these:
  dark::Register<7 + 3 + 1> request_full_id;
  dark::Register<32> operand1;
  dark::Register<32> operand2;
  dark::Register<32> op_imm;
  dark::Register<6> op_shamt;
  dark::Register<32> alu_ins_PC;
  dark::Register<5> request_ROB_index;
  dark::Register<6> RS_remain_space_output;
};
struct RS_Record {
  dark::Register<2> state;  // 0: no, 1: initializing dependency, 2: waiting for data
  dark::Register<7 + 3 + 1> full_ins_id;
  dark::Register<32> Vj, Vk;
  dark::Register<5> Qj, Qk;
  dark::Register<5> ins_ROB_index;
  dark::Register<32> ins_self_PC;
  dark::Register<32> ins_imm;
  dark::Register<32> addr;
};
struct ReserveStation_Private {
  dark::Register<6> RS_remaining_space;
  std::array<RS_Record, 32> RS_records;
  dark::Register<1> has_accepted_ins_last_cycle;
};
struct ReserveStation : public dark::Module<ReserveStation_Input, ReserveStation_Output, ReserveStation_Private> {
  ReserveStation() {
    // Constructor
  }
  void work() {
    // Update function
    if (bool(reset)) {
      for (auto &record : RS_records) {
        record.state <= 0;
      }
      RS_remaining_space <= 32;
      request_full_id <= 0;
      return;
    }
    if (bool(force_clear_receiver)) {
      for (auto &record : RS_records) {
        record.state <= 0;
      }
      RS_remaining_space <= 32;
      request_full_id <= 0;
      return;
    }
    if (bool(is_issuing) && issue_type == 0) {
#ifdef _DEBUG
      if (RS_remaining_space == 0 || RS_remaining_space > 32) {
        std::cerr << "Reserve Station is full, cannot issue new instruction" << std::endl;
        return;
      }
#endif
      has_accepted_ins_last_cycle <= 1;
      // TODO: to something to accept the instruction
    } else
      has_accepted_ins_last_cycle <= 0;
    if (bool(has_accepted_ins_last_cycle)) {
      // TODO: now dependency info can be read from the register file, in the mean time, CSU will provide the
      // potentially missing data
    }
    // TODO: now alu, memory (and L0 cache of memory) may provide data to satisfy the dependency
    // TODO: now, we can check if we can execute the instruction, memory and L0 cache will listen to this
  }
};
}  // namespace ZYM
#endif