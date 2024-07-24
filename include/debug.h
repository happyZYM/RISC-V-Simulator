#pragma once
#include <utility>
#ifdef _DEBUG
#include <iostream>
#endif
#include <source_location>

namespace dark::debug {

/**
 * Copied from https://en.cppreference.com/w/cpp/utility/unreachable
 * If compiled under C++23, just use std::unreachable() instead.
*/
[[noreturn]] inline void unreachable() {
#if __cplusplus > 202002L
	std::unreachable();
#elif defined(_MSC_VER) && !defined(__clang__) // MSVC
	// Uses compiler specific extensions if possible.
	// Even if no extension is used, undefined behavior is still raised by
	// an empty function body and the noreturn attribute.
	__assume(false);
#else                                          // GCC, Clang
	__builtin_unreachable();
#endif
}

template<typename _Tp, typename... _Args>
struct assert {
#ifdef _DEBUG
	explicit assert(_Tp &&condition, _Args &&...args,
					std::source_location location = std::source_location::current()) {
		if (condition) return;
		std::cerr << "Assertion failed at: "
				  << location.file_name() << ":" << location.line() << "\n";
		if constexpr (sizeof...(args) != 0) {
			std::cerr << "Message: ";
			((std::cerr << args), ...) << std::endl;
		}
		std::exit(EXIT_FAILURE);
	}
#else
	explicit assert(_Tp &&, _Args &&...) {}
#endif
};

template<typename _Tp, typename... _Args>
assert(_Tp &&, _Args &&...) -> assert<_Tp, _Args...>;

template<typename _Tp, _Tp _Default>
struct DebugValue {
#ifdef _DEBUG
private:
	_Tp _M_value = _Default;

public:
	auto get_value() const { return this->_M_value; }
	auto set_value(_Tp value) { this->_M_value = value; }
#else
public:
	auto get_value() const { return _Default; }
	auto set_value(_Tp) { /* do nothing */ }
#endif
public:
	explicit operator _Tp() const { return this->get_value(); }
	DebugValue &operator=(_Tp value) {
		this->set_value(value);
		return *this;
	}
};

} // namespace dark::debug
