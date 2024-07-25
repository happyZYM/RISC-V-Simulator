#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#ifdef DEBUG
#define DEBUG_CERR std::cerr
#else
#define DEBUG_CERR if(0) std::cerr
#endif
inline uint8_t ReadBit(uint32_t data, int pos) { return (data >> pos) & 1; }
inline void WriteBit(uint32_t &data, int pos, uint8_t bit) {
  data &= ~(1 << pos);
  data |= bit << pos;
}
class RV32IInterpreter;
typedef std::function<void(RV32IInterpreter &, uint32_t)> ExecuteFunc;
void Execute_lui(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_auipc(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_jal(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_jalr(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_beq(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_bne(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_blt(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_bge(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_bltu(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_bgeu(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_lb(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_lh(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_lw(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_lbu(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_lhu(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_sb(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_sh(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_sw(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_addi(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_slti(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_sltiu(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_xori(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_ori(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_andi(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_slli(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_srli(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_srai(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_add(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_sub(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_sll(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_slt(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_sltu(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_xor(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_srl(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_sra(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_or(RV32IInterpreter &interpreter, uint32_t instruction);
void Execute_and(RV32IInterpreter &interpreter, uint32_t instruction);
std::map<std::pair<uint8_t, uint8_t>, ExecuteFunc> ExecuteFuncMap = {
    {{0x37, 0}, Execute_lui},   {{0x17, 0}, Execute_auipc}, {{0x6F, 0}, Execute_jal},   {{0x67, 0}, Execute_jalr},
    {{0x63, 0}, Execute_beq},   {{0x63, 1}, Execute_bne},   {{0x63, 4}, Execute_blt},   {{0x63, 5}, Execute_bge},
    {{0x63, 6}, Execute_bltu},  {{0x63, 7}, Execute_bgeu},  {{0x03, 0}, Execute_lb},    {{0x03, 1}, Execute_lh},
    {{0x03, 2}, Execute_lw},    {{0x03, 4}, Execute_lbu},   {{0x03, 5}, Execute_lhu},   {{0x23, 0}, Execute_sb},
    {{0x23, 1}, Execute_sh},    {{0x23, 2}, Execute_sw},    {{0x13, 0}, Execute_addi},  {{0x13, 2}, Execute_slti},
    {{0x13, 3}, Execute_sltiu}, {{0x13, 4}, Execute_xori},  {{0x13, 6}, Execute_ori},   {{0x13, 7}, Execute_andi},
    {{0x13, 1}, Execute_slli},  {{0x13, 5}, Execute_srli},  {{0x13, 13}, Execute_srai}, {{0x33, 0}, Execute_add},
    {{0x33, 8}, Execute_sub},   {{0x33, 1}, Execute_sll},   {{0x33, 2}, Execute_slt},   {{0x33, 3}, Execute_sltu},
    {{0x33, 4}, Execute_xor},   {{0x33, 5}, Execute_srl},   {{0x33, 13}, Execute_sra},  {{0x33, 6}, Execute_or},
    {{0x33, 7}, Execute_and}};
ExecuteFunc Decode(uint32_t instr) {
  uint8_t opcode = instr & 127;
  uint8_t funct3 = (instr >> 12) & 7;
  uint8_t funct7 = (instr >> 25) & 127;
  if (opcode == 0x37 || opcode == 0x17 || opcode == 0x6f) funct3 = 0;
  funct7 >>= 5;
  funct7 &= 3;
  if (!(opcode == 0x13 && (funct3 == 1 || funct3 == 5)) && opcode != 0x33) {
    funct7 = 0;
  }
  uint8_t second_key = funct3 | (funct7 << 3);
  DEBUG_CERR << "Decoding, opcode=" << std::hex << (int)opcode << " second_key=" << std::dec << (int)second_key
            << std::endl;
  if (ExecuteFuncMap.find({opcode, second_key}) == ExecuteFuncMap.end()) {
    throw std::runtime_error("Unsupported instruction");
  }
  return ExecuteFuncMap[{opcode, second_key}];
}
// no I type
class RV32IInterpreter {
  std::vector<uint8_t> dat;
  uint8_t exit_code = 255;
  uint32_t PC;
  uint32_t IR;
  uint32_t reg[32];
  size_t counter;
  friend void Execute_lui(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_auipc(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_jal(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_jalr(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_beq(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_bne(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_blt(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_bge(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_bltu(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_bgeu(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_lb(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_lh(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_lw(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_lbu(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_lhu(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_sb(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_sh(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_sw(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_addi(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_slti(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_sltiu(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_xori(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_ori(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_andi(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_slli(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_srli(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_srai(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_add(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_sub(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_sll(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_slt(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_sltu(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_xor(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_srl(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_sra(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_or(RV32IInterpreter &interpreter, uint32_t instruction);
  friend void Execute_and(RV32IInterpreter &interpreter, uint32_t instruction);

  void PrintRegisters() {
    for (int i = 0; i < 32; i++) {
      DEBUG_CERR << "x" << i << "=" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << reg[i]
                << std::endl;
    }
  }

 public:
  RV32IInterpreter() {
    dat.reserve(1 << 20);
    counter = 0;
  }
  void LoadProgram(std::istream &fin) {
    fin >> std::hex;
    std::string rubbish_bin;
    do {
      while (!fin.eof() && fin.get() != '@')
        ;
      if (fin.eof()) break;
      int addr, tmp;
      std::vector<uint8_t> buf;
      fin >> addr;
      // DEBUG_CERR << "begin:" << std::hex << addr << std::endl;
      while (fin >> tmp) {
        buf.push_back(tmp);
      }
      if (dat.size() < addr + buf.size()) {
        dat.resize(addr + buf.size());
      }
      for (int i = 0; i < buf.size(); i++) {
        dat[addr + i] = buf[i];
        // DEBUG_CERR << std::hex << addr + i << ' ' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex
        // << (int)buf[i] << std::endl;
      }
      fin.clear();
    } while (!fin.eof());
    PC = 0;
    memset(reg, 0, sizeof(reg));
  }
  bool Fetch() {
    DEBUG_CERR<<"Fetching PC: "<<std::hex<<PC<<std::endl;
    IR = *reinterpret_cast<uint32_t *>(&dat[PC]);
    if (IR == 0x0FF00513) {
      // DEBUG_CERR<<"ready to exit"<<std::endl;
      return false;
    }
    return true;
  }
  void RunProgram() {
    /**
     * Begin simulation. The simulator will process RV32I instructions of U, J, B, I, S, R types (no I type).
     * The simulator will stop when it encounters an li a0,255(addi a0, zero, 255) instruction, that is, when the
     * simulator encounters it, it will do nothing and store the last 8 bits of a0 to exit_code.
     * The simulator is little-endian.
     */
    while (Fetch()) {
      // uint8_t opcode=IR&127;
      // std::cout<<"PC: "<<std::hex<<PC<<std::endl;
      PrintRegisters();
      Decode(IR)(*this, IR);
      DEBUG_CERR << std::endl;
      DEBUG_CERR<<"instruction to Fetch: "<<std::hex<<PC<<std::endl<<std::endl;
    }
    // now set exit_code
    exit_code = reg[10] & 255;
  }
  uint8_t GetExitCode() { return exit_code; }
};
int main() {
  RV32IInterpreter interpreter;
  interpreter.LoadProgram(std::cin);
  interpreter.RunProgram();
  std::cout <<std::dec<< (int)interpreter.GetExitCode() << std::endl;
  return 0;
}

void Execute_lui(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "lui: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint32_t imm = instruction & 0xFFFFF000;
  interpreter.reg[rd] = imm;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_auipc(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "auipc: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint32_t imm = instruction & 0xFFFFF000;
  interpreter.reg[rd] = interpreter.PC + imm;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_jal(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "jal: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint32_t rd = (instruction >> 7) & 31;
  interpreter.reg[rd] = interpreter.PC + 4;
  interpreter.reg[0]=0;
  uint32_t offset = 0;
  // 提取并组合立即数
  WriteBit(offset, 20, ReadBit(instruction, 31));
  for (int i = 12; i <= 19; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i));
  }
  WriteBit(offset, 11, ReadBit(instruction, 20));
  for (int i = 1; i <= 10; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 20));
  }
  WriteBit(offset, 0, 0);

  // 符号扩展
  if (ReadBit(offset, 20)) {
    for (int i = 21; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }

  // 更新PC
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  DEBUG_CERR << "offset=" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << offset << std::endl;
  DEBUG_CERR << "offset_signed=" << std::dec << offset_signed << std::endl;
  interpreter.PC += offset_signed;
  DEBUG_CERR << "now PC=" << std::hex << interpreter.PC << std::endl;
}
void Execute_jalr(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "jalr: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint32_t t = interpreter.PC + 4;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t offset = instruction >> 20;
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  interpreter.PC = (interpreter.reg[rs1] + offset_signed) & 0xFFFFFFFE;
  uint8_t rd = (instruction >> 7) & 31;
  interpreter.reg[rd] = t;
  interpreter.reg[0]=0;
  DEBUG_CERR << "now PC=" << std::hex << interpreter.PC << std::endl;
}
void Execute_beq(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "beq: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  // 构建立即数（偏移量）
  WriteBit(offset, 12, ReadBit(instruction, 31));  // imm[12]
  WriteBit(offset, 11, ReadBit(instruction, 7));   // imm[11]
  for (int i = 1; i <= 4; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 7));  // imm[10:7]
  }
  for (int i = 5; i <= 10; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 20));  // imm[4:1|11]
  }
  WriteBit(offset, 0, 0);  // imm[0] 总是0

  // 符号扩展
  if (ReadBit(offset, 12)) {
    for (int i = 13; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }

  // 比较寄存器值并决定是否跳转
  if (interpreter.reg[rs1] == interpreter.reg[rs2]) {
    int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
    interpreter.PC += offset_signed;
  } else {
    interpreter.PC += 4;  // 如果不相等，PC简单地加4（下一条指令）
  }
  DEBUG_CERR << "now PC=" << std::hex << interpreter.PC << std::endl;
}
void Execute_bne(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "bne: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  // 构建立即数（偏移量）
  WriteBit(offset, 12, ReadBit(instruction, 31));  // imm[12]
  WriteBit(offset, 11, ReadBit(instruction, 7));   // imm[11]
  for (int i = 1; i <= 4; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 7));  // imm[10:7]
  }
  for (int i = 5; i <= 10; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 20));  // imm[4:1|11]
  }
  WriteBit(offset, 0, 0);  // imm[0] 总是0

  // 符号扩展
  if (ReadBit(offset, 12)) {
    for (int i = 13; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }

  // 比较寄存器值并决定是否跳转
  if (interpreter.reg[rs1] != interpreter.reg[rs2]) {
    int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
    interpreter.PC += offset_signed;
  } else {
    interpreter.PC += 4;  // 如果不相等，PC简单地加4（下一条指令）
  }
  DEBUG_CERR << "now PC=" << std::hex << interpreter.PC << std::endl;
}
void Execute_blt(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "blt: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  // 构建立即数（偏移量）
  WriteBit(offset, 12, ReadBit(instruction, 31));  // imm[12]
  WriteBit(offset, 11, ReadBit(instruction, 7));   // imm[11]
  for (int i = 1; i <= 4; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 7));  // imm[10:7]
  }
  for (int i = 5; i <= 10; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 20));  // imm[4:1|11]
  }
  WriteBit(offset, 0, 0);  // imm[0] 总是0

  // 符号扩展
  if (ReadBit(offset, 12)) {
    for (int i = 13; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }

  // 比较寄存器值并决定是否跳转
  int32_t rs1_val = *reinterpret_cast<int32_t *>(&interpreter.reg[rs1]);
  int32_t rs2_val = *reinterpret_cast<int32_t *>(&interpreter.reg[rs2]);
  if (rs1_val < rs2_val) {
    int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
    interpreter.PC += offset_signed;
  } else {
    interpreter.PC += 4;  // 如果不相等，PC简单地加4（下一条指令）
  }
  DEBUG_CERR << "now PC=" << std::hex << interpreter.PC << std::endl;
}
void Execute_bge(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "bge: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  // 构建立即数（偏移量）
  WriteBit(offset, 12, ReadBit(instruction, 31));  // imm[12]
  WriteBit(offset, 11, ReadBit(instruction, 7));   // imm[11]
  for (int i = 1; i <= 4; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 7));  // imm[10:7]
  }
  for (int i = 5; i <= 10; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 20));  // imm[4:1|11]
  }
  WriteBit(offset, 0, 0);  // imm[0] 总是0

  // 符号扩展
  if (ReadBit(offset, 12)) {
    for (int i = 13; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }

  // 比较寄存器值并决定是否跳转
  int32_t rs1_val = *reinterpret_cast<int32_t *>(&interpreter.reg[rs1]);
  int32_t rs2_val = *reinterpret_cast<int32_t *>(&interpreter.reg[rs2]);
  if (rs1_val >= rs2_val) {
    int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
    interpreter.PC += offset_signed;
  } else {
    interpreter.PC += 4;  // 如果不相等，PC简单地加4（下一条指令）
  }
  DEBUG_CERR << "now PC=" << std::hex << interpreter.PC << std::endl;
}
void Execute_bltu(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "bltu: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  // 构建立即数（偏移量）
  WriteBit(offset, 12, ReadBit(instruction, 31));  // imm[12]
  WriteBit(offset, 11, ReadBit(instruction, 7));   // imm[11]
  for (int i = 1; i <= 4; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 7));  // imm[10:7]
  }
  for (int i = 5; i <= 10; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 20));  // imm[4:1|11]
  }
  WriteBit(offset, 0, 0);  // imm[0] 总是0

  // 符号扩展
  if (ReadBit(offset, 12)) {
    for (int i = 13; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }

  // 比较寄存器值并决定是否跳转
  if (interpreter.reg[rs1] < interpreter.reg[rs2]) {
    int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
    interpreter.PC += offset_signed;
  } else {
    interpreter.PC += 4;  // 如果不相等，PC简单地加4（下一条指令）
  }
  DEBUG_CERR << "now PC=" << std::hex << interpreter.PC << std::endl;
}
void Execute_bgeu(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "bgeu: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  // 构建立即数（偏移量）
  WriteBit(offset, 12, ReadBit(instruction, 31));  // imm[12]
  WriteBit(offset, 11, ReadBit(instruction, 7));   // imm[11]
  for (int i = 1; i <= 4; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 7));  // imm[10:7]
  }
  for (int i = 5; i <= 10; ++i) {
    WriteBit(offset, i, ReadBit(instruction, i + 20));  // imm[4:1|11]
  }
  WriteBit(offset, 0, 0);  // imm[0] 总是0

  // 符号扩展
  if (ReadBit(offset, 12)) {
    for (int i = 13; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }

  // 比较寄存器值并决定是否跳转
  if (interpreter.reg[rs1] >= interpreter.reg[rs2]) {
    int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
    interpreter.PC += offset_signed;
  } else {
    interpreter.PC += 4;  // 如果不相等，PC简单地加4（下一条指令）
  }
  DEBUG_CERR << "now PC=" << std::hex << interpreter.PC << std::endl;
}
void Execute_lb(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "lb: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t offset = instruction >> 20;
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  uint32_t addr = interpreter.reg[rs1] + offset_signed;
  uint32_t val = interpreter.dat[addr];
  if (ReadBit(val, 7)) {
    for (int i = 8; i < 32; ++i) {
      WriteBit(val, i, 1);
    }
  }
  interpreter.reg[rd] = val;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_lh(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "lh: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t offset = instruction >> 20;
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  uint32_t addr = interpreter.reg[rs1] + offset_signed;
  uint32_t val = *(reinterpret_cast<uint16_t*>(&interpreter.dat[addr]));
  if (ReadBit(val, 15)) {
    for (int i = 16; i < 32; ++i) {
      WriteBit(val, i, 1);
    }
  }
  interpreter.reg[rd] = val;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_lw(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "lw: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t offset = instruction >> 20;
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  uint32_t addr = interpreter.reg[rs1] + offset_signed;
  uint32_t val = *(reinterpret_cast<uint32_t*>(&interpreter.dat[addr]));
  interpreter.reg[rd] = val;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_lbu(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "lbu: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t offset = instruction >> 20;
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  uint32_t addr = interpreter.reg[rs1] + offset_signed;
  uint32_t val = interpreter.dat[addr];
  interpreter.reg[rd] = val;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_lhu(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "lhu: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t offset = instruction >> 20;
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  uint32_t addr = interpreter.reg[rs1] + offset_signed;
  uint32_t val = *(reinterpret_cast<uint16_t*>(&interpreter.dat[addr]));
  interpreter.reg[rd] = val;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_sb(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "sb: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  for (int i = 0; i <= 4; i++) WriteBit(offset, i, ReadBit(instruction, i + 7));
  for (int i = 5; i <= 11; i++) WriteBit(offset, i, ReadBit(instruction, i + 20));
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  uint32_t addr = interpreter.reg[rs1] + offset_signed;
  interpreter.dat[addr] = interpreter.reg[rs2] & 0xFF;
  interpreter.PC += 4;
}
void Execute_sh(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "sh: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  for (int i = 0; i <= 4; i++) WriteBit(offset, i, ReadBit(instruction, i + 7));
  for (int i = 5; i <= 11; i++) WriteBit(offset, i, ReadBit(instruction, i + 20));
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  uint32_t addr = interpreter.reg[rs1] + offset_signed;
  *reinterpret_cast<uint16_t *>(&interpreter.dat[addr]) = interpreter.reg[rs2] & 0xFFFF;
  interpreter.PC += 4;
}
void Execute_sw(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "sw: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t offset = 0;
  for (int i = 0; i <= 4; i++) WriteBit(offset, i, ReadBit(instruction, i + 7));
  for (int i = 5; i <= 11; i++) WriteBit(offset, i, ReadBit(instruction, i + 20));
  uint8_t offset_sign_bit = ReadBit(offset, 11);
  // now sext
  if (offset_sign_bit) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(offset, i, 1);
    }
  }
  int32_t offset_signed = *reinterpret_cast<int32_t *>(&offset);
  uint32_t addr = interpreter.reg[rs1] + offset_signed;
  *reinterpret_cast<uint32_t *>(&interpreter.dat[addr])=interpreter.reg[rs2];
  interpreter.PC += 4;
}
void Execute_addi(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "addi: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t imm = instruction >> 20;
  if (ReadBit(imm, 11)) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(imm, i, 1);
    }
  }
  interpreter.reg[rd] = interpreter.reg[rs1] + imm;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_slti(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "slti: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t imm = instruction >> 20;
  if (ReadBit(imm, 11)) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(imm, i, 1);
    }
  }
  int32_t rs1_val = *reinterpret_cast<int32_t *>(&interpreter.reg[rs1]);
  int32_t signed_imm = *reinterpret_cast<int32_t *>(&imm);
  interpreter.reg[rd] = rs1_val < signed_imm ? 1 : 0;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_sltiu(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "sltiu: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t imm = instruction >> 20;
  if (ReadBit(imm, 11)) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(imm, i, 1);
    }
  }
  uint32_t rs1_val = interpreter.reg[rs1];
  uint32_t unsigned_imm = imm;
  interpreter.reg[rd] = rs1_val < unsigned_imm ? 1 : 0;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_xori(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "xori: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t imm = instruction >> 20;
  if (ReadBit(imm, 11)) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(imm, i, 1);
    }
  }
  interpreter.reg[rd] = interpreter.reg[rs1] ^ imm;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_ori(RV32IInterpreter &interpreter, uint32_t instruction) {
  DEBUG_CERR << "ori: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t imm = instruction >> 20;
  if (ReadBit(imm, 11)) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(imm, i, 1);
    }
  }
  interpreter.reg[rd] = interpreter.reg[rs1] | imm;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_andi(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "andi: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint32_t imm = instruction >> 20;
  if (ReadBit(imm, 11)) {
    for (int i = 12; i < 32; ++i) {
      WriteBit(imm, i, 1);
    }
  }
  interpreter.reg[rd] = interpreter.reg[rs1] & imm;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_slli(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "slli: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t shamt = (instruction >> 20) & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] << shamt;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_srli(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "srli: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t shamt = (instruction >> 20) & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] >> shamt;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_srai(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "srai: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t shamt = (instruction >> 20) & 31;
  uint8_t sign_bit = ReadBit(interpreter.reg[rs1], 31);
  interpreter.reg[rd] = interpreter.reg[rs1] >> shamt;
  for (int i = 31; i > 31 - shamt; --i) {
    WriteBit(interpreter.reg[rd], i, sign_bit);
  }
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_add(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "add: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] + interpreter.reg[rs2];
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_sub(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "sub: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] - interpreter.reg[rs2];
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_sll(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "sll: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint8_t shamt = interpreter.reg[rs2] & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] << shamt;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_slt(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "slt: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  int32_t rs1_val = *reinterpret_cast<int32_t *>(&interpreter.reg[rs1]);
  int32_t rs2_val = *reinterpret_cast<int32_t *>(&interpreter.reg[rs2]);
  interpreter.reg[rd] = rs1_val < rs2_val ? 1 : 0;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_sltu(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "sltu: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint32_t rs1_val = interpreter.reg[rs1];
  uint32_t rs2_val = interpreter.reg[rs2];
  interpreter.reg[rd] = rs1_val < rs2_val ? 1 : 0;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_xor(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "xor: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] ^ interpreter.reg[rs2];
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_srl(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "srl: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint8_t shamt = interpreter.reg[rs2] & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] >> shamt;
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_sra(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "sra: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  uint8_t shamt = interpreter.reg[rs2] & 31;
  uint8_t sign_bit = ReadBit(interpreter.reg[rs1], 31);
  interpreter.reg[rd] = interpreter.reg[rs1] >> shamt;
  for (int i = 31; i > 31 - shamt; --i) {
    WriteBit(interpreter.reg[rd], i, sign_bit);
  }
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_or(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "or: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] | interpreter.reg[rs2];
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}
void Execute_and(RV32IInterpreter &interpreter, uint32_t instruction) {
  ++interpreter.counter;
  DEBUG_CERR << "executing ins count: " << std::dec << interpreter.counter << " PC= " << std::hex << std::uppercase
            << interpreter.PC << std::endl;
  DEBUG_CERR << "and: instruction=" << std::hex << std::setw(8) << std::setfill('0') << instruction << std::endl;
  uint8_t rd = (instruction >> 7) & 31;
  uint8_t rs1 = (instruction >> 15) & 31;
  uint8_t rs2 = (instruction >> 20) & 31;
  interpreter.reg[rd] = interpreter.reg[rs1] & interpreter.reg[rs2];
  interpreter.reg[0]=0;
  interpreter.PC += 4;
}