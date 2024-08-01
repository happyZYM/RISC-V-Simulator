#pragma once
#include <cstddef>
#include "concept.h"
#ifndef MEMORY_H
#include <cstdint>
#include <ios>
#include <set>
#include <vector>
#include "tools.h"
using dark::max_size_t;
namespace ZYM {
struct Memory_Input {
  dark::Wire<1> reset;
  dark::Wire<1> force_clear_receiver;
  dark::Wire<7 + 3 + 1> full_ins_id;
  dark::Wire<4> request_type_input;
  dark::Wire<32> address_input;
  dark::Wire<32> data_input;
  dark::Wire<5> request_ROB_index;
  dark::Wire<1> is_committing;
  dark::Wire<5> commit_ins_ROB_index;
};
struct Memory_Output {
  dark::Register<2> data_sign;
  dark::Register<5> completed_memins_ROB_index;
  dark::Register<32> completed_memins_read_data;
};
struct Change {
  dark::Register<1> this_byte_changed;
  dark::Register<32> addr;
  dark::Register<8> before;
};
struct OperationPlayback {
  dark::Register<1> has_uncommitted_write;
  dark::Register<32> timestamp;
  std::array<Change, 4> changes;
};
struct Memory_Private {
  dark::Register<3> status;
  dark::Register<32> cur_opt_addr;
  dark::Register<32> cur_opt_data;
  dark::Register<2> cur_opt_type;
  dark::Register<2> cur_opt_bytes;
  std::array<OperationPlayback, 32> playback;
  dark::Register<32> cur_timestamp;
};
struct Memory : dark::Module<Memory_Input, Memory_Output, Memory_Private> {
 private:
  std::vector<uint8_t> memory_data;
  void Undo() {
    std::set<std::pair<uint32_t, std::pair<uint32_t, uint8_t>>> undo_list;  // (timestamp, (addr, before))
    for (int i = 0; i < 32; i++) {
      if (bool(playback[i].has_uncommitted_write)) {
        for (int j = 0; j < 4; j++) {
          if (bool(playback[i].changes[j].this_byte_changed)) {
            undo_list.insert(std::make_pair(static_cast<max_size_t>(playback[i].timestamp),
                                            std::make_pair(static_cast<max_size_t>(playback[i].changes[j].addr),
                                                           static_cast<max_size_t>(playback[i].changes[j].before))));
          }
        }
      }
    }
    size_t sz = undo_list.size();
    auto it = undo_list.end();
    for (int i = 0; i < sz; i++) {
      it--;
      memory_data[it->second.first] = it->second.second;
    }
  }

