/// *Must* be included before standard headers
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <assert.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "string.c"

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
    // TODO: implement an arena
    String current_code = StringNew();
    unsigned int indentation = 0, current_statement_len = 0;
    const char *prompt = PROMPT_NEW_STATEMENT;

    while (1) {
        line = readline(prompt);
        if (!line) break;
        if (*line == '\0') {
            if (indentation) {
                /* end of current indentation */
                indentation -= 1;
                if (indentation == 0) {
                    prompt = PROMPT_NEW_STATEMENT;
                    goto execute;
                } else {
                    goto next;
                }
            } else {
                /* empty line in the global context */
                break; 
            }
        }

        if (last_char(line) == ':') {
            StringAddIndentation(&current_code, indentation);
            indentation += 1;
            StringAppendRaw(&current_code, line, strlen(line));
            StringAppendChar(&current_code, '\n');
            prompt = PROMPT_CONTINUE_STATEMENT;
            goto next;
        }


        if (indentation) {
            StringAddIndentation(&current_code, indentation);
            StringAppendRaw(&current_code, line, strlen(line));
            StringAppendChar(&current_code, '\n');
            goto next;
        }
    
execute:
        int execution_status = 0;
        if (!StringIsEmpty(&current_code)) {
            printf("%s", current_code.buffer);
            execution_status = PyRun_SimpleString(current_code.buffer);
            StringReset(&current_code);
        } else {
            execution_status = PyRun_SimpleString(line);
        }

next:
        add_history(line);
        free(line);
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
