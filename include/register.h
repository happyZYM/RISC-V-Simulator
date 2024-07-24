#pragma once
#include "concept.h"
#include "debug.h"

namespace dark {

template<std::size_t _Len>
struct Register {
private:
	static_assert(0 < _Len && _Len <= kMaxLength,
				  "Register: _Len must be in range [1, kMaxLength].");

	friend class Visitor;

	max_size_t _M_old : _Len;
	max_size_t _M_new : _Len;

	[[no_unique_address]]
	debug::DebugValue<bool, false> _M_assigned;

	void sync() {
		this->_M_assigned = false;
		this->_M_old = this->_M_new;
	}

public:
	static constexpr std::size_t _Bit_Len = _Len;

	Register() : _M_old(), _M_new(), _M_assigned() {}

	Register(Register &&) = delete;
	Register(const Register &) = delete;
	Register &operator=(Register &&) = delete;
	Register &operator=(const Register &rhs) = delete;

	template<concepts::bit_convertible<_Len> _Tp>
	void operator<=(const _Tp &value) {
		debug::assert(!this->_M_assigned, "Register is double assigned in this cycle.");
		this->_M_assigned = true;
		this->_M_new = static_cast<max_size_t>(value);
	}

	explicit operator max_size_t() const { return this->_M_old; }
	explicit operator bool() const { return this->_M_old; }
};

} // namespace dark
