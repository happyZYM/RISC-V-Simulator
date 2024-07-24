#pragma once
#include <concepts>
#include <tuple>

namespace dark::reflect {

/* A init helper to get the size of a struct. */
struct init_helper {
	template<typename _Tp>
	operator _Tp();
};

/* A size helper to get the size of a struct. */
template<typename _Tp>
	requires std::is_aggregate_v<_Tp>
inline consteval auto member_size_aux(auto &&...args) -> std::size_t {
	constexpr std::size_t size = sizeof...(args);
	constexpr std::size_t maximum = 114;
	if constexpr (size > maximum) {
		static_assert(sizeof(_Tp) == 0, "The struct has too many members.");
	}
	else if constexpr (!requires { _Tp{args...}; }) {
		return size - 1;
	}
	else {
		return member_size_aux<_Tp>(args..., init_helper{});
	}
}

/* Return the member size for a aggregate type without base.  */
template<typename _Tp>
	requires std::is_aggregate_v<_Tp>
inline consteval auto member_size(_Tp &) -> std::size_t { return member_size_aux<_Tp>(); }

template<typename _Tp>
	requires std::is_aggregate_v<_Tp>
inline consteval auto member_size() -> std::size_t { return member_size_aux<_Tp>(); }

template<typename _Tp>
	requires std::is_aggregate_v<_Tp>
auto tuplify(_Tp &value) {
	constexpr auto size = member_size<_Tp>();
	if constexpr (size == 1) {
		auto &[x0] = value;
		return std::forward_as_tuple(x0);
	}
	else if constexpr (size == 2) {
		auto &[x0, x1] = value;
		return std::forward_as_tuple(x0, x1);
	}
	else if constexpr (size == 3) {
		auto &[x0, x1, x2] = value;
		return std::forward_as_tuple(x0, x1, x2);
	}
	else if constexpr (size == 4) {
		auto &[x0, x1, x2, x3] = value;
		return std::forward_as_tuple(x0, x1, x2, x3);
	}
	else if constexpr (size == 5) {
		auto &[x0, x1, x2, x3, x4] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4);
	}
	else if constexpr (size == 6) {
		auto &[x0, x1, x2, x3, x4, x5] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5);
	}
	else if constexpr (size == 7) {
		auto &[x0, x1, x2, x3, x4, x5, x6] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5, x6);
	}
	else if constexpr (size == 8) {
		auto &[x0, x1, x2, x3, x4, x5, x6, x7] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5, x6, x7);
	}
	else if constexpr (size == 9) {
		auto &[x0, x1, x2, x3, x4, x5, x6, x7, x8] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5, x6, x7, x8);
	}
	else if constexpr (size == 10) {
		auto &[x0, x1, x2, x3, x4, x5, x6, x7, x8, x9] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);
	}
	else if constexpr (size == 11) {
		auto &[x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10);
	}
	else if constexpr (size == 12) {
		auto &[x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11);
	}
	else if constexpr (size == 13) {
		auto &[x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12);
	}
	else if constexpr (size == 14) {
		auto &[x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13] = value;
		return std::forward_as_tuple(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13);
	}
	else {
		static_assert(sizeof(_Tp) == 0, "The struct has too many members.");
	}
}

} // namespace dark::reflect
