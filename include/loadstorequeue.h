#pragma once
#ifndef LOADSTOREQUEUE_H
#include <array>
#include "tools.h"
namespace ZYM {
struct LoadStoreQueue_Input {
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
  // receive status signal from Memory
  dark::Wire<2> mem_data_sign;
  dark::Wire<5> completed_memins_ROB_index;
  dark::Wire<32> completed_memins_read_data;
  // receive status signal from L0 cache
  dark::Wire<1> cache_hit;
  dark::Wire<5> cache_hit_ROB_index;
  dark::Wire<32> cache_hit_data;
};
struct LoadStoreQueue_Output {
  // request signal, Memory and the L0 cache in ROB will listen to this
  dark::Register<4> request_type_output;
  dark::Register<5> request_ROB_index;
  dark::Register<32> request_address_output;
  dark::Register<32> request_data_output;
  dark::Register<6> LSQ_remain_space_output;
};
struct LSQ_Record {
  dark::Register<2> state;  // 0: no, 1: initializing dependency, 2: waiting for data
  dark::Register<7 + 3 + 1> full_ins_id;
  dark::Register<32> Vj, Vk;
  dark::Register<5> Qj, Qk;
  dark::Register<5> ins_ROB_index;
  dark::Register<32> ins_self_PC;
  dark::Register<32> ins_imm;
  dark::Register<32> addr;
};
struct LoadStoreQueue_Private {
  dark::Register<5> LSQ_head;
  dark::Register<5> LSQ_tail;
  dark::Register<6> LSQ_remain_space;
  std::array<LSQ_Record, 32> LSQ_queue;
  dark::Register<1> has_accepted_ins_last_cycle;
  dark::Register<5> last_cycle_ins_LSQ_index;
};
struct LoadStoreQueue : public dark::Module<LoadStoreQueue_Input, LoadStoreQueue_Output, LoadStoreQueue_Private> {
  LoadStoreQueue() {
    // Constructor
  }
  void work() {
    if (bool(reset)) {
      LSQ_remain_space <= 32;
      LSQ_head <= 0;
      LSQ_tail <= 0;
      for (auto &record : LSQ_queue) {
        record.state <= 0;
      }
      has_accepted_ins_last_cycle <= 0;
      return;
    }
    if (bool(force_clear_receiver)) {
      LSQ_remain_space <= 32;
      LSQ_head <= 0;
      LSQ_tail <= 0;
      for (auto &record : LSQ_queue) {
        record.state <= 0;
      }
      has_accepted_ins_last_cycle <= 0;
      return;
    }
    if (bool(is_issuing) && issue_type == 1) {
#ifdef _DEBUG
      if (LSQ_remain_space == 0 || LSQ_remain_space > 32) throw std::runtime_error("LSQ_remain_space is out of range");
#endif
      has_accepted_ins_last_cycle <= 1;
      // TODO: now we can accept the instruction, that is, to store it in the LSQ
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