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
    if (this->len != 0) {
        // copy the old string
        memcpy(ptr, this->buffer, this->len);
    }

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

void StringAppendRaw(String *this, Arena *arena, char *other) {
    u32 other_len = strlen(other);
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
    memset(this->buffer, 0, this->cap);
    this->len = 0;
    this->cap = 0;
}
bool StringIsEmpty(String *this) {
    return this->len == 0;
}

static String GenerateIndentation(u32 indentation, Arena* arena) {
    u32 len = indentation * 4;
    char* buffer = ArenaAlloc(arena, len);
    // this's more readable than memset, imo
    // and any compiler will optimize this out to use memset
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

char StringGetChar(String *this, u32 index) {
    assert(index < this->len);
    return this->buffer[index];
}

char StringRemoveChar(String *this, u32 index) {
    assert(index < this->len);
    if (index == this->len - 1) {
        return StringPop(this);
    }
    char c = this->buffer[index];
    memmove(this->buffer + index, this->buffer + index + 1, this->len - index);
    this->len -= 1;
    return c;
}

char StringPeek(String *this) {
    return this->len != 0 ? this->buffer[this->len - 1] : '\0';
}

u32 StringSearchNth(String *this, u32 n, char needle) {
    u32 idx;
    for (idx = 0; idx < this->len && n > 0; idx += 1) {
        if (this->buffer[idx] == needle) n -= 1;
    }
    return idx;
}

void StringInsert(String *this, Arena *arena, u32 index, String *other) {
    if (other->len == 0) return;
    if (index == this->len - 1) {
        StringAppend(this, arena, other);
        return;
    }
    if (this->cap - this->len < other->len) {
        StringResize(this, arena);
    }
    memmove(this->buffer, this->buffer + index + other->len, this->len - index);
    memcpy(this->buffer + index, other->buffer, other->len);
    this->len += other->len;
}

u32 StringIndentationLevel(String *this) {
    u32 idx;
    for (idx = 0; idx < this->len; idx += 1) {
        if (StringGetChar(this, idx) != ' ') break;
    }
    return idx / 4;
}

void StringInsertRaw(String *this, Arena *arena, u32 index, char* raw) {
    u32 raw_len = strlen(raw); 
    if (raw_len == 0) return;
    if (index == this->len - 1) {
        StringAppendRaw(this, arena, raw);
        return;
    }
    if (this->cap - this->len < raw_len) {
        StringResize(this, arena);
    }
    memmove(this->buffer, this->buffer + index + raw_len, this->len - index);
    memcpy(this->buffer + index, raw, raw_len);
    this->len += raw_len;
}

void StringInsertIndentation(String *this, Arena *arena, u32 index, u32 indentation) {
    switch (indentation) {
        case 0:
            break;
        case 1:
            StringInsertRaw(this, arena, index, INDENTATION1);
            break;
        case 2:
            StringInsertRaw(this, arena, index, INDENTATION2);
            break;
        case 3:
            StringInsertRaw(this, arena, index, INDENTATION3);
            break;
        case 4:
            StringInsertRaw(this, arena, index, INDENTATION4);
            break;
        default: {
            String indentation_string = GenerateIndentation(indentation, arena);
            StringInsert(this, arena, index, &indentation_string);
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

/// Semi-open range [from, this-len)
String StringSliceFrom(String *this, u32 from) {
    assert(from <= this->len);
    return (String){.buffer = this->buffer + from, .len = this->len - from, .cap = this->len - from};
}

/// Semi-open range [0, to)
String StringSliceTo(String *this, u32 to) {
    assert(to <= this->len);
    return (String){.buffer = this->buffer, .len = to, .cap = to};
}

/// Semi-open range [from, to)
String StringSliceFromTo(String *this, u32 from, u32 to) {
    assert(from <= to && to <= this->len);
    return (String){.buffer = this->buffer + from, .len = to - from, .cap = to - from};
}

u32 StringCount(String *this, char needle) {
    u32 count = 0;
    for (u32 i = 0; i < this->len; i += 1) {
        if (this->buffer[i] == needle) count += 1;
    }
    return count;
}



u32 StringSearchNth_TODO(String *this, u32 n, char needle) {
    u32 idx;
    for (idx = 0; idx < this->len && n != 0; idx += 1) {
        if (this->buffer[idx] == needle) n -= 1;
        if (n == 0) return idx;
    }
    return idx;
}

u32 StringLineCount(String *multiline) {
    u32 num_lines = 0;
    u32 start = 0;
    while (true) {
        u32 end = StringSearchNth_TODO(multiline, num_lines + 1, '\n');
        if (start < end && end - start != 1) {
            start = end;
            num_lines += 1;
        } else {
            break;
        }
    }
    return num_lines;
}

String StringNthLine(String *multiline, u32 n) {
    u32 start = StringSearchNth_TODO(multiline, n, '\n');
    if (start < multiline->len && multiline->buffer[start] == '\n' && start + 1 < multiline->len) start += 1;
    u32 end = StringSearchNth_TODO(multiline, n + 1, '\n');
    // if (end != 0 && StringGetChar(multiline, end) == '\n') end -= 1;
    return StringSliceFromTo(multiline, start, end);
}
