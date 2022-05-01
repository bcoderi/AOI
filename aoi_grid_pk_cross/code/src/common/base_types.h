#pragma once

#include <stdint.h>

typedef unsigned int Uint32;
typedef unsigned long long Uint64;
typedef Uint64 Nuid;

typedef int16_t Int16;

#define AOI_CLASS_ADD_FLAG(flag_name, flag_idx, flags_name)   \
  inline void SetFlag_##flag_name() {flags_name |= 1 << flag_idx;}   \
  inline void UnsetFlag_##flag_name() {flags_name &= ~(1 << flag_idx);}    \
  inline bool GetFlag_##flag_name() const {return flags & 1 << flag_idx;}


