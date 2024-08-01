#pragma once
#include "concept.h"
#include "module.h"
#include "wire.h"
#include <algorithm>
#include <memory>
#include <random>
#include <vector>
extern unsigned int global_clock;

namespace dark {

class CPU {
private:
	std::vector<std::unique_ptr<ModuleBase>> mod_owned;
	std::vector<ModuleBase *> modules;
  bool reset_signal = false;

public:
	unsigned long long cycles = 0;
  dark::Wire<9> halt_signal;

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
  bool GetResetSignal(){
    return reset_signal;
  }
	uint8_t run(unsigned long long max_cycles = 0, bool shuffle = false) {
		auto func = shuffle ? &CPU::run_once_shuffle : &CPU::run_once;
    reset_signal=true;
		while (max_cycles == 0 || cycles < max_cycles) {
      DEBUG_CERR<<"\nclock: "<<std::dec<<global_clock<<std::endl;
			(this->*func)();
      reset_signal=false;
      halt_signal.sync();
      uint32_t halt_signal_value = static_cast<max_size_t>(halt_signal);
      DEBUG_CERR<<"simulator received halt_signal_value="<<std::dec<<halt_signal_value<<std::endl;
      if(halt_signal_value &(1<<8)) {
        return halt_signal_value&0xff;
      }
      global_clock++;
    }
    return 255;
	}
};

} // namespace dark
