#pragma once

#if _STL_COMPILER_PREPROCESSOR
#if _HAS_CXX20
#include <span>
template <typename ElementType, std::size_t Extent>
using MSpan = std::span<ElementType, Extent>;
#else
#include "span.hpp"
template <typename ElementType, std::size_t Extent = tcb::dynamic_extent>
using MSpan = tcb::span<ElementType, Extent>;
#endif
#endif

