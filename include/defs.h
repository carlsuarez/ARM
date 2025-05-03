#pragma once

#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)

#define RESERVE_U32(n) uint32_t CONCAT2(_pad_, __LINE__)[n]
#define RESERVE_U8(n) uint8_t CONCAT2(_pad_, __LINE__)[n]