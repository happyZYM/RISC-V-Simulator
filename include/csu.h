#pragma once
#ifndef CSU_H
#include "tools.h"
namespace ZYM {
const int kROBSize = 32;
const int kTotalRegister = 32;
struct CentralScheduleUnit_Input {
  dark::Wire<1> reset;
};
struct CentralScheduleUnit_Output {
  dark::Register<1> ready;
  dark::Register<1> force_clear_announcer;
};
struct CentralScheduleUnit_Private {
  dark::Register<32> predicated_PC;
};
struct CentralScheduleUnit
    : public dark::Module<CentralScheduleUnit_Input, CentralScheduleUnit_Output, CentralScheduleUnit_Private> {
 private:
 public:
  CentralScheduleUnit() { ; }
};
}  // namespace ZYM
#endif