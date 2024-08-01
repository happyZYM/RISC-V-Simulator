#pragma once
#include <sys/types.h>
#include <cstdint>
#include <iostream>
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
  dark::Wire<8> a0;
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
  dark::Wire<7 + 3 + 1> mem_request_full_ins_id;
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
  // dark::Register<1> cache_hit;
  // dark::Register<5> cache_hit_ROB_index;
  // dark::Register<32> cache_hit_data;
  dark::Register<1> is_committing;
  dark::Register<1> commit_has_resulting_register;
  dark::Register<5> commit_reg_index;
  dark::Register<32> commit_reg_value;
  dark::Register<5> commit_ins_ROB_index;
};
struct ROBRecordType {
  dark::Register<4> state;  // 0: no entry; 1: just issued; 2: waiting; 3: ready to commit
  dark::Register<32> instruction;
  dark::Register<1> has_resulting_register;
  dark::Register<5> resulting_register_idx;
  dark::Register<32> resulting_register_value;
  dark::Register<1> resulting_PC_ready;
  dark::Register<32> resulting_PC;
  dark::Register<1> PC_mismatch_mark;
  // dark::Register<4> mem_request_type;  // see memory.h
  // dark::Register<32> mem_request_addr;
  // dark::Register<32> mem_request_data;
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
      uint8_t funct7 = 0;
      if (opcode == 0b0010011 && funct3 == 0b101) funct7 = ins >> 25 & 0x7F;
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
    } else if (opcode == 0b0110111) {
      // U-type
      has_decoded_rd = 1;
      has_decoded_rs1 = 0;
      has_decoded_rs2 = 0;
      decoded_rd = ins >> 7 & 0x1F;
      decoded_imm = (ins >> 12) << 12;
      full_ins_id = opcode;
    } else if (opcode == 0b1101111) {
      // J-type
      has_decoded_rd = 1;
      has_decoded_rs1 = 0;
      has_decoded_rs2 = 0;
      decoded_rd = ins >> 7 & 0x1F;
      decoded_imm = (ins & 0xFF000) | (((ins >> 20) & 1) << 11) | (((ins >> 21) & 0x3FF) << 1) | ((ins >> 31) << 20);
      uint32_t sign_bit = ins >> 31;
      if (sign_bit) {
        decoded_imm = static_cast<uint32_t>(decoded_imm) | 0xFFE00000;
      }
      full_ins_id = opcode;
    } else
      throw std::runtime_error("Unknown instruction in Decode");
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
      for (auto &record : ROB_records) {
        record.state <= 0;
        record.PC_mismatch_mark <= 0;
      }
      has_instruction_issued_last_cycle <= 0;
      is_issuing <= 0;
      is_committing <= 0;
      return;
    }
    if (bool(force_clear_announcer)) {
      force_clear_announcer <= 0;
      ROB_head <= 0;
      ROB_tail <= 0;
      ROB_remain_space <= kROBSize;
      for (auto &record : ROB_records) {
        record.state <= 0;
        record.PC_mismatch_mark <= 0;
      }
      predicted_PC <= actual_PC;
      has_predicted_PC <= 1;
      has_instruction_issued_last_cycle <= 0;
      is_issuing <= 0;
      is_committing <= 0;
      return;
    }
    // STEP1: try to commit and see if we need to rollback
    uint32_t ROB_next_remain_space = static_cast<max_size_t>(ROB_remain_space);
    bool has_committed = false;
    {
      uint32_t i = static_cast<max_size_t>(ROB_head);
      auto &record = ROB_records[i];
      if (static_cast<max_size_t>(record.state) == 3) {
        ROB_head <= (static_cast<max_size_t>(ROB_head) + 1) % kROBSize;
        std::cerr << "csu is committing instruct " << std::hex << std::setw(8) << std::setfill('0') << std::uppercase
                  << static_cast<max_size_t>(record.instruction) << std::endl;
        is_committing <= 1;
        has_committed = true;
        commit_has_resulting_register <= record.has_resulting_register;
        commit_reg_index <= record.resulting_register_idx;
        commit_reg_value <= record.resulting_register_value;
        std::cerr << "commit_reg_index=" << std::dec << commit_reg_index.peek() << " commit_reg_value=" << std::hex
                  << std::setw(8) << std::setfill('0') << std::uppercase << commit_reg_value.peek() << std::endl;
        commit_ins_ROB_index <= i;
        actual_PC <= static_cast<max_size_t>(record.resulting_PC);
        if (static_cast<max_size_t>(record.PC_mismatch_mark) == 1) {
          force_clear_announcer <= 1;
          std::cerr << "[warning] csu is announcing rolling back due to PC mismatch" << std::endl;
        }
        ROB_next_remain_space++;
        if (record.instruction == 0x0ff00513) {
          halt_signal <= (0b100000000 | static_cast<max_size_t>(a0));
          std::cerr << "halting with code " << std::dec << int(halt_signal.peek()) << std::endl;
        }
        if (record.instruction == 0x1B07A503) {
          std::cerr << "judgeResult loaded from memory is " << std::dec
                    << static_cast<max_size_t>(record.resulting_register_value) << std::endl;
        }
      }
    }
    if (!has_committed) is_committing <= 0;
    if (force_clear_announcer.peek()) {
      ROB_remain_space <= ROB_next_remain_space;
      return;
    }
    // listen to the data from Memory and ALU
    auto process_data = [&](uint32_t res_ROB_index, uint32_t res_data, uint32_t res_PC) {
      uint32_t i = -1;
      for (auto &record : ROB_records) {
        ++i;
        if (static_cast<max_size_t>(record.state) != 2) continue;
        if (i == res_ROB_index) {
          record.resulting_register_value <= res_data;
          if (!bool(record.resulting_PC_ready)) {
            record.resulting_PC <= res_PC;
            if (res_PC != static_cast<max_size_t>(record.resulting_PC) &&
                (static_cast<max_size_t>(record.instruction) & 0x7F) != 0b1100111) {
              record.PC_mismatch_mark <= 1;
            }
            record.resulting_PC_ready <= 1;
            if ((static_cast<max_size_t>(record.instruction) & 0x7F) == 0b1100111) {
              has_predicted_PC <= 1;
              predicted_PC <= res_PC;
              std::cerr << "The jalr instruction is committed, now predicted_PC is " << std::hex << std::setw(8)
                        << std::setfill('0') << std::uppercase << predicted_PC.peek() << std::endl;
            }
          }
          record.state <= 3;
        }
      }
    };
    std::cerr << "csu is listening data from memory" << std::endl;
    if (static_cast<max_size_t>(mem_status_receiver) == 0b10) {
      process_data(static_cast<max_size_t>(completed_memins_ROB_index),
                   static_cast<max_size_t>(completed_memins_read_data), 0);
    }
    std::cerr << "csu is listening data from alu" << std::endl;
    if (static_cast<max_size_t>(alu_status_receiver) == 0b10) {
      process_data(static_cast<max_size_t>(completed_aluins_ROB_index),
                   static_cast<max_size_t>(completed_aluins_result),
                   static_cast<max_size_t>(completed_alu_resulting_PC));
    }
    // try to issue and check if we need to stall
    if (bool(has_predicted_PC)) {  // currently not in stall state
      uint32_t instruction = instruction_fetcher(static_cast<max_size_t>(predicted_PC));
      auto decoded_tuple = Decode(instruction);
      uint32_t full_ins_id = static_cast<max_size_t>(std::get<0>(decoded_tuple));
      uint8_t decoded_rd = static_cast<max_size_t>(std::get<1>(decoded_tuple));
      uint8_t has_decoded_rd = static_cast<max_size_t>(std::get<2>(decoded_tuple));
      uint8_t decoded_rs1 = static_cast<max_size_t>(std::get<3>(decoded_tuple));
      uint8_t has_decoded_rs1 = static_cast<max_size_t>(std::get<4>(decoded_tuple));
      uint8_t decoded_rs2 = static_cast<max_size_t>(std::get<5>(decoded_tuple));
      uint8_t has_decoded_rs2 = static_cast<max_size_t>(std::get<6>(decoded_tuple));
      uint32_t decoded_imm = static_cast<max_size_t>(std::get<7>(decoded_tuple));
      uint8_t decoded_shamt = static_cast<max_size_t>(std::get<8>(decoded_tuple));
      if ((full_ins_id & 0x7F) == 0b0000011 || (full_ins_id & 0x7F) == 0b0100011) {
        // memory instruction
        int32_t actual_remain_space = static_cast<max_size_t>(load_store_queue_emptyspace_receiver) -
                                      static_cast<max_size_t>(has_instruction_issued_last_cycle);
        if (ROB_next_remain_space > 0 && actual_remain_space > 0) {
          // can issue
          std::cerr << "csu is issuing mem instruct " << std::hex << std::setw(8) << std::setfill('0') << std::uppercase
                    << instruction << " full_ins_id= " << std::hex << std::setw(8) << std::setfill('0')
                    << std::uppercase << full_ins_id << " with ROB_index=" << std::dec
                    << static_cast<max_size_t>(ROB_tail) << std::endl;
          is_issuing <= 1;
          has_instruction_issued_last_cycle <= 1;
          uint32_t tail = static_cast<max_size_t>(ROB_tail);
          ROB_tail <= (tail + 1) % kROBSize;
          ROB_next_remain_space--;
          predicted_PC <= static_cast<max_size_t>(predicted_PC) + 4;
          ROB_records[tail].state <= 1;
          ROB_records[tail].instruction <= instruction;
          ROB_records[tail].has_resulting_register <= has_decoded_rd;
          ROB_records[tail].resulting_register_idx <= decoded_rd;
          ROB_records[tail].resulting_PC_ready <= 1;
          ROB_records[tail].resulting_PC <= static_cast<max_size_t>(predicted_PC) + 4;
          ROB_records[tail].PC_mismatch_mark <= 0;
          this->issue_type <= 1;
          this->issue_ROB_index <= tail;
          this->full_ins_id <= full_ins_id;
          this->full_ins <= instruction;
          this->issuing_PC <= static_cast<max_size_t>(predicted_PC);
          this->decoded_rd <= decoded_rd;
          this->has_decoded_rd <= has_decoded_rd;
          this->decoded_rs1 <= decoded_rs1;
          this->has_decoded_rs1 <= has_decoded_rs1;
          this->decoded_rs2 <= decoded_rs2;
          this->has_decoded_rs2 <= has_decoded_rs2;
          this->decoded_imm <= decoded_imm;
          this->decoded_shamt <= decoded_shamt;
        } else {
          has_instruction_issued_last_cycle <= 0;
          is_issuing <= 0;
        }
      } else {
        // alu instruction
        int32_t actual_remain_space = static_cast<max_size_t>(reservestation_emptyspace_receiver) -
                                      static_cast<max_size_t>(has_instruction_issued_last_cycle);
        if (ROB_next_remain_space > 0 && actual_remain_space > 0) {
          // can issue
          std::cerr << "csu is issuing alu instruct " << std::hex << std::setw(8) << std::setfill('0') << std::uppercase
                    << instruction << " full_ins_id= " << std::hex << std::setw(8) << std::setfill('0')
                    << std::uppercase << full_ins_id << " with ROB_index=" << std::dec
                    << static_cast<max_size_t>(ROB_tail) << std::endl;
          is_issuing <= 1;
          has_instruction_issued_last_cycle <= 1;
          uint32_t tail = static_cast<max_size_t>(ROB_tail);
          ROB_tail <= (tail + 1) % kROBSize;
          ROB_next_remain_space--;
          ROB_records[tail].state <= 1;
          ROB_records[tail].instruction <= instruction;
          ROB_records[tail].has_resulting_register <= has_decoded_rd;
          ROB_records[tail].resulting_register_idx <= decoded_rd;
          if ((full_ins_id & 0x7F) == 0b1100011 || ((full_ins_id & 0x7F) == 0b1100111) ||
              ((full_ins_id & 0x7F) == 0b1101111)) {
            switch (full_ins_id & 0x7F) {
              case 0b1101111:
                // jal
                ROB_records[tail].resulting_PC_ready <= 1;
                ROB_records[tail].resulting_PC <= static_cast<max_size_t>(predicted_PC) + decoded_imm;
                break;
              case 0b1100111:
                // jalr
                std::cerr << "encounter jalr" << std::endl;
                ROB_records[tail].resulting_PC_ready <= 0;
                has_predicted_PC <= 0;
                break;
              case 0b1100011:
                // branch
                ROB_records[tail].resulting_PC_ready <= 0;
                ROB_records[tail].resulting_PC <= static_cast<max_size_t>(predicted_PC) + decoded_imm;  // just guess
                break;
            }
          } else {
            ROB_records[tail].resulting_PC_ready <= 1;
            ROB_records[tail].resulting_PC <= static_cast<max_size_t>(predicted_PC) + 4;
          }
          predicted_PC <= ROB_records[tail].resulting_PC.peek();
          ROB_records[tail].PC_mismatch_mark <= 0;
          this->issue_type <= 0;
          this->issue_ROB_index <= tail;
          if (instruction == 0x0ff00513) {
            this->full_ins_id <= 1;
            has_predicted_PC <= 0;
          } else
            this->full_ins_id <= full_ins_id;
          this->full_ins <= instruction;
          this->issuing_PC <= static_cast<max_size_t>(predicted_PC);
          this->decoded_rd <= decoded_rd;
          this->has_decoded_rd <= has_decoded_rd;
          this->decoded_rs1 <= decoded_rs1;
          this->has_decoded_rs1 <= has_decoded_rs1;
          this->decoded_rs2 <= decoded_rs2;
          this->has_decoded_rs2 <= has_decoded_rs2;
          this->decoded_imm <= decoded_imm;
          this->decoded_shamt <= decoded_shamt;
        } else {
          has_instruction_issued_last_cycle <= 0;
          is_issuing <= 0;
        }
      }
    } else {
      has_instruction_issued_last_cycle <= 0;
      is_issuing <= 0;
    }
    // provide the potentially missing data for instruction issued last cycle
    if (bool(has_instruction_issued_last_cycle)) {
      std::cerr << "CSU is processing potentially missing data for instruction issued last cycle" << std::endl;
      uint8_t rs1 = static_cast<max_size_t>(this->decoded_rs1);
      uint8_t found_rs1 = 0;
      uint32_t rs1_v;
      uint8_t rs2 = static_cast<max_size_t>(this->decoded_rs2);
      uint8_t found_rs2 = 0;
      uint32_t rs2_v;
      for (uint32_t ptr = static_cast<max_size_t>(ROB_head);
           ptr != static_cast<max_size_t>(ROB_tail) && (ptr + 1) % kROBSize != static_cast<max_size_t>(ROB_tail);
           ptr = (ptr + 1) % kROBSize) {
        if (ROB_records[ptr].state.peek() == 3) {
          if (bool(ROB_records[ptr].has_resulting_register) &&
              static_cast<max_size_t>(ROB_records[ptr].resulting_register_idx) == rs1) {
            rs1_v = ROB_records[ptr].resulting_register_value.peek();
            found_rs1 = 1;
            std::cerr << "matching rs1=" << std::dec << int(rs1) << " ptr=" << std::dec << ptr << " rs1_v=" << std::hex
                      << std::setw(8) << std::setfill('0') << rs1_v << std::endl;
          }
          if (bool(ROB_records[ptr].has_resulting_register) &&
              static_cast<max_size_t>(ROB_records[ptr].resulting_register_idx) == rs2) {
            rs2_v = ROB_records[ptr].resulting_register_value.peek();
            found_rs2 = 1;
          }
        } else {
          if (bool(ROB_records[ptr].has_resulting_register) &&
              static_cast<max_size_t>(ROB_records[ptr].resulting_register_idx) == rs1) {
            found_rs1 = 0;
            std::cerr << "dematching rs1=" << std::dec << int(rs1) << " ptr=" << std::dec << ptr << std::endl;
          }
          if (bool(ROB_records[ptr].has_resulting_register) &&
              static_cast<max_size_t>(ROB_records[ptr].resulting_register_idx) == rs2) {
            found_rs2 = 0;
          }
        }
      }
      this->rs1_is_in_ROB <= found_rs1;
      this->rs1_in_ROB_value <= rs1_v;
      this->rs2_is_in_ROB <= found_rs2;
      this->rs2_in_ROB_value <= rs2_v;
    }
    // other data
    ROB_remain_space <= ROB_next_remain_space;
    for (auto &record : ROB_records) {
      if (static_cast<max_size_t>(record.state) == 1) {
        record.state <= 2;
      }
    }
  }
};
}  // namespace ZYM
#endif