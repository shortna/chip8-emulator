#ifndef UTILS_H
#define UTILS_H
#include <assert.h>

// we have rust at home
#define EXPECT(x, y) ((x) ? __ASSERT_VOID_CAST(0) : (y))
#define TODO assert(false && "TODO")
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof(*(x)))
#define PANIC(msg) assert(false && (msg))
#define BITu8(x) ((uint8_t)1 << (x))

#endif
