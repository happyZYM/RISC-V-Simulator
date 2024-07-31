#pragma once
#include <sys/types.h>
#include "concept.h"
#ifndef CSU_H
#include <array>
#include <functional>
#include <tuple>
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
  // dark::Wire<1> rs1_nodep;
  // dark::Wire<5> rs1_deps;
  // dark::Wire<1> rs2_nodep;
  // dark::Wire<5> rs2_deps;
};
struct CentralScheduleUnit_Output {
  dark::Register<1> force_clear_announcer;
  dark::Register<9> halt_signal;  // The highest bit is the marker, and the rest is the exit code
  dark::Register<1> is_issuing;
  dark::Register<1> issue_type;  // 0: alu, 1: mem
  dark::Register<5> issue_ROB_index;
  dark::Register<7 + 3 + 1> full_ins_id;
  dark::Register<32> full_ins;
  dark::Register<32> issuing_PC;
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
  dark::Register<1> is_committing;
  dark::Register<5> commit_reg_index;
  dark::Register<32> commit_reg_value;
  dark::Register<5> commit_ins_ROB_index;
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
  inline uint8_t ReadBit(uint32_t data, int pos) { return (data >> pos) & 1; }
  inline void WriteBit(uint32_t &data, int pos, uint8_t bit) {
    data &= ~(1 << pos);
    data |= bit << pos;
  }
  std::tuple<dark::Bit<7 + 3 + 1>, dark::Bit<5>, dark::Bit<1>, dark::Bit<5>, dark::Bit<1>, dark::Bit<5>, dark::Bit<1>,
             dark::Bit<32>, dark::Bit<6>>
  Decode(max_size_t ins) {
    // Decode the instruction
    dark::Bit<7 + 3 + 1> full_ins_id;
    dark::Bit<5> decoded_rd;
    dark::Bit<1> has_decoded_rd;
    dark::Bit<5> decoded_rs1;
    dark::Bit<1> has_decoded_rs1;
    dark::Bit<5> decoded_rs2;
    dark::Bit<1> has_decoded_rs2;
    dark::Bit<32> decoded_imm;
    dark::Bit<6> decoded_shamt;
    uint8_t opcode = ins & 0x7F;
    if (opcode == 0b0110011) {
      // R-type
      has_decoded_rd = 1;
      has_decoded_rs1 = 1;
      has_decoded_rs2 = 1;
      decoded_rd = ins >> 7 & 0x1F;
      decoded_rs1 = ins >> 15 & 0x1F;
      decoded_rs2 = ins >> 20 & 0x1F;
      uint8_t funct3 = ins >> 12 & 0x7;
      uint8_t funct7 = ins >> 25 & 0x7F;
      full_ins_id = opcode | (funct3 << 7) | (((funct7 >> 5) & 1) << 10);
    } else if (opcode == 0b1100111 || opcode == 0b0000011 || opcode == 0b0010011) {
      // I-type
      has_decoded_rd = 1;
      has_decoded_rs1 = 1;
      has_decoded_rs2 = 0;
      decoded_rd = ins >> 7 & 0x1F;
      decoded_rs1 = ins >> 15 & 0x1F;
      decoded_imm = ins >> 20;
      uint32_t sign_bit = ins >> 31;
      if (sign_bit) {
        decoded_imm = static_cast<uint32_t>(decoded_imm) | 0xFFFFF000;
      }
      uint8_t funct3 = ins >> 12 & 0x7;
      uint8_t funct7 = ins >> 25 & 0x7F;
      full_ins_id = opcode | (funct3 << 7) | (((funct7 >> 5) & 1) << 10);
      decoded_shamt = ins >> 20 & 0x3F;
    } else if (opcode == 0b0100011) {
      // S-type
      has_decoded_rd = 0;
      has_decoded_rs1 = 1;
      has_decoded_rs2 = 1;
      decoded_rs1 = (ins >> 15) & 0x1F;
      decoded_rs2 = (ins >> 20) & 0x1F;
      decoded_imm = ((ins >> 7) & 0x1F) | ((ins >> 25) << 5);
      uint32_t sign_bit = ins >> 31;
      if (sign_bit) {
        decoded_imm = static_cast<uint32_t>(decoded_imm) | 0xFFFFF000;
      }
      uint8_t funct3 = ins >> 12 & 0x7;
      full_ins_id = opcode | (funct3 << 7);
    } else if (opcode == 0b1100011) {
      // B-type
      has_decoded_rd = 0;
      has_decoded_rs1 = 1;
      has_decoded_rs2 = 1;
      decoded_rs1 = (ins >> 15) & 0x1F;
      decoded_rs2 = (ins >> 20) & 0x1F;
      decoded_imm = (((ins >> 8) & 0xF) << 1) | (((ins >> 25) & 0x3F) << 5) | (((ins >> 7) & 0x1) << 11) |
                    (((ins >> 31) & 0x1) << 12);
      uint32_t sign_bit = ins >> 31;
      if (sign_bit) {
        decoded_imm = static_cast<uint32_t>(decoded_imm) | 0xFFFFE000;
      }
      uint8_t funct3 = ins >> 12 & 0x7;
      full_ins_id = opcode | (funct3 << 7);
    } else if(opcode==0b0110111) {
      // U-type
      has_decoded_rd = 1;
      has_decoded_rs1 = 0;
      has_decoded_rs2 = 0;
      decoded_rd = ins >> 7 & 0x1F;
      decoded_imm = ins >> 12;
      full_ins_id = opcode;
    } else if(opcode==0b1101111) {
      // J-type
      has_decoded_rd = 1;
      has_decoded_rs1 = 0;
      has_decoded_rs2 = 0;
      decoded_rd = ins >> 7 & 0x1F;
      decoded_imm = (ins & 0xFF000) | (((ins >> 20) & 1)<<11) | (((ins >> 21) & 0x3FF)<<1) | ((ins >> 31) << 20);
      uint32_t sign_bit = ins >> 31;
      if (sign_bit) {
        decoded_imm = static_cast<uint32_t>(decoded_imm) | 0xFFE00000;
      }
      full_ins_id = opcode;
    } else throw std::runtime_error("Unknown instruction in Decode");
    return std::make_tuple(full_ins_id, decoded_rd, has_decoded_rd, decoded_rs1, has_decoded_rs1, decoded_rs2,
                           has_decoded_rs2, decoded_imm, decoded_shamt);
  }

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
    // STEP1: try to commit and see if we need to rollback
    // process memory access request from LSQ
    // listen to the data from Memory and ALU
    // try to issue and check if we need to stall
    // provide the potentially missing data for instruction issued last cycle
  }
};
}  // namespace ZYM
#endif