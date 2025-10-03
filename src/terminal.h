#pragma once

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "core.h"
#include "event.h"
#include "string.c"

#define TERM_ESCAPE           "\x1b"
#define TERM_ESCAPE_CHAR      '\x1b'
#define TERM_CLEAR_LINE       TERM_ESCAPE "[2K"
#define TERM_SAVE_POSITION    TERM_ESCAPE "[s"
#define TERM_RESTORE_POSITION TERM_ESCAPE "[u"
#define TERM_ERASE_UNTIL_END  TERM_ESCAPE "[0J"
#define TERM_REQUEST_POSITION TERM_ESCAPE "[6n"
#define TERM_LINE_WRAPPING    TERM_ESCAPE "[?7l"

#define TERM_STYLE_RESET TERM_ESCAPE "[0m"
#define TERM_STYLE_BOLD  TERM_ESCAPE "[1m"
#define TERM_STYLE_CYAN  TERM_ESCAPE "[36m"

// Keycodes
#define TERM_DEL       0x7F
#define TERM_BACKSPACE '\b'
#define TERM_ARROW_UP  '\x1bA'

// Internal macros
#define __TERMINAL_POSITIONS_BUFFER_SIZE 10

typedef struct {
    u32 row, col;
} TerminalPosition;

typedef struct {
    TerminalPosition buffer[__TERMINAL_POSITIONS_BUFFER_SIZE];
    u32 head;
} TerminalPositions;

typedef struct {
    struct termios handle;
    u32 width, height, indentation, line_offset;
    String input;
    // EventRing event_ring;
    TerminalPositions positions;
} Terminal;

/// Initialize the terminal
Terminal TerminalSetup(void);

/// Update terminal dimestions, useful for handling resizes
void TerminalUpdateDimension(Terminal *terminal);

/// Read a single char from `stdin`. This function keeps the internal
/// input buffer and what gets printed on the screen in sync.
bool TerminalInput(Terminal *terminal);

void TerminalStartNewLine(Terminal *terminal);

void TerminalPutChar(Terminal *terminal, char c);
void TerminalPopChar(Terminal *terminal);
void TerminalPutIndentation(Terminal *terminal);
void TerminalRender(void);
void TerminalEraseUntilEnd(void);
void TerminalClearLine(void);
void TerminalSavePosition(void);
void TerminalRestorePosition(void);
void TerminalEnableWrapping(void);

void TerminalUpdateDimension(Terminal *terminal) {
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
    terminal->width = window.ws_col;
    terminal->height = window.ws_row;
}

Terminal TerminalSetup(void) {
    struct termios handle = {0};
    tcgetattr(STDIN_FILENO, &handle);

    // set noncanonical mode and unset echo:
    //   don't render typed input - those will be printed anyway
    handle.c_lflag &= ~(ICANON | ECHO);

    // non blocking read
    handle.c_cc[VMIN] = 0;
    handle.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &handle);
    Terminal terminal = {.handle = handle, .input = StringNew()};
    TerminalUpdateDimension(&terminal);
    return terminal;
}

bool TerminalInput(Terminal *terminal) {
    char c;
    bool read_any = read(STDIN_FILENO, &c, 1);
    if (!read_any) return false;

    switch (c) {
        case '\r':
        case '\n': {
            TerminalSavePosition();
            TerminalPutChar(terminal, c);

        } break;

        case TERM_BACKSPACE:
        case TERM_DEL: {
            TerminalPopChar(terminal);
        } break;

        default: {
            if ((isalnum(c) || isspace(c) || ispunct(c))) {
                StringAppendChar(&terminal->input, c);
                putc(c, stdout);
            }
        } break;
    }

    return true;
}

void TerminalStartNewLine(Terminal *terminal) {
    if (terminal->indentation) {
        // fprintf(stderr, "current line offset is: %u, and the line is `%s`\n",
        //         terminal->line_offset, terminal->input.buffer + terminal->line_offset);
        terminal->line_offset = terminal->input.len;
        fputs("..| ", stdout);
        TerminalPutIndentation(terminal);
    } else {
        StringReset(&terminal->input);
        terminal->line_offset = 0;
        fputs(">>> ", stdout);
    }
}

void TerminalPutChar(Terminal *terminal, char c) {
    StringAppendChar(&terminal->input, c);
    putc(c, stdout);
}

void TerminalPopChar(Terminal *terminal) {
    if (StringIsEmpty(&terminal->input)) return;
    char c = StringPop(&terminal->input);
    /* if this is not the last char in the current line */
    if (c != '\n') {
        /* this only moves the cursor back */
        putc(TERM_BACKSPACE, stdout);
        TerminalEraseUntilEnd();
    } else {
        /* before going one line down, position of the end of the prev. line was saved */
        TerminalClearLine();
        TerminalRestorePosition();
    }
}

void TerminalPutIndentation(Terminal *terminal) {
    // TODO: use `StringAddIndentation` and sync with terminal
    for (u32 i = 0; i < terminal->indentation * 4; i += 1) TerminalPutChar(terminal, ' ');
}

void TerminalRender(void) {
    fflush(stdout);
}

void TerminalEraseUntilEnd(void) {
    printf("%s", TERM_ERASE_UNTIL_END);
}
void TerminalClearLine(void) {
    printf("%s\r", TERM_CLEAR_LINE);
}
void TerminalSavePosition(void) {
    printf("%s", TERM_SAVE_POSITION);
}
void TerminalRestorePosition(void) {
    printf("%s", TERM_RESTORE_POSITION);
}
void TerminalEnableWrapping(void) {
    printf("%s", TERM_LINE_WRAPPING);
}
