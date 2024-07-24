#pragma once
#include "bit.h"

namespace dark {

template<int _First>
constexpr auto int_concat(max_size_t arg) { return arg; }

template<int _First, int... _Lens>
constexpr auto int_concat(max_size_t arg, auto... args) {
	return (arg << (_Lens + ...)) | int_concat<_Lens...>(args...);
}

template<std::size_t _Old, std::size_t _New = kMaxLength>
constexpr auto sign_extend(max_size_t val) {
	static_assert(_Old <= _New, "sign_extend: _Old should be less than _New");
	struct {
		max_ssize_t _M_data : _Old;
	} tmp;
	return Bit<_New>(tmp._M_data = val);
}

template<std::size_t _New = kMaxLength, concepts::bit_type _Tp>
constexpr auto sign_extend(const _Tp &val) {
	return sign_extend<_Tp::_Bit_Len, _New>(static_cast<max_size_t>(val));
}

template<std::size_t _Old, std::size_t _New = kMaxLength>
constexpr auto zero_extend(max_size_t val) {
	static_assert(_Old < _New, "zero_extend: _Old should be less than _New");
	struct {
		max_size_t _M_data : _Old;
	} tmp;
	return Bit<_New>(tmp._M_data = val);
}

template<std::size_t _New = kMaxLength, concepts::bit_type _Tp>
constexpr auto zero_extend(const _Tp &val) {
	return zero_extend<_Tp::_Bit_Len, _New>(static_cast<max_size_t>(val));
}

template<std::size_t _Nm>
template<concepts::bit_type... _Tp>
	requires((_Tp::_Bit_Len + ...) == _Nm)
constexpr Bit<_Nm>::Bit(const _Tp &...args)
	: _M_data(int_concat<_Tp::_Bit_Len...>(static_cast<max_size_t>(args)...)) {}

template<std::size_t _Nm>
template<concepts::bit_convertible<_Nm> _Tp>
constexpr Bit<_Nm> &Bit<_Nm>::operator=(const _Tp &val) {
	this->_M_data = static_cast<max_size_t>(val);
	return *this;
}

template<std::size_t _Nm>
template<std::size_t _Hi, std::size_t _Lo>
constexpr void Bit<_Nm>::_M_range_check() {
	static_assert(_Lo <= _Hi, "Bit::range_check: _Lo should be no greater than _Hi");
	static_assert(_Hi < _Nm, "Bit::range_check: _Hi should be less than _Nm");
}

template<std::size_t _Nm>
template<std::size_t _Hi, std::size_t _Lo, concepts::bit_convertible<_Nm> _Tp>
constexpr void Bit<_Nm>::set(const _Tp &val) {
	this->_M_range_check<_Hi, _Lo>();
	auto data = static_cast<max_size_t>(val);
	constexpr auto _Length = _Hi - _Lo + 1;
	auto mask = make_mask<_Length> << _Lo;
	this->_M_data = (this->_M_data & ~mask) | ((data << _Lo) & mask);
}

template<std::size_t _Nm>
template<std::size_t _Hi, std::size_t _Lo>
constexpr auto Bit<_Nm>::range() const -> Bit<_Hi - _Lo + 1> {
	this->_M_range_check<_Hi, _Lo>();
	constexpr auto _Length = _Hi - _Lo + 1;
	return Bit<_Length>(this->_M_data >> _Lo);
}

template<std::size_t _Nm>
template<std::size_t _Len>
constexpr auto Bit<_Nm>::slice(std::size_t pos) const -> Bit<_Len> {
	static_assert(_Len <= _Nm, "Bit::slice: _Len should be no greater than _Nm");
	debug::assert(pos <= _Nm - _Len, "Bit::slice: pos should be less than _Nm - _Len");
	return Bit<_Len>(this->_M_data >> pos);
}


} // namespace dark
