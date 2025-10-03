/// *Must* be included before standard headers
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "string.c"
#include "terminal.h"

#define SUCCESS                   0
#define PROMPT_NEW_STATEMENT      ">>> "
#define PROMPT_CONTINUE_STATEMENT "..| "

char last_char(char* string) {
    ssize_t len = strlen(string);
    assert(len != 0);
    return string[len - 1];
}

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
    Terminal terminal = TerminalSetup();

    fputs(PROMPT_NEW_STATEMENT, stdout);
    while (1) {
        if(!TerminalInput(&terminal) ) {
            TerminalRender();
            goto next;
        }
        /* TODO: lex and highlight input in real time here */
        if (StringPeek(&terminal.input) != '\n' ) goto next;
        String current_line = StringSliceLeft(&terminal.input, terminal.line_offset);
        if (StringIsSpace(&current_line)) {
            if (terminal.indentation) terminal.indentation = 0;
            goto execute;
        }

        if (current_line.buffer[current_line.len - 2] == ':') {
            terminal.indentation += 1;
            goto new_line;
        }

        if (terminal.indentation) goto new_line;

execute:
        /* remove the new line char */
        StringPop(&terminal.input);

        PyRun_SimpleString(terminal.input.buffer);
        
new_line:
        TerminalStartNewLine(&terminal);

next:
        usleep(1000);
    }

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
