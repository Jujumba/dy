#pragma once

#include <assert.h>
#include <ctype.h>
#include <memory.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "arena.h"
#include "core.h"

#define INDENTATION1 "    "
#define INDENTATION2 "        "
#define INDENTATION3 "            "
#define INDENTATION4 "                "

#define S(raw)                                                                                     \
    (String) { .buffer = raw, .len = strlen(raw), .cap = strlen(raw) }

typedef struct String {
    char *buffer;
    u32   len;
    u32   cap;
} String;

void StringAppend(String *this, Arena *arena, String *other);
void StringAppendRaw(String *this, Arena *arena, char *other);
void StringAppendChar(String *this, Arena *arena, char c);
void StringInsert(String *this, Arena *arena, u32 index, String *other);
void StringInsertChar(String *this, Arena *arena, u32 index, char c);
void StringInsertRaw(String *this, Arena *arena, u32 index, char *raw);
void StringNulTerminate(String *this, Arena *arena);
void StringMemZero(String *this);

char StringGetChar(String *this, u32 index);
char StringRemoveChar(String *this, u32 index);
char StringPop(String *this);
char StringPeek(String *this);

void StringClear(String *this);
void StringReset(String *this);

void StringEnsureAdditional(String *this, Arena *arena, u32 additional);
bool StringIsEmpty(String *this);
bool StringHasCapacity(String *this);

String StringSliceFrom(String *this, u32 from);
String StringSliceTo(String *this, u32 to);
String StringSliceFromTo(String *this, u32 from, u32 to);

u32    StringSearchNth(String *this, u32 n, char needle);
u32    StringSearchNthAddOne(String *this, u32 n, char needle);
String StringNthLine(String *multiline, u32 n);

u32 StringCount(String *this, char needle);
u32 StringLineCount(String *multiline);

bool StringIsSpace(String *this);
bool StringEndsWith(String *string, char end);
bool StringIsPyTerminated(String *string);

bool CharIsSpace(char c);
bool CharIsDigit(char c);
bool CharIsAlnum(char c);
bool CharIsDigit(char c);
bool CharIsPunct(char c);
bool CharIsQuote(char c);
bool CharIsOperator(char c);
bool CharIsPrintable(char c);

String StringCopy(String *this, Arena *arena);

String GenerateIndentation(u32 indentation, Arena *arena);
u32    StringIndentationLevel(String *this);
void   StringInsertIndentation(String *this, Arena *arena, u32 index, u32 indentation);
String StringRightTrim(String *this);

void StringAppend(String *this, Arena *arena, String *other) {
    StringEnsureAdditional(this, arena, other->len);
    if (other->len) {
        memcpy(this->buffer + this->len, other->buffer, other->len);
        this->len += other->len;
    }
}

void StringAppendChar(String *this, Arena *arena, char c) {
    StringEnsureAdditional(this, arena, 1);
    assert(this->len < this->cap);
    this->buffer[this->len] = c;
    this->len += 1;
}

void StringAppendRaw(String *this, Arena *arena, char *other) {
    u32 other_len = strlen(other);
    StringEnsureAdditional(this, arena, other_len);
    assert(this->len + other_len <= this->cap);
    memcpy(this->buffer + this->len, other, other_len);
    this->len += other_len;
}

void StringInsert(String *this, Arena *arena, u32 index, String *other) {
    if (other->len == 0) return;
    StringEnsureAdditional(this, arena, other->len);
    if (index == this->len - 1) {
        StringAppend(this, arena, other);
        return;
    }
    memmove(this->buffer + index + other->len, this->buffer + index, this->len - index);
    memcpy(this->buffer + index, other->buffer, other->len);
    this->len += other->len;
    StringNulTerminate(this, arena);
}

void StringInsertChar(String *this, Arena *arena, u32 index, char c) {
    assert(index <= this->len);
    if (index == this->len) {
        StringAppendChar(this, arena, c);
        return;
    }
    memmove(this->buffer + index + 1, this->buffer + index, this->len - index);
    this->buffer[index] = c;
    this->len += 1;
    StringNulTerminate(this, arena);
}

void StringInsertRaw(String *this, Arena *arena, u32 index, char *raw) {
    u32 raw_len = strlen(raw);
    StringEnsureAdditional(this, arena, raw_len);
    if (index == this->len - 1) {
        StringAppendRaw(this, arena, raw);
        return;
    }
    memmove(this->buffer + index + raw_len, this->buffer + index, this->len - index);
    memcpy(this->buffer + index, raw, raw_len);
    this->len += raw_len;
    StringNulTerminate(this, arena);
}

void StringNulTerminate(String *this, Arena *arena) {
    StringEnsureAdditional(this, arena, 1);
    assert(this->cap - this->len > 1);
    this->buffer[this->len] = '\0';
}

void StringMemZero(String *this) {
    if (this->cap) {
        memset(this->buffer, 0, this->cap);
    }
}

