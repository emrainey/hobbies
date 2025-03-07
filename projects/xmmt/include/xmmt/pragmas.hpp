#ifndef XMMT_PRAGMAS_HPP
#define XMMT_PRAGMAS_HPP

// Requires __SSE2__ and __AVX__ in the least for intel intrinsics

#if defined(__x86_64__)
#include <emmintrin.h>
#include <immintrin.h>
#elif defined(__arm__) or defined(__aarch64__) or defined(__arm64__)
#include "sse2neon.h"
#endif

#if defined(__AVX512F__) and (__AVX512F__ == 1)
#pragma message("AVX512 Enabled")
#else
#pragma message("AVX512 Disabled")
#endif

#if defined(__AVX2__) and (__AVX2__ == 1)
#pragma message("AVX2 Enabled")
#else
#pragma message("AVX2 Disabled")
#endif

#if defined(__AVX__) and (__AVX__ == 1)
#pragma message("AVX Enabled")
#else
#pragma message("AVX Disabled")
#endif

#if defined(__SSE3__) and (__SSE3__ == 1)
#pragma message("SSE3 Enabled")
#else
#pragma message("SSE3 Disabled")
#endif

#if defined(__SSE2__) and (__SSE2__ == 1)
#pragma message("SSE2 Enabled")
#else
#pragma message("SSE2 Disabled")
#endif

#if defined(__SSE__) and (__SSE__ == 1)
#pragma message("SSE Enabled")
#else
#pragma message("SSE Disabled")
#endif

#endif  // XMMT_PRAGMAS_HPP
