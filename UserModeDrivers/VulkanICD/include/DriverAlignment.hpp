#pragma once

#include <new>

namespace vk {

static inline constexpr ::std::size_t DriverVkAlignmentSz = sizeof(void*);
static inline constexpr ::std::align_val_t DriverVkAlignment = ::std::align_val_t { DriverVkAlignmentSz };

}
