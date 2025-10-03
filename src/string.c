#pragma once
#include "core.h"

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <sys/mman.h>

#define INDENTATION1 "    "
#define INDENTATION2 "        "
#define INDENTATION3 "            "
#define INDENTATION4 "                "

typedef struct String {
    char *buffer;
    u32 len;
    u32 cap;
} String;

String StringNew(void) {
    u32 cap = 4096 * 2;
    char *buffer = mmap(0, cap, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(buffer);
    // Leave the last char for \0, treat it as unavailable
    return (String){.buffer = buffer, .len = 0, .cap = cap - 1};
}

void StringAppendChar(String *this, char c) {
    assert(this->len < this->cap - 1);
    this->buffer[this->len] = c;
    this->len += 1;
}

void StringAppend(String *this, String *other) {
    assert(this->len + other->len <= this->cap);
    memcpy(this->buffer + this->len, other->buffer, other->len);
    this->len += other->len;
}

void StringAppendRaw(String *this, char *other, u32 other_len) {
    assert(this->len + other_len <= this->cap);
    memcpy(this->buffer + this->len, other, other_len);
    this->len += other_len;
}

void StringReset(String *this) { this->len = 0; }
bool StringIsEmpty(String *this) { return this->len == 0; }

void StringFree(String this) { munmap(this.buffer, this.cap + 1); }

static String GenerateIndentation(u32 indentation) {
    const u32 len = indentation * 4;
    const u32 cap = (u32)(ceil(log(len) / log(4096)));
    char *buffer = mmap(0, cap, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (String){.buffer = buffer, .len = len, .cap = cap};
}

void StringAddIndentation(String *this, u32 indentation) {
    switch (indentation) {
        case 1:
            StringAppendRaw(this, INDENTATION1, 4);
            break;
        case 2:
            StringAppendRaw(this, INDENTATION2, 8);
            break;
        case 3:
            StringAppendRaw(this, INDENTATION3, 12);
            break;
        case 4:
            StringAppendRaw(this, INDENTATION3, 16);
            break;
        default: {
            String indentation_string = GenerateIndentation(indentation);
            StringAppend(this, &indentation_string);
            StringFree(indentation_string);
        } break;
    }
}
