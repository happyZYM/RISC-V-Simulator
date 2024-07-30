#pragma once
#ifndef LOADSTOREQUEUE_H
#include "tools.h"
struct LoadStoreQueue_Input {
  // receive control signal from CSU
  dark::Wire<1> reset;
  dark::Wire<1> force_clear_receiver;
  dark::Wire<1> is_issuing;
  dark::Wire<1> issue_type;
  dark::Wire<5> issue_ROB_index;
  dark::Wire<7+3+1> full_ins_id;
  dark::Wire<32> full_ins;
  dark::Wire<5> decoded_rd;
  dark::Wire<1> has_decoded_rd;
  dark::Wire<5> decoded_rs1;
  dark::Wire<1> has_decoded_rs1;
  dark::Wire<5> decoded_rs2;
  dark::Wire<1> has_decoded_rs2;
  dark::Wire<32> decoded_imm;
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
};
struct LoadStoreQueue_Private {
  dark::Register<5> LSQ_head;
  dark::Register<5> LSQ_tail;
  dark::Register<6> LSQ_remain_space;
};
struct LoadStoreQueue: public dark::Module<LoadStoreQueue_Input,LoadStoreQueue_Output,LoadStoreQueue_Private> {
    LoadStoreQueue() {
        // Constructor
    }
    void update() {
        // Update function
    }
};
#endif