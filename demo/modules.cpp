#include "tools.h"
#include <iostream>

struct RegFile_Input {
	Wire <5> rs1_index;		// Read
	Wire <5> rs2_index;		// Read

	Wire <5> wb_index;		// Writeback
	Wire <1> wb_enable;		// Writeback enabled?
	Wire <32> wb_data;		// Data to writeback
};

struct RegFile_Output {
	Register <32> rs1_data;		// Read
	Register <32> rs2_data;		// Read
};

struct RegFile_Private {
	std::array <Register<32>, 32> regs;
};

struct RegFile : dark::Module <RegFile_Input, RegFile_Output, RegFile_Private> {
	void work() override final {
		rs1_data <= regs[to_unsigned(rs1_index)];
		rs2_data <= regs[to_unsigned(rs2_index)];

		if (wb_enable && wb_index != 0) {
			regs[to_unsigned(wb_index)] <= wb_data;
		}

	}
};

struct InsDecode_Input {
	Wire <32> rs1_data;
	Wire <32> rs2_data;
};

struct InsDecode_Output {
	Register <5> rs1_index;
	Register <5> rs2_index;
	Register <5> wb_index;
	Register <32> wb_data;
	Register <1> wb_enable;
};

struct InsDecode : dark::Module <InsDecode_Input, InsDecode_Output> {
	void work() override final {
		char c;
		max_size_t x;
		max_size_t y;
		std::cin >> c >> x >> y;
		if (c == 'r') {
			rs1_index <= x;
			rs2_index <= y;
			wb_index <= 0;
			wb_data <= 0;
			wb_enable <= 0;
		} else {
			rs1_index <= 0;
			rs2_index <= 0;
			wb_index <= x;
			wb_data <= y;
			wb_enable <= 1;
		}

		std::cout << "rs1_data: " << to_unsigned(rs1_data) << std::endl;
		std::cout << "rs2_data: " << to_unsigned(rs2_data) << std::endl;
	}
};

signed main() {
	InsDecode ins_decode;
	RegFile reg_file;

	dark::CPU cpu;

	cpu.add_module(&ins_decode);
	cpu.add_module(&reg_file);

	reg_file.rs1_index = [&]() -> auto & { return ins_decode.rs1_index; };
	reg_file.rs2_index = [&]() -> auto & { return ins_decode.rs2_index; };
	reg_file.wb_index = [&]() -> auto & { return ins_decode.wb_index; };
	reg_file.wb_enable = [&]() -> auto & { return ins_decode.wb_enable; };
	reg_file.wb_data = [&]() -> auto & { return ins_decode.wb_data; };

	ins_decode.rs1_data = [&]() -> auto & { return reg_file.rs1_data; };
	ins_decode.rs2_data = [&]() -> auto & { return reg_file.rs2_data; };

	cpu.run(114514, true);

	// Demo input:
	// w 1 2	(output 0 0)
	// r 1 2	(output 0 0)
	// w 2 3	(output 0 0)
	// r 1 2	(output 2 0)
	// r 1 2	(output 0 0)
	// r 1 2	(output 2 3)

	return 0;
}
