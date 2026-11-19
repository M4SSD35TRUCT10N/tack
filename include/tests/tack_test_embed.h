#ifndef TACK_TEST_EMBED_H
#define TACK_TEST_EMBED_H

/* Clang targeting the MSVC UCRT: use ISO C89 CRT APIs without Microsoft's
 * vendor-only 'unsafe' deprecation annotations. This opt-out must precede
 * ALL CRT/Windows headers; it does not disable -Werror or other diagnostics.
 * fopen_s/getenv_s/strerror_s are not ISO C89 replacements. */
#if defined(_WIN32) && defined(_MSC_VER) && defined(__clang__)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#define TACK_TEST 1

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include "tack.c"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
