#pragma once

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "arena.h"
#include "core.h"
#include "indentation.h"
#include "print.h"
#include "string.h"

#define TERM_ESCAPE            "\x1b"
#define TERM_ESCAPE_CHAR       '\x1b'
#define TERM_CLEAR_LINE        TERM_ESCAPE "[2K"
#define TERM_SAVE_POSITION     TERM_ESCAPE "[s"
#define TERM_RESTORE_POSITION  TERM_ESCAPE "[u"
#define TERM_ERASE_UNTIL_END   TERM_ESCAPE "[0J"
#define TERM_ERASE_ENTIRE_LINE TERM_ESCAPE "[2K"
#define TERM_REQUEST_POSITION  TERM_ESCAPE "[6n"
#define TERM_LINE_WRAPPING     TERM_ESCAPE "[?7l"
#define TERM_GO_ONE_LINE_UP    TERM_ESCAPE "[1A"

#define TERM_STYLE_RESET   TERM_ESCAPE "[0m"
#define TERM_STYLE_BOLD    TERM_ESCAPE "[1m"
#define TERM_STYLE_CYAN    TERM_ESCAPE "[36m"
#define TERM_STYLE_BRBLUE  TERM_ESCAPE "[94m"
#define TERM_STYLE_BRBLACK TERM_ESCAPE "[90m"

// Status macros
#define TERM_STATUS_NONE     0
#define TERM_STATUS_EOF      -1
#define TERM_STATUS_NEW_LINE -2

// Keycodes
#define TERM_DEL       0x7F
#define TERM_EOF       0x4
#define TERM_BACKSPACE '\b'
#define TERM_ARROW_UP  '\x1bA'

#define TERM_PROMPT_NEW      TERM_STYLE_BOLD TERM_STYLE_BRBLUE ">>>" TERM_STYLE_RESET " "
#define TERM_PROMPT_CONTINUE TERM_STYLE_BOLD TERM_STYLE_BRBLACK "..." TERM_STYLE_RESET " "

typedef struct {
    u32 row, col;
} TerminalPosition;

typedef struct TerminalPositions {
    TerminalPosition *ptr;
    u32 len, cap;
} TerminalPositions;

typedef struct {
    struct termios handle;

    /// Dimension of the terminal
    u32 width, height;

    /// Terminal input
    String input;

    /// Cursor position
    TerminalPosition pos;

} Terminal;

/// Initialize the terminal
Terminal TerminalSetup(void);

/// Update terminal dimestions, useful for handling resizes
void TerminalUpdateDimension(Terminal *terminal);

/// Read a single char from `stdin`. This function keeps the internal
/// input buffer and what gets printed on the screen in sync.
///
/// Returns the number of bytes read, or a negative status
i32 TerminalInput(Terminal *terminal, Arena *arena);

void TerminalStartNewLine(Terminal *terminal, Arena *arena);

