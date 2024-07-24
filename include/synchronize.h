#pragma once
#include "reflect.h"
#include <array>

namespace dark {

struct Visitor {
	template<typename _Tp>
	static constexpr bool is_syncable_v =
			requires(_Tp &val) { { val.sync() } -> std::same_as<void>; };

	template<typename _Tp>
		requires is_syncable_v<_Tp>
	static void sync(_Tp &val) { val.sync(); }

	template<typename _Tp, typename _Base>
	static _Base &cast(_Tp &value) { return static_cast<_Base &>(value); }
};

template<typename... _Base>
struct SyncTags {};

template<typename _Tp>
static constexpr bool is_valid_tag_v = false;
template<typename... _Base>
static constexpr bool is_valid_tag_v<SyncTags<_Base...>> = true;
template<typename _Tp>
concept has_valid_tag = is_valid_tag_v<typename _Tp::Tags>;

template<typename _Tp>
static constexpr bool is_std_array_v = std::is_array_v<_Tp>;
template<typename _Tp, std::size_t _Nm>
static constexpr bool is_std_array_v<std::array<_Tp, _Nm>> = true;

template<typename _Tp>
inline void sync_member(_Tp &value);

template<typename _Tp, typename... _Base>
inline void sync_by_tag(_Tp &value, SyncTags<_Base...>) {
	(sync_member(Visitor::cast<_Tp, _Base>(value)), ...);
}

template<typename _Tp>
inline void sync_member(_Tp &value) {
	if constexpr (std::is_const_v<_Tp>) {
		/* Do nothing! Constant members need no synchronization! */
	}
	else if constexpr (is_std_array_v<_Tp>) {
		for (auto &member: value) sync_member(member);
	}
	else if constexpr (Visitor::is_syncable_v<_Tp>) {
		Visitor::sync(value);
	}
	else if constexpr (has_valid_tag<_Tp>) {
		sync_by_tag(value, typename _Tp::Tags{});
	}
	else if constexpr (std::is_aggregate_v<_Tp>) {
		auto &&tuple = reflect::tuplify(value);
		std::apply([](auto &...members) { (sync_member(members), ...); }, tuple);
	}
	else {
		static_assert(sizeof(_Tp) == 0, "This type is not syncable.");
	}
}

} // namespace dark
