// compat_endian.h
#pragma once

#if defined(__linux__)
  #include <compat/compat_endian.h>
#elif defined(__APPLE__)
  #include <machine/endian.h>
  #include <libkern/OSByteOrder.h>

  // Define GNU-style aliases if needed
  #define __BYTE_ORDER    BYTE_ORDER
  #define __BIG_ENDIAN    BIG_ENDIAN
  #define __LITTLE_ENDIAN LITTLE_ENDIAN
  #define __PDP_ENDIAN    PDP_ENDIAN

  // Byte-swap helpers (map to macOS functions)
  #define bswap_16(x) OSSwapInt16(x)
  #define bswap_32(x) OSSwapInt32(x)
  #define bswap_64(x) OSSwapInt64(x)
#else
  #error "Unsupported platform: need endian definitions"
#endif
