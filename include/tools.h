#pragma once
#include "bit.h"
#include "bit_impl.h"
#include "operator.h"
#include "register.h"
#include "synchronize.h"
#include "wire.h"
#include "module.h"
#include "cpu.h"
#include "memory.h"
#include "bus.h"

using dark::Bit;
using dark::sign_extend;
using dark::zero_extend;

using dark::Register;
using dark::Wire;

using dark::sync_member;
using dark::SyncTags;
using dark::Visitor;

using dark::max_size_t;
using dark::max_ssize_t;

template<dark::concepts::bit_type _Tp>
constexpr auto to_unsigned(const _Tp &x) {
	return static_cast<max_size_t>(x);
}

template<dark::concepts::bit_type _Tp>
constexpr auto to_signed(const _Tp &x) {
	return static_cast<max_ssize_t>(static_cast<max_size_t>(sign_extend(x)));
}
