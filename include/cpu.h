#pragma once
#include "module.h"
#include <algorithm>
#include <memory>
#include <random>
#include <vector>

namespace dark {

class CPU {
private:
	std::vector<std::unique_ptr<ModuleBase>> mod_owned;
	std::vector<ModuleBase *> modules;

public:
	unsigned long long cycles = 0;

private:
	void sync_all() {
		for (auto &module: modules)
			module->sync();
	}

public:
	/// @attention the pointer will be moved. you SHOULD NOT use it after calling this function.
	template<typename _Tp>
		requires std::derived_from<_Tp, ModuleBase>
	void add_module(std::unique_ptr<_Tp> &module) {
		modules.push_back(module.get());
		mod_owned.emplace_back(std::move(module));
	}
	void add_module(std::unique_ptr<ModuleBase> module) {
		modules.push_back(module.get());
		mod_owned.emplace_back(std::move(module));
	}
	void add_module(ModuleBase *module) {
		modules.push_back(module);
	}

	void run_once() {
		++cycles;
		for (auto &module: modules)
			module->work();
		sync_all();
	}
	void run_once_shuffle() {
		static std::default_random_engine engine;
		std::vector<ModuleBase *> shuffled = modules;
		std::shuffle(shuffled.begin(), shuffled.end(), engine);

		++cycles;
		for (auto &module: shuffled)
			module->work();
		sync_all();
	}
	void run(unsigned long long max_cycles = 0, bool shuffle = false) {
		auto func = shuffle ? &CPU::run_once_shuffle : &CPU::run_once;
		while (max_cycles == 0 || cycles < max_cycles)
			(this->*func)();
	}
};

} // namespace dark