void TerminalInsertCharAtCursor(Terminal *terminal, Arena *arena, char c);
void TerminalRemoveCharAtCursor(Terminal *terminal, Arena *arena);
void TerminalMoveCursorUp(Terminal *terminal);
void TerminalMoveCursorDown(Terminal *terminal);
void TerminalPutIndentation(Terminal *terminal, Arena *arena);
String TerminalGetCursorLine(Terminal *terminal);
String TerminalGetPreviousLine(Terminal *terminal);
void TerminalRender(void);
void TerminalEraseUntilEnd(void);
void TerminalClearLine(void);
void TerminalSavePosition(void);
void TerminalRestorePosition(void);
void TerminalEnableWrapping(void);
void TerminalResetInput(Terminal *terminal);
void TerminalMoveCursorLeft(Terminal *terminal);
void TerminalMoveCursorRight(Terminal *terminal);
void TerminalEnsureColumnPosition(Terminal *terminal);
void TerminalReRenderCursorLine(Terminal* terminal);
void TerminalReRenderLinesBelowCursor(Terminal *terminal);

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
    handle.c_cc[VMIN] = 1;
    handle.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &handle);
    Terminal terminal = {
        .handle = handle,
    };
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
            TerminalInsertCharAtCursor(terminal, arena, c);
            return TERM_STATUS_NEW_LINE;
            break;

        case TERM_ESCAPE_CHAR: {
            char buffer[2] = {0};
            (void)read(STDIN_FILENO, buffer, 2);
            if (buffer[0] != '[') break; // not-interesting keycode
            switch (buffer[1]) {
                // arrow up
                case 'A':
                    TerminalMoveCursorUp(terminal);
                    break;

                // arrow down
                case 'B':
                    TerminalMoveCursorDown(terminal);
                    break;

                // arrow left
                case 'D':
                    TerminalMoveCursorLeft(terminal);
                    break;

                // arrow right
                case 'C':
                    TerminalMoveCursorRight(terminal);
                    break;
            }
        } break;

        case TERM_DEL:
        case TERM_BACKSPACE:
            TerminalRemoveCharAtCursor(terminal, arena);
            break;

        default:
            if ((isalnum(c) || isspace(c) || ispunct(c))) {
                TerminalInsertCharAtCursor(terminal, arena, c);
            }
            break;
    }

    return true;
}

// TODO: in the future this should get refactored
//       to allow lexing and syntax highlighting
i32 TerminalReadLine(Terminal *terminal, Arena *arena) {
    i32 status;
    while (true) {
        status = TerminalInput(terminal, arena);
        TerminalRender();
        switch (status) {
            case TERM_STATUS_EOF:
                putc('\n', stdout);
                return TERM_STATUS_EOF;
                break;

            case TERM_STATUS_NONE:
                break;

            case TERM_STATUS_NEW_LINE: {
                u32 total_lines = StringCount(&terminal->input, '\n');
                String last_line = StringNthLine(&terminal->input, terminal->pos.row - 1);
                if (terminal->pos.row == total_lines && StringIsSpace(&last_line)) {
                    goto exit;
                }
           }
           break;
        }
    }
exit:
    return status;
}

void TerminalStartNewLine(Terminal *terminal, Arena *arena) {
    fputs(TERM_PROMPT_NEW, stdout);
    fflush(stdout);
}

void TerminalInsertCharAtCursor(Terminal *terminal, Arena *arena, char c) {
    u32 line_offset = StringSearchNth(&terminal->input, terminal->pos.row, '\n') + terminal->pos.col;
    String current_line = TerminalGetCursorLine(terminal);

    StringInsertChar(&terminal->input, arena, line_offset, c);

    terminal->pos.col += 1;
    TerminalReRenderCursorLine(terminal);
    if (c == '\n') {
        u32 indentation_level = StringIndentationLevel(&current_line);
        if (!StringIsSpace(&current_line)) {
            if (StringGetChar(&terminal->input, line_offset - 1) == ':') {
                indentation_level += 1;
            }
            StringInsertIndentation(&terminal->input, arena, line_offset + 1, indentation_level);
        } 

        TerminalReRenderLinesBelowCursor(terminal);
        terminal->pos.row += 1;
        terminal->pos.col = indentation_level * 4;
        printf("\n");
        // if this is just a one line experession,
        // this would add extra indentation
        if (line_offset != terminal->input.len - 1) {
            TerminalEnsureColumnPosition(terminal);
        }
    }
}

