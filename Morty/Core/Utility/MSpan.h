#pragma once

#if __cplusplus >= 202002L
#include <span>
template <typename ElementType, std::size_t Extent>
using MSpan = std::span<ElementType, Extent>;
#else
#include "span.hpp"
template <typename ElementType, std::size_t Extent = tcb::dynamic_extent>
using MSpan = tcb::span<ElementType, Extent>;
#endif

