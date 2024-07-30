#pragma once
#include "bit.h"
#include "register.h"

namespace dark {

using dark::concepts::bit_match;
using dark::concepts::bit_type;
using dark::concepts::int_type;

template<typename _Tp>
constexpr auto cast(const _Tp &value) {
	return static_cast<max_size_t>(value);
}

template<typename _Tp, typename _Up>
consteval auto get_common_length() -> std::size_t {
	static_assert(bit_match<_Tp, _Up>);
	if constexpr (bit_type<_Tp>) {
		return _Tp::_Bit_Len;
	}
	else {
		static_assert(bit_type<_Up>, "Invalid common length");
		return _Up::_Bit_Len;
	}
}


template<typename _Tp, typename _Up>
	requires bit_match<_Tp, _Up>
constexpr auto operator+(const _Tp &lhs, const _Up &rhs) {
	constexpr auto _Len = get_common_length<_Tp, _Up>();
	return Bit<_Len>(cast(lhs) + cast(rhs));
}

template<typename _Tp, typename _Up>
	requires bit_match<_Tp, _Up>
constexpr auto operator-(const _Tp &lhs, const _Up &rhs) {
	constexpr auto _Len = get_common_length<_Tp, _Up>();
	return Bit<_Len>(cast(lhs) - cast(rhs));
}

template<typename _Tp, typename _Up>
	requires bit_match<_Tp, _Up>
constexpr auto operator*(const _Tp &lhs, const _Up &rhs) {
	constexpr auto _Len = get_common_length<_Tp, _Up>();
	return Bit<_Len>(cast(lhs) * cast(rhs));
}

template<typename _Tp, typename _Up>
	requires bit_match<_Tp, _Up>
constexpr auto operator/(const _Tp &lhs, const _Up &rhs) {
	constexpr auto _Len = get_common_length<_Tp, _Up>();
	return Bit<_Len>(cast(lhs) / cast(rhs));
}

template<typename _Tp, typename _Up>
	requires bit_match<_Tp, _Up>
constexpr auto operator&(const _Tp &lhs, const _Up &rhs) {
	constexpr auto _Len = get_common_length<_Tp, _Up>();
	return Bit<_Len>(cast(lhs) & cast(rhs));
}

template<typename _Tp, typename _Up>
	requires bit_match<_Tp, _Up>
constexpr auto operator|(const _Tp &lhs, const _Up &rhs) {
	constexpr auto _Len = get_common_length<_Tp, _Up>();
	return Bit<_Len>(cast(lhs) | cast(rhs));
}

template<typename _Tp, typename _Up>
	requires bit_match<_Tp, _Up>
constexpr auto operator^(const _Tp &lhs, const _Up &rhs) {
	constexpr auto _Len = get_common_length<_Tp, _Up>();
	return Bit<_Len>(cast(lhs) ^ cast(rhs));
}

template<typename _Tp>
concept int_or_bit = int_type<_Tp> || bit_type<std::decay_t<_Tp>>;

template<bit_type _Tp, int_or_bit _Up>
constexpr auto operator<<(const _Tp &lhs, const _Up &rhs) {
	return Bit<_Tp::_Bit_Len>(cast(lhs) << (cast(rhs) & (kMaxLength - 1)));
}

template<bit_type _Tp, int_or_bit _Up>
constexpr auto operator>>(const _Tp &lhs, const _Up &rhs) {
	return Bit<_Tp::_Bit_Len>(cast(lhs) >> (cast(rhs) & (kMaxLength - 1)));
}

template<bit_type _Tp>
constexpr auto operator~(const _Tp &value) {
	return Bit<_Tp::_Bit_Len>(~cast(value));
}

template<bit_type _Tp>
constexpr auto operator!(const _Tp &value) {
	return ~value;
}

template<bit_type _Tp>
constexpr auto operator+(const _Tp &value) {
	return Bit<_Tp::_Bit_Len>(+cast(value));
}

template<bit_type _Tp>
constexpr auto operator-(const _Tp &value) {
	return Bit<_Tp::_Bit_Len>(-cast(value));
}

template<int_or_bit _Tp, int_or_bit _Up>
constexpr bool operator==(const _Tp &lhs, const _Up &rhs) {
	return cast(lhs) == cast(rhs);
}

template <typename _Tp>
inline constexpr bool is_reg_ref_v = false;

template <std::size_t _Len>
inline constexpr bool is_reg_ref_v<Register<_Len> &> = true;

template<int_or_bit _Tp, int_or_bit _Up>
constexpr auto operator <=> (const _Tp &lhs, const _Up &rhs) {
	return cast(lhs) <=> cast(rhs);
}

} // namespace dark
