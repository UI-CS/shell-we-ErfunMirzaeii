//Alireza Mirzaei ----------- 4024013110


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

char last_command[MAX_LINE];
int has_history = 0;

char **parse_input(char *input) {
    char **args = malloc(MAX_ARGS * sizeof(char *));
    if (!args) {
        perror("malloc failed");
        return NULL;
    }

    char *token;
    int i = 0;

    token = strtok(input, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return args;
}

int handle_builtin(char **args) {

    // related to the exit command 

    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }
    // related to the cd command 
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            char *home = getenv("HOME");
            if (!home) {
                fprintf(stderr, "cd: HOME not set\n");
            } else if (chdir(home) != 0) {
                perror("cd failed");
            }
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd failed");
            }
        }
        return 1;
    }
    //related to the pwd command 
    if (strcmp(args[0], "pwd") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("%s\n", cwd);
        } else {
            perror("pwd failed");
        }
        return 1;
    }
    //related to the help command 
    if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  exit      Exit the shell\n");
        printf("  cd [dir]  Change directory\n");
        printf("  pwd       Print working directory\n");
        printf("  help      Show this help message\n");
        printf("  history   Show last command\n");
        return 1;
    }
    //related to the history command(use command with --> !!) 
    if (strcmp(args[0], "history") == 0) {
        if (!has_history) {
            printf("No commands in history\n");
        } else {
            printf("%s", last_command);
        }
        return 1;
    }

    return 0;
}

//managing pipe execution :

void execute_pipe(char **args) {
    int pipefd[2];
    pid_t pid1, pid2;
    char *cmd1[MAX_ARGS];
    char *cmd2[MAX_ARGS];

    int i = 0, j = 0;
    while (args[i] != NULL && strcmp(args[i], "|") != 0) {
        cmd1[i] = args[i];
        i++;
    }
    cmd1[i] = NULL;
    i++;

    while (args[i] != NULL) {
        cmd2[j++] = args[i++];
    }
    cmd2[j] = NULL;

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

    pid1 = fork();
    if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd1[0], cmd1);
        perror("exec failed");
        exit(1);
    }

    pid2 = fork();
    if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        execvp(cmd2[0], cmd2);
        perror("exec failed");
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int main() {
    char input[MAX_LINE];

    while (1) {

        while (waitpid(-1, NULL, WNOHANG) > 0);

        printf("uinxsh> ");
        fflush(stdout);

        if (fgets(input, MAX_LINE, stdin) == NULL) {
            printf("\nExiting shell...\n");
            break;
        }

        if (strcmp(input, "!!\n") == 0) {
            if (!has_history) {
                printf("No commands in history\n");
                continue;
            }
            printf("%s", last_command);
            strcpy(input, last_command);
        } else {
            strcpy(last_command, input);
            has_history = 1;
        }

        char **args = parse_input(input);
        if (!args || args[0] == NULL) {
            free(args);
            continue;
        }

        if (handle_builtin(args)) {
            free(args);
            continue;
        }

        int has_pipe = 0;
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "|") == 0) {
                has_pipe = 1;
                break;
            }
        }

        if (has_pipe) {
            execute_pipe(args);
            free(args);
            continue;
        }

        int background = 0;
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "&") == 0) {
                background = 1;
                args[i] = NULL;
                break;
            }
        }

        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        } else if (pid > 0) {
            if (!background)
                waitpid(pid, NULL, 0);
            else
                printf("[Running in background] PID %d\n", pid);
        }

        free(args);
    }

    return 0;
}
