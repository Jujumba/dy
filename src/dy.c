// Python headers *must* be included before standard headers
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <assert.h>
#include <stdint.h>

#include "arena.h"
#include "core.h"
#include "string.h"
#include "terminal.h"
#include "history.h"

int main(void) {
    PyStatus pystatus;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    config.isolated = 1;
    pystatus = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(pystatus)) {
        goto exception;
    }

    Arena input_arena = {0};
    Arena history_arena = {0};
    ReplHistory history = {0};
    Terminal terminal = TerminalSetup();

    while (1) {
        TerminalStartNewLine(&terminal, &input_arena);

        i32 status = TerminalReadLine(&terminal, &input_arena, &history_arena);
        if (status == Eof) break;

        PyRun_SimpleString(terminal.input.buffer);

        String copy = StringCopy(&terminal.input, &history_arena);
        ArrayPush(&history, &history_arena, copy);

        ArenaReset(&input_arena);
        TerminalResetInput(&terminal);
    }

    ArenaFree(&input_arena);
    ArenaFree(&history_arena);
    Py_FinalizeEx();

    return 0;

exception:
    PyConfig_Clear(&config);
    if (PyStatus_IsExit(pystatus)) {
        return pystatus.exitcode;
    }
    /* display the error message and exit the process with non-zero exit code */
    Py_ExitStatusException(pystatus);
}
