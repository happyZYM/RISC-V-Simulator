#pragma once
#ifndef RESERVATIONSTATION_H
#include "tools.h"
namespace ZYM {
struct ReserveStation_Input {
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
  dark::Register<7+3+1> request_full_id;
  dark::Register<32> operand1;
  dark::Register<32> operand2;
  dark::Register<5> request_ROB_index;
  dark::Register<6> RS_remain_space_output;
};
struct ReserveStation_Private {
  dark::Register<5> RS_head;
  dark::Register<5> RS_tail;
};
struct ReserveStation : public dark::Module<ReserveStation_Input, ReserveStation_Output, ReserveStation_Private> {
    ReserveStation() {
        // Constructor
    }
    void work() {
        // Update function
    }
};
}
#endif