 public:
  Memory() { memory_data.resize(1 << 20, 0); }
  void work() override final {
    if (bool(reset)) {
      // do some initialization
      status <= 0;
      data_sign <= 1;
      cur_timestamp <= 0;
      for (int i = 0; i < 32; i++) playback[i].has_uncommitted_write <= 0;
      return;
    }
    if (bool(force_clear_receiver)) {
      status <= 0;
      data_sign <= 1;
      Undo();
      cur_timestamp <= 0;
      for (int i = 0; i < 32; i++) playback[i].has_uncommitted_write <= 0;
      return;
    }
    if (bool(is_committing)) {
      playback[static_cast<max_size_t>(commit_ins_ROB_index)].has_uncommitted_write <= 0;
    }
    cur_timestamp <= static_cast<max_size_t>(cur_timestamp) + 1;
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
      if (max_size_t(cur_opt_type) == 0b01) {
        size_t len = 1 << max_size_t(cur_opt_bytes);
        switch (len) {
          case 1: {
            uint32_t tmp = static_cast<max_size_t>(memory_data[max_size_t(cur_opt_addr)]);
            if (static_cast<max_size_t>(full_ins_id) == 0b01000000011) {
              // sign exetend
              if (tmp & 0x80) {
                tmp |= 0xffffff00;
              }
            }
            completed_memins_read_data <= tmp;
            std::cerr << "memory read: " << std::hex << std::setfill('0') << std::setw(2) << tmp << " from " << std::hex
                      << static_cast<max_size_t>(cur_opt_addr) << std::endl;
            break;
          }
          case 2: {
            uint32_t tmp = *reinterpret_cast<uint16_t *>(&memory_data[max_size_t(cur_opt_addr)]);
            if (static_cast<max_size_t>(full_ins_id) == 0b01010000011) {
              // sign exetend
              if (tmp & 0x8000) {
                tmp |= 0xffff0000;
              }
            }
            completed_memins_read_data <= tmp;
            std::cerr << "memory read: " << std::hex << std::setfill('0') << std::setw(4) << tmp << " from " << std::hex
                      << static_cast<max_size_t>(cur_opt_addr) << std::endl;
            break;
          }
          case 4:
            completed_memins_read_data <= *reinterpret_cast<uint32_t *>(&memory_data[max_size_t(cur_opt_addr)]);
            std::cerr << "memory read: " << std::hex << std::setfill('0') << std::setw(8)
                      << *reinterpret_cast<uint32_t *>(&memory_data[max_size_t(cur_opt_addr)]) << " from " << std::hex
                      << static_cast<max_size_t>(cur_opt_addr) << std::endl;
            break;
          default:
            throw std::runtime_error("Invalid bytes");
        }
        data_sign <= 2;  // has data and free
        return;
      } else {
        size_t len = 1 << max_size_t(cur_opt_bytes);
        uint32_t cur_opt_ROB_index = static_cast<max_size_t>(completed_memins_ROB_index);
        switch (len) {
          case 1:
            playback[cur_opt_ROB_index].has_uncommitted_write <= 1;
            playback[cur_opt_ROB_index].timestamp <= cur_timestamp;
            playback[cur_opt_ROB_index].changes[0].this_byte_changed <= 1;
            playback[cur_opt_ROB_index].changes[0].addr <= cur_opt_addr;
            playback[cur_opt_ROB_index].changes[0].before <= memory_data[max_size_t(cur_opt_addr)];
            playback[cur_opt_ROB_index].changes[1].this_byte_changed <= 0;
            playback[cur_opt_ROB_index].changes[2].this_byte_changed <= 0;
            playback[cur_opt_ROB_index].changes[3].this_byte_changed <= 0;
            memory_data[max_size_t(cur_opt_addr)] = max_size_t(cur_opt_data) & 0xff;
            break;
          case 2:
            playback[cur_opt_ROB_index].has_uncommitted_write <= 1;
            playback[cur_opt_ROB_index].timestamp <= cur_timestamp;
            playback[cur_opt_ROB_index].changes[0].this_byte_changed <= 1;
            playback[cur_opt_ROB_index].changes[0].addr <= cur_opt_addr;
            playback[cur_opt_ROB_index].changes[0].before <= memory_data[max_size_t(cur_opt_addr)];
            playback[cur_opt_ROB_index].changes[1].this_byte_changed <= 1;
            playback[cur_opt_ROB_index].changes[1].addr <= cur_opt_addr + 1;
            playback[cur_opt_ROB_index].changes[1].before <= memory_data[max_size_t(cur_opt_addr) + 1];
            playback[cur_opt_ROB_index].changes[2].this_byte_changed <= 0;
            playback[cur_opt_ROB_index].changes[3].this_byte_changed <= 0;
            *reinterpret_cast<uint16_t *>(&memory_data[max_size_t(cur_opt_addr)]) = max_size_t(cur_opt_data) & 0xffff;
            break;
          case 4:
            playback[cur_opt_ROB_index].has_uncommitted_write <= 1;
            playback[cur_opt_ROB_index].timestamp <= cur_timestamp;
            playback[cur_opt_ROB_index].changes[0].this_byte_changed <= 1;
            playback[cur_opt_ROB_index].changes[0].addr <= cur_opt_addr;
            playback[cur_opt_ROB_index].changes[0].before <= memory_data[max_size_t(cur_opt_addr)];
            playback[cur_opt_ROB_index].changes[1].this_byte_changed <= 1;
            playback[cur_opt_ROB_index].changes[1].addr <= cur_opt_addr + 1;
            playback[cur_opt_ROB_index].changes[1].before <= memory_data[max_size_t(cur_opt_addr) + 1];
            playback[cur_opt_ROB_index].changes[2].this_byte_changed <= 1;
            playback[cur_opt_ROB_index].changes[2].addr <= cur_opt_addr + 2;
            playback[cur_opt_ROB_index].changes[2].before <= memory_data[max_size_t(cur_opt_addr) + 2];
            playback[cur_opt_ROB_index].changes[3].this_byte_changed <= 1;
            playback[cur_opt_ROB_index].changes[3].addr <= cur_opt_addr + 3;
            playback[cur_opt_ROB_index].changes[3].before <= memory_data[max_size_t(cur_opt_addr) + 3];
            *reinterpret_cast<uint32_t *>(&memory_data[max_size_t(cur_opt_addr)]) = max_size_t(cur_opt_data);
            std::cerr << "Memory executing sw, ROB_index=" << std::dec
                      << static_cast<max_size_t>(completed_memins_ROB_index) << std::endl;
            std::cerr << "\taddr=" << std::hex << std::setfill('0') << std::setw(8)
                      << static_cast<max_size_t>(cur_opt_addr) << std::endl;
            std::cerr << "\tdata=" << std::hex << std::setfill('0') << std::setw(8)
                      << static_cast<max_size_t>(cur_opt_data) << std::endl;
            break;
          default:
            throw std::runtime_error("Invalid bytes");
        }
        data_sign <= 2;  // free
        return;
      }
    }
    // now the memory is not busy
    if (request_type_signal == 0) {
      data_sign <= 1;  // free
      return;
    }
    status <= 1;
    data_sign <= 0;  // busy
    completed_memins_ROB_index <= request_ROB_index;
    cur_opt_addr <= address_input;
    cur_opt_data <= data_input;
    cur_opt_type <= rw_type;
    cur_opt_bytes <= opt_bytes;
    std::cerr << "Memory is accepting a request" << std::endl;
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
      // DEBUG_CERR << "begin:" << std::hex << addr << std::endl;
      while (fin >> tmp) {
        buf.push_back(tmp);
      }
      if (memory_data.size() < addr + buf.size()) {
        memory_data.resize(addr + buf.size());
      }
      for (int i = 0; i < buf.size(); i++) {
        memory_data[addr + i] = buf[i];
        // DEBUG_CERR << std::hex << addr + i << ' ' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex
        //  << (int)buf[i] << std::endl;
      }
      fin.clear();
    } while (!fin.eof());
  }
};
}  // namespace ZYM
#endif  // MEMORY_H