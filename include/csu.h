#pragma once
#ifndef CSU_H
#include <array>
#include <functional>
#include "tools.h"
namespace ZYM {
const int kROBSize = 32;
const int kTotalRegister = 32;
struct CentralScheduleUnit_Input {
  dark::Wire<1> reset;
  // data from load store queue
  dark::Wire<6> load_store_queue_emptyspace_receiver;
  // data from reservation station
  dark::Wire<6> reservestation_emptyspace_receiver;
  // data from Memory
  dark::Wire<2> mem_status_receiver;
  dark::Wire<5> completed_memins_ROB_index;
  dark::Wire<32> completed_memins_read_data;
  // data from LoadStoreQueue
  dark::Wire<4> mem_request_type_input;
  dark::Wire<32> mem_address_input;
  dark::Wire<32> mem_data_input;
  dark::Wire<5> mem_request_ROB_index;
  // data from alu
  dark::Wire<2> alu_status_receiver;
  dark::Wire<5> completed_aluins_ROB_index;
  dark::Wire<32> completed_aluins_result;
  dark::Wire<32> completed_alu_resulting_PC;
  // receive data from register file
  dark::Wire<1> rs1_nodep;
  dark::Wire<5> rs1_deps;
  dark::Wire<1> rs2_nodep;
  dark::Wire<5> rs2_deps;
};
struct CentralScheduleUnit_Output {
  dark::Register<1> force_clear_announcer;
  dark::Register<9> halt_signal;  // The highest bit is the marker, and the rest is the exit code
  dark::Register<1> is_issuing;
  dark::Register<1> issue_type;  // 0: alu, 1: mem
  dark::Register<5> issue_ROB_index;
  dark::Register<7 + 3 + 1> full_ins_id;
  dark::Register<32> full_ins;
  dark::Register<5> decoded_rd;
  dark::Register<1> has_decoded_rd;
  dark::Register<5> decoded_rs1;
  dark::Register<1> has_decoded_rs1;
  dark::Register<1> rs1_is_in_ROB;
  dark::Register<32> rs1_in_ROB_value;
  dark::Register<5> decoded_rs2;
  dark::Register<1> has_decoded_rs2;
  dark::Register<1> rs2_is_in_ROB;
  dark::Register<32> rs2_in_ROB_value;
  dark::Register<32> decoded_imm;
  dark::Register<6> decoded_shamt;
  dark::Register<1> cache_hit;
  dark::Register<5> cache_hit_ROB_index;
  dark::Register<32> cache_hit_data;
};
struct ROBRecordType {
  dark::Register<4> state;
  dark::Register<32> instruction;
  dark::Register<5> resulting_register_idx;
  dark::Register<32> resulting_register_value;
  dark::Register<1> resulting_PC_ready;
  dark::Register<32> resulting_PC;
  dark::Register<4> mem_request_type;  // see memory.h
  dark::Register<32> mem_request_addr;
  dark::Register<32> mem_request_data;
};
struct CentralScheduleUnit_Private {
  dark::Register<32> predicted_PC;
  dark::Register<1> has_predicted_PC;  // jalr may force the CPU to stall
  dark::Register<32> actual_PC;
  std::array<ROBRecordType, kROBSize> ROB_records;
  dark::Register<5> ROB_head;
  dark::Register<5> ROB_tail;
  dark::Register<6> ROB_remain_space;
  dark::Register<1> has_instruction_issued_last_cycle;  // to correctly calculate remain space for next cycle in issue a
                                                        // ins this cycle
};
struct CentralScheduleUnit
    : public dark::Module<CentralScheduleUnit_Input, CentralScheduleUnit_Output, CentralScheduleUnit_Private> {
 private:
  std::function<max_size_t(max_size_t)> instruction_fetcher;
  bool instruction_fetcher_initialized = false;

 public:
  CentralScheduleUnit() { ; }
  void SetInstructionFetcher(std::function<max_size_t(max_size_t)> fetcher) {
    if (instruction_fetcher_initialized) throw std::runtime_error("Instruction fetcher has been initialized");
    instruction_fetcher = fetcher;
    instruction_fetcher_initialized = true;
  }
  void work() override final {
    if (bool(reset)) {
      predicted_PC <= 0;
      actual_PC <= 0;
      has_predicted_PC <= 1;
      ROB_head <= 0;
      ROB_tail <= 0;
      ROB_remain_space <= kROBSize;
    }
  }
};
}  // namespace ZYM
#endif