void TerminalRemoveCharAtCursor(Terminal *terminal, Arena *arena) {
    /* if the multiline is empty -- there is nothing to delete */
    if (StringIsEmpty(&terminal->input) || (terminal->pos.row == 0 && terminal->pos.col == 0))
        return;

    u32 line_start = StringSearchNth(&terminal->input, terminal->pos.row, '\n');
    StringRemoveChar(&terminal->input, line_start + terminal->pos.col - 1);

    if (terminal->pos.col == 0) {
        u32 prev_line_start = StringSearchNth(&terminal->input, terminal->pos.row - 1, '\n');
        terminal->pos.col = line_start - prev_line_start - 1;
        TerminalMoveCursorUp(terminal);
        /* clear from cursor to the end of the screen */
        printf("\r\x1b[0J");
        TerminalReRenderCursorLine(terminal);
        TerminalReRenderLinesBelowCursor(terminal);
    } else {
        terminal->pos.col -= 1;
        TerminalReRenderCursorLine(terminal);
    }
    
}

void TerminalMoveCursorUp(Terminal *terminal) {
    if (terminal->pos.row == 0) return;

    String prev_line = StringNthLine(&terminal->input, terminal->pos.row - 1);
    u32 col = terminal->pos.col;
    if (col > prev_line.len) col = prev_line.len;
    terminal->pos.row -= 1;
    terminal->pos.col = col;

    printf(TERM_ESCAPE "[1F");
    TerminalEnsureColumnPosition(terminal);
}

void TerminalMoveCursorDown(Terminal *terminal) {
    u32 total_lines = StringCount(&terminal->input, '\n');
    if (terminal->pos.row == total_lines) return;

    String next_line = StringNthLine(&terminal->input, terminal->pos.row + 1);
    u32 col = terminal->pos.col;
    if (col > next_line.len) col = next_line.len;
    terminal->pos.row += 1;
    terminal->pos.col = col;
    printf("\n");
    TerminalEnsureColumnPosition(terminal);
}

String TerminalGetCursorLine(Terminal *terminal) {
    return StringNthLine(&terminal->input, terminal->pos.row);
}

String TerminalGetPreviousLine(Terminal *terminal) {
    assert(terminal->pos.row > 0);

    return StringNthLine(&terminal->input, terminal->pos.row - 1);
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

void TerminalResetInput(Terminal *terminal) {
    StringReset(&terminal->input);
    terminal->pos = (TerminalPosition){0};
}

void TerminalMoveCursorLeft(Terminal *terminal) {
    if (terminal->pos.col > 0) {
        putc('\b', stdout);
        terminal->pos.col -= 1;
    }
}

void TerminalMoveCursorRight(Terminal *terminal) {
    String current_line = TerminalGetCursorLine(terminal);
    u32 next_position = terminal->pos.col + 1;
    if (next_position <= current_line.len) {
        printf("\x1b[C");
        terminal->pos.col = next_position;
    }
}

void TerminalEnsureColumnPosition(Terminal *terminal) {
    printf("%s[%uG", TERM_ESCAPE, terminal->pos.col + 5);
}

void TerminalReRenderCursorLine(Terminal* terminal) {
    const char* prompt = TERM_PROMPT_NEW;
    if (terminal->pos.row != 0) prompt = TERM_PROMPT_CONTINUE;
    
    String cursor_line = TerminalGetCursorLine(terminal);
    printf("%s\r%s%.*s", TERM_ERASE_ENTIRE_LINE, prompt, cursor_line.len, cursor_line.buffer);
    TerminalEnsureColumnPosition(terminal);
    TerminalRender();
}

void TerminalReRenderLinesBelowCursor(Terminal* terminal) {
    const char* prompt = TERM_PROMPT_CONTINUE;
    u32 num_lines = StringLineCount(&terminal->input);
    u32 current_line_idx = terminal->pos.row + 1;

    // if this is the last line
    if (current_line_idx == num_lines) return;

    for (; current_line_idx < num_lines; current_line_idx += 1) {
        String next_line = StringNthLine(&terminal->input, current_line_idx);
        if (StringIsEmpty(&next_line)) break;
        printf("\n%s%s%.*s", TERM_ERASE_ENTIRE_LINE, prompt, next_line.len, next_line.buffer);
    }

    printf("\x1b[%uF\x1b[%uG", num_lines - terminal->pos.row - 1, terminal->pos.col);
}
