#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>

static char *line_read = (char*)NULL;

void rl_cleanup() {
    if (line_read) {
        free(line_read);
        line_read = (char*)NULL;
    }
}

char* rl_gets(const char* prompt) {
    rl_cleanup();
    line_read = readline(prompt);
    if (line_read && *line_read) {
        add_history(line_read);
    }

    return line_read;
}