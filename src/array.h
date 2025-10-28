#pragma once

#include <math.h>
#include <string.h>

#include "arena.h"
#include "core.h"

typedef struct ArrayHeader {
    u32 len, cap;
} ArrayHeader;

#define INITIAL_CAP 16

#define ArrayEnsureAdditionalCap(array, arena, additional)                                         \
    do {                                                                                           \
        u32 ___array_remaining = (array)->header.cap - (array)->header.len;                        \
        if (___array_remaining >= additional) break;                                               \
        u32 ___array_additional_cap =                                                              \
            (array)->header.cap != 0 ? (array)->header.cap : INITIAL_CAP;                          \
        if (___array_additional_cap < additional) ___array_additional_cap = additional;            \
        u32 ___array_new_cap = (array)->header.cap + ___array_additional_cap;                      \
        void *___array_new_buffer =                                                                \
            ArenaAlloc(arena, ___array_new_cap * sizeof(*(array)->buffer));                        \
        if ((array)->header.len != 0 && (array)->buffer) {                                         \
            memcpy(___array_new_buffer, (array)->buffer,                                           \
                   (array)->header.len * sizeof(*(array)->buffer));                                \
        }                                                                                          \
        (array)->buffer = ___array_new_buffer;                                                     \
        (array)->header.cap = ___array_new_cap;                                                    \
    } while (0);

#define ArrayPush(array, arena, item)                                                              \
    do {                                                                                           \
        ArrayEnsureAdditionalCap(array, arena, 1);                                                 \
        (array)->header.len += 1;                                                                  \
        (array)->buffer[(array)->header.len - 1] = item;                                           \
    } while (0);

#define ArrayPop(array)                                                                            \
    ({                                                                                             \
        assert((array)->header.len != 0 && "Array should contain at least 1 element");             \
        typeof(*(array)->buffer) __array_item = (array)->buffer[(array)->header.len - 1];          \
        (array)->header.len -= 1;                                                                  \
        __array_item;                                                                              \
    })

#define ArrayGetNth(array, index)                                                                  \
    ({                                                                                             \
        assert(index < (array)->header.len && "Array index should be in-bounds");                  \
        typeof(*(array)->buffer) __array_item = (array)->buffer[index];                            \
        __array_item;                                                                              \
    })

#define ArrayIsEmpty(array) (array)->header.len == 0

#define ArrayLen(array) (array)->header.len
