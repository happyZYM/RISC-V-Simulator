#include <iostream>
#include "alu.h"
#include "csu.h"
#include "loadstorequeue.h"
#include "memory.h"
#include "registerfile.h"
#include "reservestation.h"
#include "tools.h"
template <std::size_t N>
inline static void RWConnect(dark::Register<N> &src, dark::Wire<N> &dest) {
  dest.assign([&]() -> auto & { return src; });
}
int main(int argc, char **argv) {
  dark::CPU cpu;
  ZYM::CentralScheduleUnit csu;
  ZYM::Memory memory;
  ZYM::LoadStoreQueue lsq;
  ZYM::ALU alu;
  ZYM::RegisterFile rf;
  ZYM::ReserveStation rs;
  cpu.add_module(&csu);
  cpu.add_module(&memory);
  cpu.add_module(&lsq);
  cpu.add_module(&alu);
  cpu.add_module(&rf);
  cpu.add_module(&rs);
  // some basic siganls
  cpu.halt_signal.assign([&]() -> auto & { return csu.halt_signal; });
  csu.SetInstructionFetcher([&](auto addr) { return memory.FetchInstruction(addr); });
  csu.reset.assign([&]() { return cpu.GetResetSignal(); });
  memory.reset.assign([&]() { return cpu.GetResetSignal(); });
  memory.LoadProgram(std::cin);
  lsq.reset.assign([&]() { return cpu.GetResetSignal(); });
  // alu.reset.assign([&]() { return cpu.GetResetSignal(); });
  rf.reset.assign([&]() { return cpu.GetResetSignal(); });
  rs.reset.assign([&]() { return cpu.GetResetSignal(); });
  // now connect the wires, see the comment and docs for help
  // csu <-> memory
  RWConnect(csu.force_clear_announcer, memory.force_clear_receiver);
  RWConnect(memory.data_sign, csu.mem_status_receiver);
  RWConnect(memory.completed_memins_ROB_index, csu.completed_memins_ROB_index);
  RWConnect(memory.completed_memins_read_data, csu.completed_memins_read_data);
  // csu <-> lsq
  RWConnect(csu.force_clear_announcer, lsq.force_clear_receiver);
  RWConnect(csu.is_issuing, lsq.is_issuing);
  RWConnect(csu.issue_type, lsq.issue_type);
  RWConnect(csu.issue_ROB_index, lsq.issue_ROB_index);
  RWConnect(csu.full_ins_id, lsq.full_ins_id);
  RWConnect(csu.full_ins, lsq.full_ins);
  RWConnect(csu.issuing_PC, lsq.issuing_PC);
  RWConnect(csu.decoded_rd, lsq.decoded_rd);
  RWConnect(csu.has_decoded_rd, lsq.has_decoded_rd);
  RWConnect(csu.decoded_rs1, lsq.decoded_rs1);
  RWConnect(csu.has_decoded_rs1, lsq.has_decoded_rs1);
  RWConnect(csu.rs1_is_in_ROB, lsq.rs1_is_in_ROB);
  RWConnect(csu.rs1_in_ROB_value, lsq.rs1_in_ROB_value);
  RWConnect(csu.decoded_rs2, lsq.decoded_rs2);
  RWConnect(csu.has_decoded_rs2, lsq.has_decoded_rs2);
  RWConnect(csu.rs2_is_in_ROB, lsq.rs2_is_in_ROB);
  RWConnect(csu.rs2_in_ROB_value, lsq.rs2_in_ROB_value);
  RWConnect(csu.decoded_imm, lsq.decoded_imm);
  RWConnect(csu.cache_hit, lsq.cache_hit);
  RWConnect(csu.cache_hit_ROB_index, lsq.cache_hit_ROB_index);
  RWConnect(csu.cache_hit_data, lsq.cache_hit_data);
  RWConnect(lsq.request_type_output, csu.mem_request_type_input);
  RWConnect(lsq.request_ROB_index, csu.mem_request_ROB_index);
  RWConnect(lsq.request_address_output, csu.mem_address_input);
  RWConnect(lsq.request_data_output, csu.mem_data_input);
  RWConnect(lsq.LSQ_remain_space_output, csu.load_store_queue_emptyspace_receiver);
  // csu <-> alu
  RWConnect(alu.alu_status, csu.alu_status_receiver);
  RWConnect(alu.result_ROB_index, csu.completed_aluins_ROB_index);
  RWConnect(alu.result, csu.completed_aluins_result);
  RWConnect(alu.completed_alu_resulting_PC, csu.completed_alu_resulting_PC);
  // csu <-> register file
  RWConnect(csu.force_clear_announcer, rf.force_clear_receiver);
  RWConnect(csu.is_issuing, rf.is_issuing);
  RWConnect(csu.issue_type, rf.issue_type);
  RWConnect(csu.issue_ROB_index, rf.issue_ROB_index);
  RWConnect(csu.full_ins_id, rf.full_ins_id);
  RWConnect(csu.full_ins, rf.full_ins);
  RWConnect(csu.decoded_rd, rf.decoded_rd);
  RWConnect(csu.has_decoded_rd, rf.has_decoded_rd);
  RWConnect(csu.decoded_rs1, rf.decoded_rs1);
  RWConnect(csu.has_decoded_rs1, rf.has_decoded_rs1);
  RWConnect(csu.decoded_rs2, rf.decoded_rs2);
  RWConnect(csu.has_decoded_rs2, rf.has_decoded_rs2);
  // RWConnect(rf.rs1_nodep, csu.rs1_nodep);
  // RWConnect(rf.rs1_deps, csu.rs1_deps);
  // RWConnect(rf.rs2_nodep, csu.rs2_nodep);
  // RWConnect(rf.rs2_deps, csu.rs2_deps);
  RWConnect(csu.is_committing, rf.is_committing);
  RWConnect(csu.commit_reg_index, rf.commit_reg_index);
  RWConnect(csu.commit_reg_value, rf.commit_reg_value);
  RWConnect(csu.commit_ins_ROB_index, rf.commit_ins_ROB_index);
  // csu <-> reserve station
  RWConnect(csu.force_clear_announcer, rs.force_clear_receiver);
  RWConnect(csu.is_issuing, rs.is_issuing);
  RWConnect(csu.issue_type, rs.issue_type);
  RWConnect(csu.issue_ROB_index, rs.issue_ROB_index);
  RWConnect(csu.full_ins_id, rs.full_ins_id);
  RWConnect(csu.full_ins, rs.full_ins);
  RWConnect(csu.issuing_PC, rs.issuing_PC);
  RWConnect(csu.decoded_rd, rs.decoded_rd);
  RWConnect(csu.has_decoded_rd, rs.has_decoded_rd);
  RWConnect(csu.decoded_rs1, rs.decoded_rs1);
  RWConnect(csu.has_decoded_rs1, rs.has_decoded_rs1);
  RWConnect(csu.rs1_is_in_ROB, rs.rs1_is_in_ROB);
  RWConnect(csu.rs1_in_ROB_value, rs.rs1_in_ROB_value);
  RWConnect(csu.decoded_rs2, rs.decoded_rs2);
  RWConnect(csu.has_decoded_rs2, rs.has_decoded_rs2);
  RWConnect(csu.rs2_is_in_ROB, rs.rs2_is_in_ROB);
  RWConnect(csu.rs2_in_ROB_value, rs.rs2_in_ROB_value);
  RWConnect(csu.decoded_imm, rs.decoded_imm);
  RWConnect(csu.decoded_shamt, rs.decoded_shamt);
  RWConnect(csu.cache_hit, rs.cache_hit);
  RWConnect(csu.cache_hit_ROB_index, rs.cache_hit_ROB_index);
  RWConnect(csu.cache_hit_data, rs.cache_hit_data);
  RWConnect(rs.RS_remain_space_output, csu.reservestation_emptyspace_receiver);
  // memory <-> lsq
  RWConnect(memory.data_sign, lsq.mem_data_sign);
  RWConnect(memory.completed_memins_ROB_index, lsq.completed_memins_ROB_index);
  RWConnect(memory.completed_memins_read_data, lsq.completed_memins_read_data);
  RWConnect(lsq.request_type_output, memory.request_type_input);
  RWConnect(lsq.request_ROB_index, memory.request_ROB_index);
  RWConnect(lsq.request_address_output, memory.address_input);
  RWConnect(lsq.request_data_output, memory.data_input);
  // memory <-> alu : no connections
  // memory <-> register file : no connections
  // memory <-> reserve station :
  RWConnect(memory.data_sign, rs.mem_status_receiver);
  RWConnect(memory.completed_memins_ROB_index, rs.completed_memins_ROB_index);
  RWConnect(memory.completed_memins_read_data, rs.completed_memins_read_data);
  // lsq <-> alu :
  RWConnect(alu.alu_status, lsq.alu_status_receiver);
  RWConnect(alu.result_ROB_index, lsq.completed_aluins_ROB_index);
  RWConnect(alu.result, lsq.completed_aluins_result);
  // lsq <-> register file
  RWConnect(rf.rs1_nodep, lsq.rs1_nodep);
  RWConnect(rf.rs1_deps, lsq.rs1_deps);
  RWConnect(rf.rs1_value, lsq.rs1_value);
  RWConnect(rf.rs2_nodep, lsq.rs2_nodep);
  RWConnect(rf.rs2_deps, lsq.rs2_deps);
  RWConnect(rf.rs2_value, lsq.rs2_value);
  // lsq <-> reserve station : no connections
  // alu <-> register file : no connections
  // alu <-> reserve station
  RWConnect(rs.request_full_id, alu.request_full_id);
  RWConnect(rs.operand1, alu.operand1);
  RWConnect(rs.operand2, alu.operand2);
  RWConnect(rs.request_ROB_index, alu.request_ROB_index);
  RWConnect(rs.alu_ins_PC, alu.request_PC);
  RWConnect(alu.alu_status, rs.alu_status_receiver);
  RWConnect(alu.result_ROB_index, rs.completed_aluins_ROB_index);
  RWConnect(alu.result, rs.completed_aluins_result);
  // register file <-> reserve station
  RWConnect(rf.rs1_nodep, rs.rs1_nodep);
  RWConnect(rf.rs1_deps, rs.rs1_deps);
  RWConnect(rf.rs1_value, rs.rs1_value);
  RWConnect(rf.rs2_nodep, rs.rs2_nodep);
  RWConnect(rf.rs2_deps, rs.rs2_deps);
  RWConnect(rf.rs2_value, rs.rs2_value);
  // now start running
  std::cout << cpu.run(0, true) << std::endl;
  return 0;
}