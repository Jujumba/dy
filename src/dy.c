// *Must* be included before standard headers
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "string.h"
#include "terminal.h"
#include "arena.h"

int main(void) {
    char *line;

    PyStatus pystatus;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.isolated = 1;

    pystatus = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(pystatus)) {
        goto exception;
    }

    PyObject *globals, *locals;
    Arena input_arena = ArenaNew();
    Terminal terminal = TerminalSetup();

    while (1) {
        TerminalStartNewLine(&terminal, &input_arena);

        // read the input until we hit the last line
        while (StringPeek(&terminal.input) != '\n') {
            bool read_any = TerminalInput(&terminal, &input_arena);
            TerminalRender();
            if (!read_any) usleep(500);
        }

        String current_line = StringSliceLeft(&terminal.input, terminal.last_line_offset);
        if (StringIsSpace(&current_line)) {
            if (terminal.indentation) terminal.indentation = 0;
            goto execute;
        }

        if (current_line.buffer[current_line.len - 2] == ':') {
            terminal.indentation += 1;
            continue;
        }

        if (terminal.indentation) continue;

execute:
        /* remove the new line char */
        StringPop(&terminal.input);


        PyRun_SimpleString(terminal.input.buffer);
        ArenaReset(&input_arena);
        StringReset(&terminal.input);
        
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
