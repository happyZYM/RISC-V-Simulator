#pragma once
#include <cstddef>
#include "concept.h"
#ifndef MEMORY_H
#include <cstdint>
#include <ios>
#include <vector>
#include "tools.h"
using dark::max_size_t;
namespace ZYM {
struct Memory_Input {
  dark::Wire<4> request_type_input;
  dark::Wire<32> address_input;
  dark::Wire<32> data_input;
  dark::Wire<5> request_ROB_index;
  dark::Wire<1> reset;
  dark::Wire<1> force_clear_receiver;
};
struct Memory_Output {
  dark::Register<2> data_sign;
  dark::Register<5> completed_memins_ROB_index;
  dark::Register<32> completed_memins_read_data;
};
struct Memory_Private {
  dark::Register<3> status;
  dark::Register<32> cur_opt_addr;
  dark::Register<32> cur_opt_data;
  dark::Register<2> cur_opt_type;
  dark::Register<2> cur_opt_bytes;
};
struct Memory : dark::Module<Memory_Input, Memory_Output, Memory_Private> {
 private:
  std::vector<uint8_t> memory_data;

 public:
  Memory() { memory_data.resize(1 << 20, 0); }
  void work() override final {
    if (bool(reset)) {
      // do some initialization
      status <= 0;
      data_sign <= 1;
      return;
    }
    if(bool(force_clear_receiver)) {
      status <= 0;
      data_sign <= 1;
      return;
    }
    max_size_t request_type_signal = max_size_t(request_type_input);
    uint8_t rw_type = request_type_signal & 3;           // 0b00->none,0b01->read,0b10->write,0b11->invalid
    uint8_t opt_bytes = (request_type_signal >> 2) & 3;  // 0->1, 1->2, 2->4
    if (rw_type == 3) throw std::runtime_error("Invalid request type");
    uint32_t current_status = max_size_t(status);
    if (current_status > 0) {
      // in working status
      if (request_type_signal > 0) throw std::runtime_error("Memory is busy");
      if (current_status == 1) {
        status <= 2;
        return;
      }
      status <= 0;
      if(max_size_t(cur_opt_type) == 0b01) {
        size_t len=1<<max_size_t(cur_opt_bytes);
        switch(len) {
          case 1:
            completed_memins_read_data <= memory_data[max_size_t(cur_opt_addr)];
            break;
          case 2:
            completed_memins_read_data <= *reinterpret_cast<uint16_t*>(&memory_data[max_size_t(cur_opt_addr)]);
            break;
          case 4:
            completed_memins_read_data <= *reinterpret_cast<uint32_t*>(&memory_data[max_size_t(cur_opt_addr)]);
            break;
          default:
            throw std::runtime_error("Invalid bytes");
        }
      } else {
        size_t len=1<<max_size_t(cur_opt_bytes);
        switch(len) {
          case 1:
            memory_data[max_size_t(cur_opt_addr)] = max_size_t(cur_opt_data)&0xff;
            break;
          case 2:
            *reinterpret_cast<uint16_t*>(&memory_data[max_size_t(cur_opt_addr)]) = max_size_t(cur_opt_data)&0xffff;
            break;
          case 4:
            *reinterpret_cast<uint32_t*>(&memory_data[max_size_t(cur_opt_addr)]) = max_size_t(cur_opt_data);
            break;
          default:
            throw std::runtime_error("Invalid bytes");
        }
      }
      data_sign <= 2; // has data and free
      return;
    }
    // now the memory is not busy
    if (request_type_signal == 0)  {
      data_sign <= 1; // free
      return;
    }
    status <= 1;
    data_sign <= 0; // busy
    completed_memins_ROB_index <= request_ROB_index;
    cur_opt_addr <= address_input;
    cur_opt_data <= data_input;
    cur_opt_type <= rw_type;
    cur_opt_bytes <= opt_bytes;
  }
  max_size_t FetchInstruction(max_size_t addr) {  // assume we have a super nb instruction fetch method that can fetch
                                                  // an instruction immediately
    max_size_t res;
    res = *reinterpret_cast<max_size_t *>(&memory_data[addr]);
    return res;
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
      DEBUG_CERR << "begin:" << std::hex << addr << std::endl;
      while (fin >> tmp) {
        buf.push_back(tmp);
      }
      if (memory_data.size() < addr + buf.size()) {
        memory_data.resize(addr + buf.size());
      }
      for (int i = 0; i < buf.size(); i++) {
        memory_data[addr + i] = buf[i];
        DEBUG_CERR << std::hex << addr + i << ' ' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex
                   << (int)buf[i] << std::endl;
      }
      fin.clear();
    } while (!fin.eof());
  }
};
}  // namespace ZYM
#endif  // MEMORY_H