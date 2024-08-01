#pragma once
#include "concept.h"
#include "debug.h"
#include <memory>

namespace dark {

namespace details {

	template<typename _Fn, std::size_t _Len>
	concept WireFunction =
			concepts::bit_convertible<std::decay_t<std::invoke_result_t<_Fn>>, _Len>;

	struct FuncBase {
		using _Ret_t = max_size_t;
		using _Cpy_t = FuncBase *;
		virtual _Ret_t call() const = 0;
		virtual _Cpy_t copy() const = 0;
		virtual ~FuncBase() = default;
	};

	template<std::size_t _Len, WireFunction<_Len> _Fn>
	struct FuncImpl final : FuncBase {
		_Fn _M_lambda;

		template<typename _Tp>
		FuncImpl(_Tp &&fn) : _M_lambda(std::forward<_Tp>(fn)) {}

		_Ret_t call() const override { return static_cast<_Ret_t>(this->_M_lambda()); }
		_Cpy_t copy() const override { return new FuncImpl(*this); }
	};

	struct EmptyWire final : FuncBase {
		_Ret_t call() const override {
			debug::assert(false, "Empty wire is called.");
			debug::unreachable();
		}
		_Cpy_t copy() const override { return new EmptyWire; }
	};

} // namespace details


template<std::size_t _Len>
struct Wire {
private:
	static_assert(0 < _Len && _Len <= kMaxLength,
				  "Wire: _Len must be in range [1, kMaxLength].");

	friend class Visitor;

	using _Manage_t = std::unique_ptr<details::FuncBase>;

	_Manage_t _M_func;

	mutable max_size_t _M_cache : _Len;
	mutable bool _M_holds;

	[[no_unique_address]]
	debug::DebugValue<bool, false> _M_assigned;

private:
	void sync() { this->_M_holds = false; }
  friend class CPU;

	template<details::WireFunction<_Len> _Fn>
	static auto _M_new_func(_Fn &&fn) {
		using _Decay_t = std::decay_t<_Fn>;
		return new details::FuncImpl<_Len, _Decay_t>{std::forward<_Fn>(fn)};
	}

	void _M_checked_assign() {
		debug::assert(!this->_M_assigned, "Wire is assigned twice.");
		this->_M_assigned = true;
	}

public:
	static constexpr std::size_t _Bit_Len = _Len;

	Wire() : _M_func(new details::EmptyWire),
			 _M_cache(), _M_holds(), _M_assigned() {}

	explicit operator max_size_t() const {
    #ifdef _DEBUG
    debug::assert(this->_M_assigned, "Wire is not assigned.");
    #endif
		if (this->_M_holds == false) {
			this->_M_holds = true;
			this->_M_cache = this->_M_func->call();
		}
		return this->_M_cache;
	}

	Wire(Wire &&) = delete;
	Wire(const Wire &) = delete;
	Wire &operator=(Wire &&) = delete;
	Wire &operator=(const Wire &rhs) = delete;

	template<details::WireFunction<_Len> _Fn>
	Wire(_Fn &&fn) : _M_func(_M_new_func(std::forward<_Fn>(fn))),
					 _M_cache(), _M_holds(), _M_assigned() {}

	template<details::WireFunction<_Len> _Fn>
	Wire &operator=(_Fn &&fn) {
		return this->assign(std::forward<_Fn>(fn)), *this;
	}

	template<details::WireFunction<_Len> _Fn>
	void assign(_Fn &&fn) {
		this->_M_checked_assign();
		this->_M_func.reset(_M_new_func(std::forward<_Fn>(fn)));
		this->sync();
	}

	explicit operator bool() const {
		return static_cast<max_size_t>(*this);
	}
};


} // namespace dark
