// *Must* be included before standard headers
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <assert.h>
#include <stdint.h>

#include "arena.h"
#include "core.h"
#include "string.h"
#include "terminal.h"

int main(void) {
    PyStatus pystatus;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    config.isolated = 1;
    pystatus = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(pystatus)) {
        goto exception;
    }

    Arena input_arena = ArenaNew();
    Terminal terminal = TerminalSetup();

    while (1) {
        TerminalStartNewLine(&terminal, &input_arena);

        // read line
        i32 status = TerminalReadLine(&terminal, &input_arena);
        if (status == TERM_STATUS_EOF) break;

        String current_line = TerminalGetPreviousLine(&terminal);
        if (StringIsSpace(&current_line)) {
            terminal.indentation = 0;
        } else if (current_line.buffer[current_line.len - 1] == ':') {
            terminal.indentation += 1;
        }

        if (terminal.indentation) continue;

        /* remove the new line char */
        StringPop(&terminal.input);

        PyRun_SimpleString(terminal.input.buffer);

        ArenaReset(&input_arena);
        TerminalResetInput(&terminal);
    }

    ArenaFree(&input_arena);
    Py_FinalizeEx();

    return 0;

exception:
    PyConfig_Clear(&config);
    if (PyStatus_IsExit(pystatus)) {
        return pystatus.exitcode;
    }
    /* Display the error message and exit the process with
       non-zero exit code */
    Py_ExitStatusException(pystatus);
}
