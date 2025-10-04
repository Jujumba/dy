#pragma once

#include <assert.h>
#include <sys/mman.h>

#include "core.h"

typedef struct Arena {
    u8 *ptr;
    u32 bound, allocated;
} Arena;

Arena ArenaNew(void) {
    u32 GibiByte = 1073741824;
    u8 *ptr = mmap(0, 2 * GibiByte, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(ptr && "well okay this time I can't alloc 2 gibibytes. btw it's gibibyte, not "
                  "gigabyte. Though I don't populate the pages");
    return (Arena){.ptr = ptr, .bound = 2 * GibiByte};
}

// TODO: Aligned alloc
u8* ArenaAlloc(Arena *this, u32 size) {
    if (this->allocated + size >= this->bound) assert(false && "Arena 4GiB limit exceeded");
    u8* ptr = this->ptr + this->allocated + size;
    this->allocated += size;
    return ptr;
}

void ArenaReset(Arena* this) {
    this->allocated = 0;
}

void ArenaFree(Arena *this) {
    i32 status = munmap(this->ptr, this->bound);
    assert(status == 0 && "failed to unmap arena's buffer");
}
