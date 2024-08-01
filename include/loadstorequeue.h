#pragma once
#include <cstdint>
#include <iomanip>
#include "concept.h"
#ifndef LOADSTOREQUEUE_H
#include <array>
#include "debug.h"
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
  // dark::Wire<1> cache_hit;
  // dark::Wire<5> cache_hit_ROB_index;
  // dark::Wire<32> cache_hit_data;
};
struct LoadStoreQueue_Output {
  // request signal, Memory and the L0 cache in ROB will listen to this
  dark::Register<7 + 3 + 1> mem_request_full_ins_id;
  dark::Register<4> request_type_output;
  dark::Register<5> request_ROB_index;
  dark::Register<32> request_address_output;
  dark::Register<32> request_data_output;
  dark::Register<6> LSQ_remain_space_output;
};
struct LSQ_Record {
  dark::Register<2> state;  // 0: no, 1: initializing dependency, 2: waiting for data
  dark::Register<7 + 3 + 1> full_ins_id;
  dark::Register<32> V1, V2;
  dark::Register<5> Q1, Q2;
  dark::Register<1> E1, E2;
  dark::Register<1> D1, D2;  // 1: no dependency, 0: dependency
  dark::Register<5> ins_ROB_index;
  dark::Register<32> ins_self_PC;
  dark::Register<32> ins_imm;
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
      LSQ_remain_space_output <= 32;
      LSQ_head <= 0;
      LSQ_tail <= 0;
      for (auto &record : LSQ_queue) {
        record.state <= 0;
      }
      has_accepted_ins_last_cycle <= 0;
      request_type_output <= 0;
      return;
    }
    if (bool(force_clear_receiver)) {
      LSQ_remain_space <= 32;
      LSQ_remain_space_output <= 32;
      LSQ_head <= 0;
      LSQ_tail <= 0;
      for (auto &record : LSQ_queue) {
        record.state <= 0;
      }
      has_accepted_ins_last_cycle <= 0;
      request_type_output <= 0;
      return;
    }
    uint32_t next_remain_space = static_cast<max_size_t>(LSQ_remain_space);
    if (bool(is_issuing) && issue_type == 1) {
#ifdef _DEBUG
      if (LSQ_remain_space == 0 || LSQ_remain_space > 32) throw std::runtime_error("LSQ_remain_space is out of range");
#endif
      has_accepted_ins_last_cycle <= 1;
      // now we can accept the instruction, that is, to store it in the LSQ
      uint32_t cur_queue_tail = static_cast<max_size_t>(LSQ_tail);
      last_cycle_ins_LSQ_index <= cur_queue_tail;
      LSQ_tail <= (cur_queue_tail + 1) % 32;
      next_remain_space--;
      LSQ_queue[cur_queue_tail].state <= 1;
      LSQ_queue[cur_queue_tail].full_ins_id <= full_ins_id;
      LSQ_queue[cur_queue_tail].ins_ROB_index <= issue_ROB_index;
      LSQ_queue[cur_queue_tail].ins_self_PC <= issuing_PC;
      LSQ_queue[cur_queue_tail].ins_imm <= decoded_imm;
      LSQ_queue[cur_queue_tail].E1 <= has_decoded_rs1;
      LSQ_queue[cur_queue_tail].E2 <= has_decoded_rs2;
      LSQ_queue[cur_queue_tail].D1 <= 1;  // temporarily
      LSQ_queue[cur_queue_tail].D2 <= 1;  // temporarily
      DEBUG_CERR << "LoadStoreQueue is accepting instruction" << std::endl;
      DEBUG_CERR << "\tfull_ins_id: " << std::hex << static_cast<max_size_t>(full_ins_id) << std::endl;
      DEBUG_CERR << "\tins_ROB_index: " << std::dec << static_cast<max_size_t>(issue_ROB_index) << std::endl;
      DEBUG_CERR << "\tins_self_PC: " << std::hex << std::setw(8) << std::setfill('0')
                << static_cast<max_size_t>(issuing_PC) << std::endl;
      DEBUG_CERR << "\tins_imm: " << std::hex << static_cast<max_size_t>(decoded_imm) << std::endl;
      DEBUG_CERR << "\thas_decoded_rs1: " << std::hex << std::setw(8) << std::setfill('0')
                << static_cast<max_size_t>(has_decoded_rs1) << std::endl;
      DEBUG_CERR << "\thas_decoded_rs2: " << std::hex << std::setw(8) << std::setfill('0')
                << static_cast<max_size_t>(has_decoded_rs2) << std::endl;
      DEBUG_CERR << "\tstored in positon " << std::dec << static_cast<max_size_t>(cur_queue_tail) << " of LSQ"
                << std::endl;
      // LSQ_queue[cur_queue_tail].Q1 <= decoded_rs1;  // temporarily, no use
      // LSQ_queue[cur_queue_tail].Q2 <= decoded_rs2;  // temporarily, no use
    } else
      has_accepted_ins_last_cycle <= 0;
    uint32_t last_idx = static_cast<max_size_t>(last_cycle_ins_LSQ_index);
    bool last_cycle_V1_proccessed = false;
    bool last_cycle_V2_proccessed = false;
    if (bool(has_accepted_ins_last_cycle)) {
      // now dependency info can be read from the register file, in the mean time, CSU will provide the
      // potentially missing data
      DEBUG_CERR << "LoadStoreQueue is process dependency information from register file and ROB" << std::endl;
      if (bool(LSQ_queue[last_idx].E1) && bool(rs1_nodep)) {
        LSQ_queue[last_idx].V1 <= rs1_value;
        LSQ_queue[last_idx].D1 <= 1;
        last_cycle_V1_proccessed = true;
        DEBUG_CERR << "\t from register file: LSQ_queue[last_idx].V1=" << std::hex << std::setw(8) << std::setfill('0')
                  << static_cast<max_size_t>(LSQ_queue[last_idx].V1) << std::endl;
      }
      if (bool(LSQ_queue[last_idx].E2) && bool(rs2_nodep)) {
        LSQ_queue[last_idx].V2 <= rs2_value;
        LSQ_queue[last_idx].D2 <= 1;
        last_cycle_V2_proccessed = true;
        DEBUG_CERR << "from register file: LSQ_queue[last_idx].V2=" << std::hex << std::setw(8) << std::setfill('0')
                  << static_cast<max_size_t>(LSQ_queue[last_idx].V2) << std::endl;
      }
      if (bool(LSQ_queue[last_idx].E1) && (!bool(rs1_nodep)) && bool(rs1_is_in_ROB)) {
        LSQ_queue[last_idx].V1 <= rs1_in_ROB_value;
        LSQ_queue[last_idx].D1 <= 1;
        last_cycle_V1_proccessed = true;
        DEBUG_CERR << "\t from ROB: LSQ_queue[last_idx].V1=" << std::hex << std::setw(8) << std::setfill('0')
                  << static_cast<max_size_t>(LSQ_queue[last_idx].V1) << std::endl;
      }
      if (bool(LSQ_queue[last_idx].E2) && (!bool(rs2_nodep)) && bool(rs2_is_in_ROB)) {
        LSQ_queue[last_idx].V2 <= rs2_in_ROB_value;
        LSQ_queue[last_idx].D2 <= 1;
        last_cycle_V2_proccessed = true;
        DEBUG_CERR << "from ROB: LSQ_queue[last_idx].V2=" << std::hex << std::setw(8) << std::setfill('0')
                  << static_cast<max_size_t>(LSQ_queue[last_idx].V2) << std::endl;
      }
      DEBUG_CERR << "End of processing dependency information from register file and ROB" << std::endl;
    }
    bool should_monitor_V1 =
        bool(has_accepted_ins_last_cycle) && bool(LSQ_queue[last_idx].E1) && !last_cycle_V1_proccessed;
    bool should_monitor_V2 =
        bool(has_accepted_ins_last_cycle) && bool(LSQ_queue[last_idx].E2) && !last_cycle_V2_proccessed;
    // now alu, memory may provide data to satisfy the dependency
    auto process_listend_data = [&](uint32_t res_ROB_index, uint32_t res_value) -> void {
      DEBUG_CERR << "res_ROB_index=" << std::dec << res_ROB_index << std::endl;
      DEBUG_CERR << "res_value=" << std::hex << std::setw(8) << std::setfill('0') << res_value << std::endl;
      DEBUG_CERR << "rs1_deps=" << std::dec << static_cast<max_size_t>(rs1_deps) << std::endl;
      DEBUG_CERR << "rs2_deps=" << std::dec << static_cast<max_size_t>(rs2_deps) << std::endl;
      uint32_t ptr = static_cast<max_size_t>(LSQ_head);
      while (ptr != static_cast<max_size_t>(LSQ_tail)) {
        DEBUG_CERR << "\tptr=" << std::dec << ptr << std::endl;
        if ((!bool(has_accepted_ins_last_cycle)) || ptr != last_idx) {
          DEBUG_CERR << "\tnormal" << std::endl;
          dark::debug::assert(LSQ_queue[ptr].state == 2, "LSQ_queue[ptr].state != 2");
          if ((!bool(LSQ_queue[ptr].D1)) && static_cast<max_size_t>(LSQ_queue[ptr].Q1) == res_ROB_index) {
            LSQ_queue[ptr].V1 <= res_value;
            LSQ_queue[ptr].D1 <= 1;
          }
          if ((!bool(LSQ_queue[ptr].D2)) && static_cast<max_size_t>(LSQ_queue[ptr].Q2) == res_ROB_index) {
            LSQ_queue[ptr].V2 <= res_value;
            LSQ_queue[ptr].D2 <= 1;
          }
        } else {
          DEBUG_CERR << "\timmediately listend data" << std::endl;
          DEBUG_CERR << "should_monitor_V1=" << should_monitor_V1 << std::endl;
          DEBUG_CERR << "should_monitor_V2=" << should_monitor_V2 << std::endl;
          if (should_monitor_V1 && static_cast<max_size_t>(rs1_deps) == res_ROB_index) {
            DEBUG_CERR << "load rs1" << std::endl;
            LSQ_queue[last_idx].V1 <= res_value;
            LSQ_queue[last_idx].D1 <= 1;
            should_monitor_V1 = false;
          }
          if (should_monitor_V2 && static_cast<max_size_t>(rs2_deps) == res_ROB_index) {
            DEBUG_CERR << "load rs2" << std::endl;
            LSQ_queue[last_idx].V2 <= res_value;
            LSQ_queue[last_idx].D2 <= 1;
            should_monitor_V2 = false;
          }
        }
        ptr = (ptr + 1) % 32;
      }
    };
    DEBUG_CERR << "Load Store Queue is listening data from alu" << std::endl;
    if (static_cast<max_size_t>(alu_status_receiver) == 0b10) {
      DEBUG_CERR << "potentially have sth from alu" << std::endl;
      process_listend_data(static_cast<max_size_t>(completed_aluins_ROB_index),
                           static_cast<max_size_t>(completed_aluins_result));
    }
    DEBUG_CERR << "Load Store Queue is listening data from memory" << std::endl;
    if (static_cast<max_size_t>(mem_data_sign) == 0b10) {
      DEBUG_CERR << "potentially have sth from memory" << std::endl;
      process_listend_data(static_cast<max_size_t>(completed_memins_ROB_index),
                           static_cast<max_size_t>(completed_memins_read_data));
    }
    // if (static_cast<max_size_t>(cache_hit) == 1) {
    // process_listend_data(static_cast<max_size_t>(cache_hit_ROB_index), static_cast<max_size_t>(cache_hit_data));
    // }
    if (should_monitor_V1) {
      LSQ_queue[last_idx].D1 <= 0;
      LSQ_queue[last_idx].Q1 <= rs1_deps;
    }
    if (should_monitor_V2) {
      LSQ_queue[last_idx].D2 <= 0;
      LSQ_queue[last_idx].Q2 <= rs2_deps;
    }
    // TODO: now, we can check if we can execute the instruction, memory and L0 cache will listen to this
    // other data
    if (bool(has_accepted_ins_last_cycle)) LSQ_queue[last_idx].state <= 2;
    bool can_execute = false;
    if (static_cast<uint32_t>(mem_data_sign) > 0 && static_cast<max_size_t>(request_type_output) == 0) {
      if (static_cast<uint32_t>(LSQ_head) != static_cast<uint32_t>(LSQ_tail)) {
        uint32_t head = static_cast<uint32_t>(LSQ_head);
        if (LSQ_queue[head].state == 2) {
          if (((LSQ_queue[head].E1 == 0) || (LSQ_queue[head].E1 == 1 && LSQ_queue[head].D1 == 1)) &&
              ((LSQ_queue[head].E2 == 0) || (LSQ_queue[head].E2 == 1 && LSQ_queue[head].D2 == 1))) {
            // now we can execute the instruction
            DEBUG_CERR << "Load Store queue is executing instruction" << std::endl;
            next_remain_space++;
            can_execute = true;
            LSQ_head <= (head + 1) % 32;
            uint32_t ins = static_cast<uint32_t>(LSQ_queue[head].full_ins_id);
            if (ins == 0b00000000011) {
              // lb
              mem_request_full_ins_id <= ins;
              request_type_output <= 0b0001;
              request_ROB_index <= static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index);
              request_address_output <=
                  (static_cast<uint32_t>(LSQ_queue[head].V1) + static_cast<uint32_t>(LSQ_queue[head].ins_imm));
            } else if (ins == 0b00010000011) {
              // lh
              mem_request_full_ins_id <= ins;
              request_type_output <= 0b0101;
              request_ROB_index <= static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index);
              request_address_output <=
                  (static_cast<uint32_t>(LSQ_queue[head].V1) + static_cast<uint32_t>(LSQ_queue[head].ins_imm));
            } else if (ins == 0b00100000011) {
              // lw
              mem_request_full_ins_id <= ins;
              request_type_output <= 0b1001;
              request_ROB_index <= static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index);
              request_address_output <=
                  (static_cast<uint32_t>(LSQ_queue[head].V1) + static_cast<uint32_t>(LSQ_queue[head].ins_imm));
            } else if (ins == 0b01000000011) {
              // lbu
              mem_request_full_ins_id <= ins;
              request_type_output <= 0b0001;
              request_ROB_index <= static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index);
              request_address_output <=
                  (static_cast<uint32_t>(LSQ_queue[head].V1) + static_cast<uint32_t>(LSQ_queue[head].ins_imm));
            } else if (ins == 0b01010000011) {
              // lhu
              mem_request_full_ins_id <= ins;
              request_type_output <= 0b0101;
              request_ROB_index <= static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index);
              request_address_output <=
                  (static_cast<uint32_t>(LSQ_queue[head].V1) + static_cast<uint32_t>(LSQ_queue[head].ins_imm));
            } else if (ins == 0b00000100011) {
              // sb
              mem_request_full_ins_id <= ins;
              request_type_output <= 0b0010;
              request_ROB_index <= static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index);
              request_address_output <=
                  (static_cast<uint32_t>(LSQ_queue[head].V1) + static_cast<uint32_t>(LSQ_queue[head].ins_imm));
              request_data_output <= (static_cast<uint32_t>(LSQ_queue[head].V2) & 0xFF);
            } else if (ins == 0b00010100011) {
              // sh
              mem_request_full_ins_id <= ins;
              request_type_output <= 0b0110;
              request_ROB_index <= static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index);
              request_address_output <=
                  (static_cast<uint32_t>(LSQ_queue[head].V1) + static_cast<uint32_t>(LSQ_queue[head].ins_imm));
              request_data_output <= (static_cast<uint32_t>(LSQ_queue[head].V2) & 0xFFFF);
            } else if (ins == 0b00100100011) {
              // sw
              mem_request_full_ins_id <= ins;
              request_type_output <= 0b1010;
              request_ROB_index <= static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index);
              request_address_output <=
                  (static_cast<uint32_t>(LSQ_queue[head].V1) + static_cast<uint32_t>(LSQ_queue[head].ins_imm));
              DEBUG_CERR << "\trequest_address_output=" << std::hex << std::setfill('0') << std::setw(8)
                        << request_address_output.peek() << std::endl;
              DEBUG_CERR << "\toperand1=" << std::hex << std::setfill('0') << std::setw(8)
                        << static_cast<uint32_t>(LSQ_queue[head].V1) << std::endl;
              DEBUG_CERR << "\timm=" << std::hex << std::setfill('0') << std::setw(8)
                        << static_cast<uint32_t>(LSQ_queue[head].ins_imm) << std::endl;
              DEBUG_CERR << "\tROB_index=" << std::dec << static_cast<uint32_t>(LSQ_queue[head].ins_ROB_index)
                        << std::endl;
              request_data_output <= static_cast<uint32_t>(LSQ_queue[head].V2);
            } else {
              throw std::runtime_error("Invalid instruction");
            }
          }
        }
      }
    }
    if (!can_execute) request_type_output <= 0;
    LSQ_remain_space <= next_remain_space;
    LSQ_remain_space_output <= next_remain_space;
    DEBUG_CERR << "LSQ_queue[16]'s V1: " << std::hex << std::setfill('0') << std::setw(8)
              << static_cast<max_size_t>(LSQ_queue[16].V1) << std::endl;
  }
};
}  // namespace ZYM
#endif