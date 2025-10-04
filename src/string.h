#pragma once

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "core.h"
#include "arena.h"

#define INDENTATION1 "    "
#define INDENTATION2 "        "
#define INDENTATION3 "            "
#define INDENTATION4 "                "

typedef struct String {
    char *buffer;
    u32 len;
    u32 cap;
} String;

void StringResize(String* this, Arena *arena) {
    u32 new_cap = 0;
    if (this->cap == 0) new_cap = 4096 * 2; // 2 pages
    else                new_cap = this->cap + (this->cap / 2);

    char* ptr = ArenaAlloc(arena, new_cap);

    // make sure we are in bounds
    assert(this->len < new_cap);
    // copy the old string
    memcpy(ptr, this->buffer, this->len);

    this->buffer = ptr;
    this->cap = new_cap;
}

bool StringHasCapacity(String *this) {
    return this->len < this->cap;
}

void StringResizeIfNeeded(String* this, Arena* arena) {
    if (!StringHasCapacity(this)) StringResize(this, arena);
}

String StringSliceLeft(String *this, u32 offset) {
    assert(offset <= this->len);
    return (String){
        .buffer = this->buffer + offset, .len = this->len - offset, .cap = this->cap - offset};
}

void StringAppend(String *this, Arena *arena, String *other) {
    StringResizeIfNeeded(this, arena);
    memcpy(this->buffer + this->len, other->buffer, other->len);
    this->len += other->len;
}

void StringAppendChar(String *this, Arena* arena, char c) {
    StringResizeIfNeeded(this, arena);
    assert(this->len < this->cap);
    this->buffer[this->len] = c;
    this->len += 1;
}

void StringAppendRaw(String *this, Arena *arena, char *other, u32 other_len) {
    StringResizeIfNeeded(this, arena);
    assert(this->len + other_len <= this->cap);
    memcpy(this->buffer + this->len, other, other_len);
    this->len += other_len;
}

void StringInsertChar(String *this, Arena *arena, u32 index, char c) {
    assert(index <= this->len);
    if (index == this->len) {
        StringAppendChar(this, arena, c);
        return;
    }
    StringResizeIfNeeded(this, arena);
    memmove(this->buffer + index + 1, this->buffer + index, this->len - index);
    this->buffer[index] = c;
    this->len += 1;
}

void StringClear(String *this) {
    this->len = 0;
}

void StringReset(String *this) {
    this->len = 0;
    this->cap = 0;
}
bool StringIsEmpty(String *this) {
    return this->len == 0;
}

static String GenerateIndentation(u32 indentation, Arena* arena) {
    u32 len = indentation * 4;
    char* buffer = ArenaAlloc(arena, len);
    for (u32 i = 0; i < len; i += 1) buffer[i] = ' ';
    return (String){.buffer = buffer, .len = len, .cap = len};
}

char StringPop(String *this) {
    char c = '\0';
    if (this->len > 0) {
        c = this->buffer[this->len - 1];
        this->buffer[this->len - 1] = '\0';
        this->len -= 1;
    }
    return c;
}

char StringPeek(String *this) {
    return this->len != 0 ? this->buffer[this->len - 1] : '\0';
}

void StringAddIndentation(String *this, Arena *arena, u32 indentation) {
    switch (indentation) {
        case 0:
            break;
        case 1:
            StringAppendRaw(this, arena, INDENTATION1, 4);
            break;
        case 2:
            StringAppendRaw(this, arena, INDENTATION2, 8);
            break;
        case 3:
            StringAppendRaw(this, arena, INDENTATION3, 12);
            break;
        case 4:
            StringAppendRaw(this, arena, INDENTATION4, 16);
            break;
        default: {
            String indentation_string = GenerateIndentation(indentation, arena);
            StringAppend(this, arena, &indentation_string);
        } break;
    }
}

bool StringIsSpace(String *this) {
    for (u32 i = 0; i < this->len; i += 1) {
        if (!isspace(this->buffer[i])) {
            return false;
        }
    }
    return true;
}
