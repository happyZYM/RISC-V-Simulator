#pragma once
#include "concept.h"
#ifndef ALU_H
#include "tools.h"
namespace ZYM {
struct ALU_Input {
  dark::Wire<7 + 3 + 1> request_full_id;
  dark::Wire<32> operand1;
  dark::Wire<32> operand2;
  dark::Wire<32> imm;
  dark::Wire<6> shamt;
  dark::Wire<5> request_ROB_index;
  dark::Wire<32> request_PC;
};
struct ALU_Output {
  dark::Register<2> alu_status;
  dark::Register<5> result_ROB_index;
  dark::Register<32> result;
  dark::Register<32> completed_alu_resulting_PC;
};
struct ALU : public dark::Module<ALU_Input, ALU_Output> {
  ALU() {
    // Constructor
  }
  void work() {
    std::cerr << "ALU: cur request_full_id=" << std::hex << std::setw(8) << std::setfill('0') << std::uppercase
              << static_cast<max_size_t>(request_full_id) << std::endl;
    switch (static_cast<max_size_t>(request_full_id)) {
      case 0: {
        alu_status <= 0b01;
        return;
      }
      case 0b00000000001: {
        // fake instruction of halt
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        return;
      }
      case 0b00000110111: {
        // lui
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= imm;
        std::cerr << "lui: imm=" << std::hex << static_cast<max_size_t>(imm) << std::endl;
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00000010111: {
        // auipc
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= static_cast<max_size_t>(request_PC) + imm;
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00001101111: {
        // jal
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= static_cast<max_size_t>(request_PC) + 4;
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + imm;
        return;
      }
      case 0b00001100111: {
        // jalr
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= static_cast<max_size_t>(request_PC) + 4;
        completed_alu_resulting_PC <= ((static_cast<max_size_t>(operand1) + static_cast<max_size_t>(imm)) & 0xfffffffe);
        return;
      }
      case 0b00001100011: {
        // beq
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<max_size_t>(operand1) == static_cast<max_size_t>(operand2)) {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + imm;
        } else {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        }
        return;
      }
      case 0b00011100011: {
        // bne
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<max_size_t>(operand1) != static_cast<max_size_t>(operand2)) {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + imm;
        } else {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        }
        return;
      }
      case 0b01001100011: {
        // blt
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<int32_t>(static_cast<max_size_t>(operand1)) <
            static_cast<int32_t>(static_cast<max_size_t>(operand2))) {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + imm;
        } else {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        }
        return;
      }
      case 0b01011100011: {
        // bge
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<int32_t>(static_cast<max_size_t>(operand1)) >=
            static_cast<int32_t>(static_cast<max_size_t>(operand2))) {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + imm;
        } else {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        }
        return;
      }
      case 0b01101100011: {
        // bltu
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<max_size_t>(operand1) < static_cast<max_size_t>(operand2)) {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + imm;
        } else {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        }
        return;
      }
      case 0b01111100011: {
        // bgeu
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<max_size_t>(operand1) >= static_cast<max_size_t>(operand2)) {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + imm;
        } else {
          completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        }
        return;
      }
      case 0b00000010011: {
        // addi
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= static_cast<max_size_t>(operand1) + imm;
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00100010011: {
        // slti
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<int32_t>(static_cast<max_size_t>(operand1)) <
            static_cast<int32_t>(static_cast<max_size_t>(imm))) {
          result <= 1;
        } else {
          result <= 0;
        }
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00110010011: {
        // sltiu
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<max_size_t>(operand1) < static_cast<max_size_t>(imm)) {
          result <= 1;
        } else {
          result <= 0;
        }
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b01000010011: {
        // xori
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= (static_cast<max_size_t>(operand1) ^ static_cast<max_size_t>(imm));
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b01100010011: {
        // ori
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= (static_cast<max_size_t>(operand1) | static_cast<max_size_t>(imm));
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b01110010011: {
        // andi
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= (static_cast<max_size_t>(operand1) & static_cast<max_size_t>(imm));
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00010010011: {
        // slli
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= (static_cast<max_size_t>(operand1) << static_cast<max_size_t>(shamt));
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b01010010011: {
        // srli
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= (static_cast<max_size_t>(operand1) >> static_cast<max_size_t>(shamt));
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b11010010011: {
        // srai
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        uint32_t res_tmp = static_cast<max_size_t>(operand1) >> static_cast<max_size_t>(shamt);
        uint8_t sign = (static_cast<max_size_t>(operand1) >> 31) & 1;
        if (sign) {
          for (size_t i = 0; i < static_cast<max_size_t>(shamt); i++) {
            res_tmp |= 1 << (31 - i);
          }
        }
        result <= res_tmp;
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00000110011: {
        // add
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= static_cast<max_size_t>(operand1) + static_cast<max_size_t>(operand2);
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b10000110011: {
        // sub
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= static_cast<max_size_t>(operand1) - static_cast<max_size_t>(operand2);
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00010110011: {
        // sll
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        uint8_t actual_shamt = static_cast<max_size_t>(operand2) & 0b11111;
        result <= (static_cast<max_size_t>(operand1) << actual_shamt);
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00100110011: {
        // slt
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<int32_t>(static_cast<max_size_t>(operand1)) <
            static_cast<int32_t>(static_cast<max_size_t>(operand2))) {
          result <= 1;
        } else {
          result <= 0;
        }
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b00110110011: {
        // sltu
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        if (static_cast<max_size_t>(operand1) < static_cast<max_size_t>(operand2)) {
          result <= 1;
        } else {
          result <= 0;
        }
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b01000110011: {
        // xor
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= (static_cast<max_size_t>(operand1) ^ static_cast<max_size_t>(operand2));
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b01010110011: {
        // srl
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        uint8_t actual_shamt = static_cast<max_size_t>(operand2) & 0b11111;
        result <= (static_cast<max_size_t>(operand1) >> actual_shamt);
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b11010110011: {
        // sra
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        uint8_t actual_shamt = static_cast<max_size_t>(operand2) & 0b11111;
        uint32_t res_tmp = static_cast<max_size_t>(operand1) >> actual_shamt;
        uint8_t sign = (static_cast<max_size_t>(operand1) >> 31) & 1;
        if (sign) {
          for (size_t i = 0; i < actual_shamt; i++) {
            res_tmp |= 1 << (31 - i);
          }
        }
        result <= res_tmp;
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b01100110011: {
        // or
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= (static_cast<max_size_t>(operand1) | static_cast<max_size_t>(operand2));
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      case 0b01110110011: {
        // and
        alu_status <= 0b10;
        result_ROB_index <= request_ROB_index;
        result <= (static_cast<max_size_t>(operand1) & static_cast<max_size_t>(operand2));
        completed_alu_resulting_PC <= static_cast<max_size_t>(request_PC) + 4;
        return;
      }
      default:
        throw std::runtime_error("Invalid instruction occured in ALU");
    }
  }
};
}  // namespace ZYM
#endif