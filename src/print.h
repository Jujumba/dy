#pragma once
#include "string.h"

#define Print(x) _Generic(x, String: PrintString, )(x)

void PrintString(String *this) {
    printf("%.*s", this->len, this->buffer);
}
