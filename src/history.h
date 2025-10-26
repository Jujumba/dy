#include "core.h"
#include "arena.h"
#include "array.h"
#include "string.h"

typedef struct ReplHistory {
    ArrayHeader header;
    String *buffer;
} ReplHistory;
