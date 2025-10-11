#pragma once

#include "arena.h"
#include "core.h"

/// Information about saved line
typedef struct LineInfo {
    /// Length of the line
    u32 len;

    /// Indentation level, useful to know when
    /// a last line with this indent level got
    /// removed
    u32 indentation;
} LineInfo;

/// Ma, look! It's a dynamic array
typedef struct LineInfos {
    LineInfo *ptr;
    u32 len, cap;
} LineInfos;

void LineInfosPush(LineInfos *this, Arena *arena, LineInfo info);
LineInfo LineInfosPop(LineInfos *this);
LineInfo LineInfosPeek(LineInfos *this);
void LineInfosResizeIfNeeded(LineInfos *this, Arena *arena);
void LineInfosReset(LineInfos *this);

void LineInfosPush(LineInfos *this, Arena *arena, LineInfo info) {
    LineInfosResizeIfNeeded(this, arena);
    this->ptr[this->len++] = info;
}

LineInfo LineInfosPop(LineInfos *this) {
    if (this->len == 0) return (LineInfo){0};
    LineInfo info = LineInfosPeek(this);
    this->len -= 1;
    return info;
}

LineInfo LineInfosPeek(LineInfos *this) {
    if (this->len == 0) return (LineInfo){0};
    return this->ptr[this->len - 1];
}

void LineInfosResizeIfNeeded(LineInfos *this, Arena *arena) {
    if (this->len < this->cap) return;

    u32 new_cap = 0;
    if (this->cap == 0) new_cap = 512; // 1 page
    else                new_cap = this->cap + (this->cap / 2);

    LineInfo* ptr = ArenaAlloc(arena, new_cap * sizeof(LineInfo));

    memcpy(ptr, this->ptr, this->cap);

    this->ptr = ptr;
    this->cap = new_cap;
}

void LineInfosReset(LineInfos *this) {
    memset(this->ptr, 0, this->cap);
    *this = (LineInfos){};
}

LineInfo LineInfosGet(LineInfos *this, u32 index) {
    assert(index < this->len);
    return this->ptr[index];
}
