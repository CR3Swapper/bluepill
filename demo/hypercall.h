#pragma once
#include <Windows.h>
#include <intrin.h>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using u128 = __m128;

using s8 = char;
using s16 = short;
using s32 = int;
using s64 = long long;

namespace bluepill
{
	constexpr auto key = 0xC0FFEE;
	extern "C" u64 hypercall(u64 key);
}