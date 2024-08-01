#pragma once
#include <iterator>
#include "concept.h"
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
  // dark::Wire<1> cache_hit;
  // dark::Wire<5> cache_hit_ROB_index;
  // dark::Wire<32> cache_hit_data;
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
  dark::Register<32> V1, V2;
  dark::Register<5> Q1, Q2;
  dark::Register<1> E1, E2;
  dark::Register<1> D1, D2;  // 1: no dependency, 0: dependency
  dark::Register<5> ins_ROB_index;
  dark::Register<32> ins_self_PC;
  dark::Register<32> ins_imm;
  dark::Register<6> ins_shamt;
};
struct ReserveStation_Private {
  dark::Register<6> RS_remaining_space;
  std::array<RS_Record, 32> RS_records;
  dark::Register<1> has_accepted_ins_last_cycle;
  dark::Register<5> last_cycle_ins_RS_index;
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
      RS_remain_space_output <= 32;
      request_full_id <= 0;
      has_accepted_ins_last_cycle <= 0;
      return;
    }
    if (bool(force_clear_receiver)) {
      for (auto &record : RS_records) {
        record.state <= 0;
      }
      RS_remaining_space <= 32;
      RS_remain_space_output <= 32;
      request_full_id <= 0;
      has_accepted_ins_last_cycle <= 0;
      return;
    }
    uint32_t next_remain_space = static_cast<max_size_t>(RS_remaining_space);
    if (bool(is_issuing) && issue_type == 0) {
#ifdef _DEBUG
      if (next_remain_space == 0 || next_remain_space > 32)
        throw std::runtime_error("ReserveStation: next_remain_space is out of range");
#endif
      has_accepted_ins_last_cycle <= 1;
      // do something to accept the instruction
      next_remain_space--;
      uint32_t deposit_index = -1;
      for (uint32_t i = 0; i < 32; i++) {
        if (static_cast<max_size_t>(RS_records[i].state) == 0) {
          deposit_index = i;
          break;
        }
      }
      dark::debug::assert(deposit_index != -1, "ReserveStation: deposit_index is -1");
      last_cycle_ins_RS_index <= deposit_index;
      RS_records[deposit_index].state <= 1;
      RS_records[deposit_index].full_ins_id <= full_ins_id;
      RS_records[deposit_index].ins_ROB_index <= issue_ROB_index;
      RS_records[deposit_index].ins_self_PC <= issuing_PC;
      RS_records[deposit_index].ins_imm <= decoded_imm;
      RS_records[deposit_index].ins_shamt <= decoded_shamt;
      RS_records[deposit_index].E1 <= has_decoded_rs1;
      RS_records[deposit_index].E2 <= has_decoded_rs2;
      RS_records[deposit_index].D1 <= 1;
      RS_records[deposit_index].D2 <= 1;
      DEBUG_CERR << "Reserve Station has accepted an instruction from CSU" << std::endl;
      DEBUG_CERR << "\tdeposit_index=" << std::dec << deposit_index << std::endl;
      DEBUG_CERR << "\tROB_index=" << std::dec << static_cast<max_size_t>(issue_ROB_index) << std::endl;
    } else
      has_accepted_ins_last_cycle <= 0;
    uint32_t last_idx = static_cast<max_size_t>(last_cycle_ins_RS_index);
    bool last_cycle_V1_proccessed = false;
    bool last_cycle_V2_proccessed = false;
    if (bool(has_accepted_ins_last_cycle)) {
      // TODO: now dependency info can be read from the register file, in the mean time, CSU will provide the
      // potentially missing data
      DEBUG_CERR << "Reserve Station is listening dependency info from Register File and CSU" << std::endl;
      if (bool(RS_records[last_idx].E1) && bool(rs1_nodep)) {
        RS_records[last_idx].V1 <= rs1_value;
        RS_records[last_idx].D1 <= 1;
        last_cycle_V1_proccessed = true;
        DEBUG_CERR << "\t Register File: RS1 is not dependent" << std::endl;
      }
      if (bool(RS_records[last_idx].E2) && bool(rs2_nodep)) {
        RS_records[last_idx].V2 <= rs2_value;
        RS_records[last_idx].D2 <= 1;
        last_cycle_V2_proccessed = true;
        DEBUG_CERR << "\t Register File: RS2 is not dependent" << std::endl;
      }
      if (bool(RS_records[last_idx].E1) && (!bool(rs1_nodep)) && bool(rs1_is_in_ROB)) {
        RS_records[last_idx].V1 <= rs1_in_ROB_value;
        RS_records[last_idx].D1 <= 1;
        last_cycle_V1_proccessed = true;
        DEBUG_CERR << "\t ROB: RS1 is in ROB" << std::endl;
      }
      if (bool(RS_records[last_idx].E2) && (!bool(rs2_nodep)) && bool(rs2_is_in_ROB)) {
        RS_records[last_idx].V2 <= rs2_in_ROB_value;
        RS_records[last_idx].D2 <= 1;
        last_cycle_V2_proccessed = true;
        DEBUG_CERR << "\t ROB: RS2 is in ROB" << std::endl;
      }
    }
    // TODO: now alu, memory may provide data to satisfy the dependency
    bool should_monitor_V1 =
        bool(has_accepted_ins_last_cycle) && bool(RS_records[last_idx].E1) && (!last_cycle_V1_proccessed);
    bool should_monitor_V2 =
        bool(has_accepted_ins_last_cycle) && bool(RS_records[last_idx].E2) && (!last_cycle_V2_proccessed);
    auto process_listend_data = [&](uint32_t res_ROB_index, uint32_t res_value) -> void {
      DEBUG_CERR << "\tres_ROB_index=" << std::dec << res_ROB_index << std::endl;
      for (uint32_t ptr = 0; ptr < 32; ptr++) {
        if (RS_records[ptr].state == 0) continue;
        if ((!bool(has_accepted_ins_last_cycle)) || ptr != last_idx) {
          dark::debug::assert(RS_records[ptr].state == 2, "RS_records[ptr].state != 2");
          if ((!bool(RS_records[ptr].D1)) && static_cast<max_size_t>(RS_records[ptr].Q1) == res_ROB_index) {
            RS_records[ptr].V1 <= res_value;
            RS_records[ptr].D1 <= 1;
          }
          if ((!bool(RS_records[ptr].D2)) && static_cast<max_size_t>(RS_records[ptr].Q2) == res_ROB_index) {
            RS_records[ptr].V2 <= res_value;
            RS_records[ptr].D2 <= 1;
          }
        } else {
          if (should_monitor_V1 && static_cast<max_size_t>(rs1_deps) == res_ROB_index) {
            RS_records[last_idx].V1 <= res_value;
            RS_records[last_idx].D1 <= 1;
            should_monitor_V1 = false;
          }
          if (should_monitor_V2 && static_cast<max_size_t>(rs2_deps) == res_ROB_index) {
            RS_records[last_idx].V2 <= res_value;
            RS_records[last_idx].D2 <= 1;
            should_monitor_V2 = false;
          }
        }
      }
    };
    DEBUG_CERR << "Reservestation is listening data from ALU" << std::endl;
    if (static_cast<max_size_t>(alu_status_receiver) == 0b10) {
      process_listend_data(static_cast<max_size_t>(completed_aluins_ROB_index),
                           static_cast<max_size_t>(completed_aluins_result));
    }
    DEBUG_CERR << "Reservestation is listening data from Memory" << std::endl;
    if (static_cast<max_size_t>(mem_status_receiver) == 0b10) {
      process_listend_data(static_cast<max_size_t>(completed_memins_ROB_index),
                           static_cast<max_size_t>(completed_memins_read_data));
    }
    // if (static_cast<max_size_t>(cache_hit) == 1) {
    // process_listend_data(static_cast<max_size_t>(cache_hit_ROB_index), static_cast<max_size_t>(cache_hit_data));
    // }
    if (should_monitor_V1) {
      RS_records[last_idx].Q1 <= rs1_deps;
      RS_records[last_idx].D1 <= 0;
      DEBUG_CERR << "\t RS1 depend on ins of ROB index " << std::dec << RS_records[last_idx].Q1.peek() << std::endl;
    }
    if (should_monitor_V2) {
      RS_records[last_idx].Q2 <= rs2_deps;
      RS_records[last_idx].D2 <= 0;
      DEBUG_CERR << "\t RS2 depend on ins of ROB index " << std::dec << RS_records[last_idx].Q2.peek() << std::endl;
    }
    // TODO: now, we can check if we can execute the instruction, memory and L0 cache will listen to this
    if (bool(has_accepted_ins_last_cycle)) RS_records[last_idx].state <= 2;
    bool can_execute = false;
    for (int i = 0; i < 32; i++) {
      if (RS_records[i].state != 2) continue;
      if (RS_records[i].E1 == 1 && RS_records[i].D1 == 0) continue;
      if (RS_records[i].E2 == 1 && RS_records[i].D2 == 0) continue;
      can_execute = true;
      request_full_id <= RS_records[i].full_ins_id;
      operand1 <= RS_records[i].V1;
      operand2 <= RS_records[i].V2;
      op_imm <= RS_records[i].ins_imm;
      op_shamt <= RS_records[i].ins_shamt;
      alu_ins_PC <= RS_records[i].ins_self_PC;
      request_ROB_index <= RS_records[i].ins_ROB_index;
      RS_records[i].state <= 0;
      next_remain_space++;
      break;
    }
    if (!can_execute) request_full_id <= 0;
    RS_remaining_space <= next_remain_space;
    RS_remain_space_output <= next_remain_space;
    DEBUG_CERR << "Reservestation: next_remain_space=" << std::dec << next_remain_space << std::endl;
    int tot = 0;
    for (int i = 0; i < 32; i++)
      if (static_cast<max_size_t>(RS_records[i].state) == 0) tot++;
    DEBUG_CERR << "\tcurrently there are " << std::dec << tot
              << " remain spaces based on state but RS_remaining_space says " << std::dec
              << static_cast<max_size_t>(RS_remaining_space) << std::endl;
    if (tot != static_cast<max_size_t>(RS_remaining_space)) {
      throw std::runtime_error("Reservestation: RS_remaining_space is not consistent with RS_records");
    }
  }
};
}  // namespace ZYM
#endif