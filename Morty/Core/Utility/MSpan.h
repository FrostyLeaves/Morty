#pragma once

#if __cplusplus >= 202002L
#include <span>
MORTY_SPACE_BEGIN
template <typename ElementType, std::size_t Extent>
using MSpan = std::span<ElementType, Extent>;
MORTY_SPACE_END
#else
#include "span.hpp"
MORTY_SPACE_BEGIN
template <typename ElementType, std::size_t Extent = tcb::dynamic_extent>
using MSpan = tcb::span<ElementType, Extent>;
MORTY_SPACE_END
#endif

