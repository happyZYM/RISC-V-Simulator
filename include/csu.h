#pragma once
#ifndef CSU_H
#include <functional>
#include <array>
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
  // data from Memory Station
  dark::Wire<2> completed_memins_type;
  dark::Wire<5> completed_memins_ROB_index;
  dark::Wire<32> completed_memins_read_data;
  // data from alu
  dark::Wire<5> completed_aluins_ROB_index;
  dark::Wire<32> completed_aluins_result;
};
struct CentralScheduleUnit_Output {
  dark::Register<1> ready;
  dark::Register<1> force_clear_announcer;
  dark::Register<9> halt_signal;  // The highest bit is the marker, and the rest is the exit code
};
struct ROBRecordType {
  dark::Register<4> state;
  dark::Register<32> instruction;
  dark::Register<5> resulting_register_idx;
  dark::Register<32> resulting_register_value;
  dark::Register<1> resulting_PC_ready;
  dark::Register<32> resulting_PC;
  dark::Register<4> mem_request_type; // see memory.h
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
  dark::Register<6> ROB_size;
};
struct CentralScheduleUnit
    : public dark::Module<CentralScheduleUnit_Input, CentralScheduleUnit_Output, CentralScheduleUnit_Private> {
 private:
 public:
  CentralScheduleUnit() { ; }
  void work() override final {
    if (bool(reset)) {
      predicted_PC <= 0;
      actual_PC <= 0;
      has_predicted_PC <= 1;
    }
  }
};
}  // namespace ZYM
#endif