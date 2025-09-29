#pragma once

#include <stdint.h>

namespace sxi
{
	using u8  = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using i8  = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;
}

#define SXI_TO_SIZE(x)  static_cast<size_t>(x)
#define SXI_TO_U8(x)    static_cast<sxi::u8>(x)
#define SXI_TO_U16(x)   static_cast<sxi::u16>(x)
#define SXI_TO_U32(x)   static_cast<sxi::u32>(x)
#define SXI_TO_U64(x)   static_cast<sxi::u64>(x)
#define SXI_TO_I8(x)    static_cast<sxi::i8>(x)
#define SXI_TO_I16(x)   static_cast<sxi::i16>(x)
#define SXI_TO_I32(x)   static_cast<sxi::i32>(x)
#define SXI_TO_I64(x)   static_cast<sxi::i64>(x)