#pragma once
#include "concept.h"
#include "debug.h"
#include <climits>
#include <concepts>
#include <version>

namespace dark {

template<std::size_t _Nm>
struct Bit {
private:
	static_assert(0 < _Nm && _Nm <= kMaxLength,
				  "Bit: _Nm out of range. Should be in [1, kMaxLength]");

	max_size_t _M_data : _Nm; // Real storage

	template<std::size_t _Hi, std::size_t _Lo>
	static constexpr void _M_range_check();

public:
	static constexpr std::size_t _Bit_Len = _Nm;

	constexpr Bit(max_size_t data = 0) : _M_data(data) {}

	constexpr explicit operator max_size_t() const { return this->_M_data; }

	template<concepts::bit_type... _Tp>
		requires((_Tp::_Bit_Len + ...) == _Nm)
	constexpr Bit(const _Tp &...args);

	template<concepts::bit_convertible<_Nm> _Tp>
	constexpr Bit &operator=(const _Tp &val);

	template<std::size_t _Hi, std::size_t _Lo = _Hi, concepts::bit_convertible<_Nm> _Tp>
	constexpr void set(const _Tp &val);

	template<std::size_t _Hi, std::size_t _Lo = _Hi>
	constexpr auto range() const -> Bit<_Hi - _Lo + 1>;

	template<std::size_t _Len = 1>
	constexpr auto slice(std::size_t pos) const -> Bit<_Len>;

	constexpr Bit<1> operator[](std::size_t pos) const { return this->slice(pos); }
};

template<concepts::bit_type... _Tp>
Bit(_Tp...) -> Bit<(_Tp::_Bit_Len + ...)>;

} // namespace dark
