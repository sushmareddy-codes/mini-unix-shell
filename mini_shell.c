
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX 100
#define ARGS 50

typedef struct {
    char *argv[ARGS];
    char *input;
    char *output;
    int append;
} Command;

/* Split command text into words and operators */
int tokenize(char *line, char *tokens[]) {
    int n = 0;
    char *p = line;

    while (*p) {
        if (*p == ' ' || *p == '\t' || *p == '\n') {
            p++;
            continue;
        }

        if (*p == '|') {
            tokens[n++] = "|";
            p++;
        } else if (*p == '<') {
            tokens[n++] = "<";
            p++;
        } else if (*p == '>') {
            if (*(p + 1) == '>') {
                tokens[n++] = ">>";
                p += 2;
            } else {
                tokens[n++] = ">";
                p++;
            }
        } else {
            char *start = p;

            while (*p && *p != ' ' && *p != '\t' &&
                   *p != '\n' && *p != '|' &&
                   *p != '<' && *p != '>') {
                p++;
            }

            int len = p - start;
            tokens[n] = malloc(len + 1);
            if (!tokens[n]) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            memcpy(tokens[n], start, len);
            tokens[n][len] = '\0';
            n++;
        }

        if (n >= MAX - 1)
            break;
    }

    tokens[n] = NULL;
    return n;
}

/* Parse arguments and redirection operators */
int parse_command(char *tokens[], int start, int end,
                  Command *cmd) {
    int argc = 0;

    memset(cmd, 0, sizeof(*cmd));

    for (int i = start; i < end; i++) {
        if (!strcmp(tokens[i], "<") ||
            !strcmp(tokens[i], ">") ||
            !strcmp(tokens[i], ">>")) {

            if (i + 1 >= end) {
                fprintf(stderr, "Missing redirection filename\n");
                return -1;
            }

            if (!strcmp(tokens[i], "<")) {
                cmd->input = tokens[++i];
            } else {
                cmd->output = tokens[++i];
                cmd->append = !strcmp(tokens[i - 1], ">>");
            }
        } else {
            if (argc >= ARGS - 1) {
                fprintf(stderr, "Too many arguments\n");
                return -1;
            }
            cmd->argv[argc++] = tokens[i];
        }
    }

    cmd->argv[argc] = NULL;
    return argc;
}

/* Apply file redirection */
int redirect_files(Command *cmd) {
    int fd;

    if (cmd->input) {
        fd = open(cmd->input, O_RDONLY);
        if (fd < 0) {
            perror(cmd->input);
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    if (cmd->output) {
        int flags = O_WRONLY | O_CREAT |
                    (cmd->append ? O_APPEND : O_TRUNC);

        fd = open(cmd->output, flags, 0644);
        if (fd < 0) {
            perror(cmd->output);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}

void execute_command(Command *cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (redirect_files(cmd) < 0)
            _exit(1);

        execvp(cmd->argv[0], cmd->argv);
        perror(cmd->argv[0]);
        _exit(127);
    }

    waitpid(pid, NULL, 0);
}

/* Execute two commands connected by a pipe */
void execute_pipe(Command *left, Command *right) {
    int fd[2];

    if (pipe(fd) < 0) {
        perror("pipe");
        return;
    }

    pid_t p1 = fork();

    if (p1 == 0) {
        close(fd[0]);

        if (dup2(fd[1], STDOUT_FILENO) < 0) {
            perror("dup2");
            _exit(1);
        }
        close(fd[1]);

        if (redirect_files(left) < 0)
            _exit(1);

        execvp(left->argv[0], left->argv);
        perror(left->argv[0]);
        _exit(127);
    }

    if (p1 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return;
    }

    pid_t p2 = fork();

    if (p2 == 0) {
        close(fd[1]);

        if (dup2(fd[0], STDIN_FILENO) < 0) {
            perror("dup2");
            _exit(1);
        }
        close(fd[0]);

        if (redirect_files(right) < 0)
            _exit(1);

        execvp(right->argv[0], right->argv);
        perror(right->argv[0]);
        _exit(127);
    }

    close(fd[0]);
    close(fd[1]);

    if (p2 < 0)
        perror("fork");

    waitpid(p1, NULL, 0);
    if (p2 > 0)
        waitpid(p2, NULL, 0);
}

int main(void) {
    char line[MAX];
    char *tokens[MAX];

    while (1) {
        printf("mini-shell$ ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        int n = tokenize(line, tokens);

        if (n == 0)
            continue;

        int pipe_pos = -1;
        int pipes = 0;

        for (int i = 0; i < n; i++) {
            if (!strcmp(tokens[i], "|")) {
                pipe_pos = i;
                pipes++;
            }
        }

        if (pipes > 1) {
            fprintf(stderr,
                    "Only one pipe is supported\n");
            goto cleanup;
        }

        Command left, right;
        int count;

        if (pipes == 1) {
            if (pipe_pos == 0 || pipe_pos == n - 1) {
                fprintf(stderr, "Invalid pipe command\n");
                goto cleanup;
            }

            count = parse_command(tokens, 0, pipe_pos, &left);
            int count2 = parse_command(
                tokens, pipe_pos + 1, n, &right);

            if (count <= 0 || count2 <= 0) {
                fprintf(stderr, "Invalid command\n");
                goto cleanup;
            }

            execute_pipe(&left, &right);
        } else {
            count = parse_command(tokens, 0, n, &left);

            if (count <= 0) {
                fprintf(stderr, "Invalid command\n");
                goto cleanup;
            }

            if (!strcmp(left.argv[0], "exit"))
                goto cleanup_exit;

            if (!strcmp(left.argv[0], "cd")) {
                const char *path = left.argv[1];

                if (!path)
                    path = getenv("HOME");

                if (!path || chdir(path) < 0)
                    perror("cd");

                goto cleanup;
            }

            execute_command(&left);
        }

cleanup:
        for (int i = 0; i < n; i++) {
            if (tokens[i] &&
                strcmp(tokens[i], "|") &&
                strcmp(tokens[i], "<") &&
                strcmp(tokens[i], ">") &&
                strcmp(tokens[i], ">>")) {
                free(tokens[i]);
            }
        }
        continue;

cleanup_exit:
        for (int i = 0; i < n; i++) {
            if (tokens[i] &&
                strcmp(tokens[i], "|") &&
                strcmp(tokens[i], "<") &&
                strcmp(tokens[i], ">") &&
                strcmp(tokens[i], ">>")) {
                free(tokens[i]);
            }
        }
        break;
    }

    printf("Shell terminated.\n");
    return 0;
}
