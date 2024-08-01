#include "tools.h"
#include <iostream>
#include <unordered_map>
unsigned int global_clock=0;
// RISC-V
enum class Opcode : dark::max_size_t {
	ADD,
	SUB,
	SLL,
	SRL,
	SRA,
	AND,
	OR,
	XOR,
	SLT,
	SLTU,
	SGE,
	SGEU,
	SEQ,
	SNEQ
};

// Normally, only wire can be used in the input.
struct AluInput {
	Wire<8> opcode;
	Wire<1> issue;
	Wire<32> rs1;
	Wire<32> rs2;
};

struct AluOutput {
	Register<32> out;
	Register<1> done;
};

struct AluModule : dark::Module<AluInput, AluOutput> {
	void work() override {
		if (issue) {
			switch (static_cast<Opcode>(static_cast<unsigned>(opcode))) {
				using enum Opcode;
				case ADD: out <= (rs1 + rs2); break;
				case SUB: out <= (rs1 - rs2); break;
				case SLL: out <= (rs1 << rs2); break;
				case SRL: out <= (rs1 >> rs2); break;
				case SRA: out <= (to_signed(rs1) >> to_unsigned(rs2));
				case AND: out <= (rs1 & rs2); break;
				case OR: out <= (rs1 | rs2); break;
				case XOR: out <= (rs1 ^ rs2); break;
				case SLT: out <= (to_signed(rs1) < to_signed(rs2)); break;
				case SLTU: out <= (rs1 < rs2); break;
				case SGE: out <= (to_signed(rs1) >= to_signed(rs2)); break;
				case SGEU: out <= (rs1 >= rs2); break;
				case SEQ: out <= (rs1 == rs2); break;
				case SNEQ: out <= (rs1 != rs2); break;
				default: dark::debug::assert(false, "Invalid opcode");
			}
			done <= 1;
		}
		else {
			done <= 0;
		}
	}
};

int main() {
	std::string opstring;

	max_size_t opcode;
	max_size_t issue;
	max_size_t rs1;
	max_size_t rs2;

	dark::CPU cpu;
	AluModule alu;
	alu.opcode = [&]() { return opcode; };
	alu.issue = [&]() { return issue; };
	alu.rs1 = [&]() { return rs1; };
	alu.rs2 = [&]() { return rs2; };
	cpu.add_module(&alu);

	std::unordered_map<std::string, Opcode> cmd2op = {
			{"add", Opcode::ADD},
			{"sub", Opcode::SUB},
			{"sll", Opcode::SLL},
			{"src", Opcode::SRL},
			{"sra", Opcode::SRA},
			{"and", Opcode::AND},
			{"or", Opcode::OR},
			{"xor", Opcode::XOR},
			{"slt", Opcode::SLT},
			{"sltu", Opcode::SLTU},
			{"sge", Opcode::SGE},
			{"sgeu", Opcode::SGEU},
			{"seq", Opcode::SEQ},
			{"sneq", Opcode::SNEQ}};
	while (std::cin >> opstring) {
		if (cmd2op.find(opstring) == cmd2op.end()) {
			std::cout << "Invalid opcode" << std::endl;
			issue = 0;
		}
		else {
			issue = 1;
			std::cin >> rs1 >> rs2;
		}
		opcode = static_cast<max_size_t>(cmd2op[opstring]);
		cpu.run_once();
		std::cout << "out: " << static_cast<unsigned int>(alu.out) << std::endl;
		std::cout << "done: " << static_cast<unsigned int>(alu.done) << std::endl;
	}
	return 0;
}