#pragma once
#include "synchronize.h"
namespace dark {

namespace details {
	struct empty_class {
		void sync() { /* do nothing */ }
	};
} // namespace details

struct ModuleBase {
	virtual void work() = 0;
	virtual void sync() = 0;
	virtual ~ModuleBase() = default;
};

template<typename _Tinput, typename _Toutput, typename _Tprivate = details::empty_class>
	requires std::is_aggregate_v<_Tinput> && std::is_aggregate_v<_Toutput> && std::is_aggregate_v<_Tprivate>
struct Module : public ModuleBase, public _Tinput, public _Toutput, protected _Tprivate {
	void sync() override final {
		sync_member(static_cast<_Tinput &>(*this));
		sync_member(static_cast<_Toutput &>(*this));
		sync_member(static_cast<_Tprivate &>(*this));
	}
};

} // namespace dark
