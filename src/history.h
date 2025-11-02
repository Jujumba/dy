#pragma once

#include "array.h"
#include "string.h"

typedef struct ReplHistory {
    ArrayHeader header;
    String     *buffer;
} ReplHistory;
