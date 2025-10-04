#pragma once

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "core.h"
#include "string.h"
#include "arena.h"

#define TERM_ESCAPE           "\x1b"
#define TERM_ESCAPE_CHAR      '\x1b'
#define TERM_CLEAR_LINE       TERM_ESCAPE "[2K"
#define TERM_SAVE_POSITION    TERM_ESCAPE "[s"
#define TERM_RESTORE_POSITION TERM_ESCAPE "[u"
#define TERM_ERASE_UNTIL_END  TERM_ESCAPE "[0J"
#define TERM_REQUEST_POSITION TERM_ESCAPE "[6n"
#define TERM_LINE_WRAPPING    TERM_ESCAPE "[?7l"

#define TERM_STYLE_RESET   TERM_ESCAPE "[0m"
#define TERM_STYLE_BOLD    TERM_ESCAPE "[1m"
#define TERM_STYLE_CYAN    TERM_ESCAPE "[36m"
#define TERM_STYLE_BRBLUE  TERM_ESCAPE "[94m"
#define TERM_STYLE_BRBLACK TERM_ESCAPE "[90m"

// Status macros
#define TERM_STATUS_NONE    0
#define TERM_STATUS_EOF    -1

// Keycodes
#define TERM_DEL       0x7F
#define TERM_EOF       0x4
#define TERM_BACKSPACE '\b'
#define TERM_ARROW_UP  '\x1bA'

typedef struct {
    u32 row, col;
} TerminalPosition;

typedef struct {
    struct termios handle;

    /// Dimension of the terminal
    u32 width, height;

    /// Current level of indentation,
    /// if multiline input
    u32 indentation;

    /// Offset in the last line in multiline
    u32 last_line_offset;

    /// Terminal input
    String input;

    // EventRing event_ring;

    /// Saved terminal positions
    // TerminalPositions positions;

    /// How many lines are in each indentation level,
    /// indentation level is the index
    u32 *indentation_lines;
} Terminal;

/// Initialize the terminal
Terminal TerminalSetup(void);

/// Update terminal dimestions, useful for handling resizes
void TerminalUpdateDimension(Terminal *terminal);

/// Read a single char from `stdin`. This function keeps the internal
/// input buffer and what gets printed on the screen in sync.
///
/// Returns the number of bytes read, or a negative status
i32 TerminalInput(Terminal *terminal, Arena* arena);

void TerminalStartNewLine(Terminal *terminal, Arena *arena);

void TerminalPutChar(Terminal *terminal, Arena *arena, char c);
void TerminalPopChar(Terminal *terminal);
void TerminalPutIndentation(Terminal *terminal, Arena *arena);
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
    Terminal terminal = { .handle = handle, };
    TerminalUpdateDimension(&terminal);
    return terminal;
}

// TODO: UTF-8 support
i32 TerminalInput(Terminal *terminal, Arena *arena) {
    char c;
    i32 num_read = read(STDIN_FILENO, &c, 1);
    if (num_read == 0) return false;

    switch (c) {
        case TERM_EOF:
            return TERM_STATUS_EOF;

        case '\r':
        case '\n':
            TerminalSavePosition();
            TerminalPutChar(terminal, arena, c);
        break;

        case TERM_DEL:
        case TERM_BACKSPACE:
            TerminalPopChar(terminal);
        break;

        default: 
            if ((isalnum(c) || isspace(c) || ispunct(c))) {
                TerminalPutChar(terminal, arena, c);
            }
         break;
    }

    return true;
}

// TODO: in the future this should get refactored
//       to allow lexing and syntax highlighting
i32 TerminalReadLine(Terminal *terminal, Arena *arena) {
    while (StringPeek(&terminal->input) != '\n') {
        i32 num_read = TerminalInput(terminal, arena);
        TerminalRender();
        if (num_read == TERM_STATUS_EOF) {
            // cursor is at the current line, move it down
            putc('\n', stdout);
            return TERM_STATUS_EOF;
        }
        if (num_read == TERM_STATUS_NONE) usleep(500);
    }
}

void TerminalStartNewLine(Terminal *terminal, Arena *arena) {
    if (terminal->indentation) {
        terminal->last_line_offset = terminal->input.len;
        fputs(TERM_STYLE_BOLD TERM_STYLE_BRBLACK "..| " TERM_STYLE_RESET, stdout);
        TerminalPutIndentation(terminal, arena);
    } else {
        terminal->last_line_offset = 0;
        fputs(TERM_STYLE_BOLD TERM_STYLE_BRBLUE ">>> " TERM_STYLE_RESET, stdout);
    }
}

void TerminalPutChar(Terminal *terminal, Arena* arena, char c) {
    StringAppendChar(&terminal->input, arena, c);
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

void TerminalPutIndentation(Terminal *terminal, Arena* arena) {
    // TODO: use `StringAddIndentation` and sync with terminal
    for (u32 i = 0; i < terminal->indentation * 4; i += 1) TerminalPutChar(terminal, arena, ' ');
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