char StringGetChar(String *this, u32 index) {
    if (index >= this->len) return 0;
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

char StringPop(String *this) {
    char c = '\0';
    if (this->len > 0) {
        c = this->buffer[this->len - 1];
        this->buffer[this->len - 1] = '\0';
        this->len -= 1;
    }
    return c;
}

char StringPeek(String *this) { return this->len != 0 ? this->buffer[this->len - 1] : '\0'; }

void StringClear(String *this) { this->len = 0; }

void StringReset(String *this) {
    memset(this->buffer, 0, this->cap);
    this->len = 0;
    this->cap = 0;
}

void StringEnsureAdditional(String *this, Arena *arena, u32 additional) {
    u32 remaining = this->cap - this->len;
    if (remaining >= additional) return;

    u32   additional_cap = this->cap != 0 ? this->cap : 64;
    u32   new_cap = this->cap + additional_cap;
    char *new_buffer = ArenaAlloc(arena, new_cap);

    if (this->len != 0 && this->buffer) {
        memcpy(new_buffer, this->buffer, this->len);
    }

    StringMemZero(this);

    this->buffer = new_buffer;
    this->cap = new_cap;
}

bool StringIsEmpty(String *this) { return this->len == 0; }

bool StringHasCapacity(String *this) { return this->len < this->cap; }

String StringSliceFrom(String *this, u32 from) {
    assert(from <= this->len);
    return (String){
        .buffer = this->buffer + from, .len = this->len - from, .cap = this->len - from};
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

u32 StringSearchNth(String *this, u32 n, char needle) {
    u32 idx;
    for (idx = 0; idx < this->len && n != 0; idx += 1) {
        if (this->buffer[idx] == needle) n -= 1;
        if (n == 0) break;
    }
    return idx;
}

u32 StringSearchNthAddOne(String *this, u32 n, char needle) {
    u32 idx = 0;
    for (idx = 0; idx < this->len && n != 0; idx += 1) {
        if (this->buffer[idx] == needle) n -= 1;
        if (n == 0) return idx + 1;
    }
    return idx;
}

String StringNthLine(String *multiline, u32 n) {
    u32 start = StringSearchNthAddOne(multiline, n, '\n');
    u32 end = StringSearchNth(multiline, n + 1, '\n');
    return StringSliceFromTo(multiline, start, end);
}

u32 StringCount(String *this, char needle) {
    u32 count = 0;
    for (u32 i = 0; i < this->len; i += 1) {
        if (this->buffer[i] == needle) count += 1;
    }
    return count;
}

/// Returns the line count, not including
/// empty lines
u32 StringLineCount(String *multiline) {
    u32 num_lines = 0;
    u32 start = 0;
    while (true) {
        u32 end = StringSearchNth(multiline, num_lines + 1, '\n');
        if (start >= end || end - start <= 1) break;
        start = end;
        num_lines += 1;
    }
    return num_lines;
}

bool StringIsSpace(String *this) {
    for (u32 i = 0; i < this->len; i += 1) {
        if (!isspace(this->buffer[i])) {
            return false;
        }
    }
    return true;
}

bool StringEndsWith(String *string, char end) {
    if (string->len == 0) return false;
    return string->buffer[string->len - 1] == end;
}

bool StringIsPyTerminated(String *string) {
    String trimmed = StringRightTrim(string);
    return !(StringEndsWith(&trimmed, ':') || StringEndsWith(&trimmed, '\\'));
}

bool CharIsSpace(char c) { return c == ' '; }

bool CharIsDigit(char c) { return c >= '0' && c <= '9'; }

bool CharIsAlnum(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

bool CharIsPunct(char c) {
    char punctuations[] = {
        ',',
        ';',
        '.',
        ':',
    };
    for (u32 i = 0; i < sizeof(punctuations); i += 1) {
        if (c == punctuations[i]) return true;
    }
    return false;
}

bool CharIsOperator(char c) {
    char operators[] = {'+', '-', '*', '/', '&', '|', '^', '!',
                        '%', '<', '>', '=', '(', ')', '[', ']'};
    for (u32 i = 0; i < sizeof(operators); i += 1) {
        if (c == operators[i]) return true;
    }
    return false;
}

bool CharIsQuote(char c) { return c == '\'' || c == '\"'; }

bool CharIsPrintable(char c) { return c >= 32 && c <= 127; }

String StringCopy(String *this, Arena *arena) {
    String copy = {0};
    StringAppend(&copy, arena, this);
    return copy;
}

String GenerateIndentation(u32 indentation, Arena *arena) {
    u32   len = indentation * 4;
    char *buffer = ArenaAlloc(arena, len);
    // this's more readable than memset, imo
    // and any compiler will optimize this out to use memset
    for (u32 i = 0; i < len; i += 1)
        buffer[i] = ' ';
    return (String){.buffer = buffer, .len = len, .cap = len};
}

u32 StringIndentationLevel(String *this) {
    u32 idx;
    for (idx = 0; idx < this->len; idx += 1) {
        if (StringGetChar(this, idx) != ' ') break;
    }
    return idx / 4;
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

String StringRightTrim(String *this) {
    if (StringIsEmpty(this)) return (String){0};

    u32 trimmed = this->len;
    while (trimmed > 0 && isspace(this->buffer[trimmed - 1]))
        trimmed -= 1;

    return (String){.len = trimmed, .cap = trimmed, .buffer = this->buffer};
}
