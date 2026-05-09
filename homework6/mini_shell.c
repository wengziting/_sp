#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE  1024
#define MAX_ARGS   64
#define MAX_PATHS  64

typedef struct {
    char *args[MAX_ARGS];      /* 命令參數列表 */
    char *input_file;           /* 輸入重導向 < */
    char *output_file;          /* 輸出重導向 > */
    int pipe_to;                /* 是否有管道，指向下一個命令 */
    int background;             /* 是否後台執行 (&) */
    int arg_count;              /* 參數數量 */
} Command;

typedef struct {
    Command commands[16];
    int num_commands;
} Pipeline;

void print_banner(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║       迷你 Shell - 系統程式教學           ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║  支援功能：                               ║\n");
    printf("║    • 基本命令執行 (ls, cat, ps...)        ║\n");
    printf("║    • 輸入重導向 < file                    ║\n");
    printf("║    • 輸出重導向 > file                    ║\n");
    printf("║    • 管道 | (cmd1 | cmd2)                 ║\n");
    printf("║    • 後台執行 &                           ║\n");
    printf("║    • 內建命令: cd, pwd, exit, help        ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    printf("\n");
}

void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

int parse_line(char *line, Pipeline *pipeline) {
    memset(pipeline, 0, sizeof(Pipeline));

    Command *cmd = &pipeline->commands[0];
    pipeline->num_commands = 1;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->pipe_to = 0;
    cmd->background = 0;
    cmd->arg_count = 0;

    char *tokens[MAX_ARGS];
    int argc = 0;

    /* 基本 token 分割 */
    char *token = strtok(line, " \t\n");
    while (token && argc < MAX_ARGS - 1) {
        tokens[argc++] = token;
        token = strtok(NULL, " \t\n");
    }

    /* 解析管道 - 先找出所有命令 */
    int cmd_indices[16] = {0};
    int cmd_count = 1;

    for (int i = 0; i < argc; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            tokens[i] = NULL;
            cmd_indices[cmd_count++] = i + 1;
        }
    }

    /* 為每個命令填充 args */
    for (int c = 0; c < cmd_count; c++) {
        Command *cur_cmd = &pipeline->commands[c];
        int start = cmd_indices[c];

        /* 找到這個命令的結束（下一個 NULL 或陣列結尾） */
        int count = 0;
        for (int i = start; i < argc && tokens[i] != NULL; i++) {
            if (c == cmd_count - 1 || tokens[i + 1] != NULL) {
                cur_cmd->args[count++] = tokens[i];
            }
        }
        cur_cmd->args[count] = NULL;
        cur_cmd->arg_count = count;

        if (c < cmd_count - 1) {
            cur_cmd->pipe_to = 1;
        }
    }
    pipeline->num_commands = cmd_count;

    /* 解析特殊符號（< > &）- 只處理最後一個命令 */
    Command *last = &pipeline->commands[pipeline->num_commands - 1];
    for (int i = 0; i < last->arg_count; i++) {
        if (strcmp(last->args[i], "<") == 0 && i + 1 < last->arg_count) {
            last->input_file = last->args[i + 1];
            last->args[i] = NULL;
            last->args[i + 1] = NULL;
            i++;
        } else if (strcmp(last->args[i], ">") == 0 && i + 1 < last->arg_count) {
            last->output_file = last->args[i + 1];
            last->args[i] = NULL;
            last->args[i + 1] = NULL;
            i++;
        } else if (strcmp(last->args[i], "&") == 0) {
            last->background = 1;
            last->args[i] = NULL;
        }
    }

    return pipeline->num_commands;
}

int count_args(char **args) {
    int count = 0;
    while (args[count] != NULL) {
        count++;
    }
    return count;
}

int execute_pipeline(Pipeline *pipeline) {
    int num_cmds = pipeline->num_commands;
    int pipefd[2];
    int prev_fd = -1;
    pid_t pids[16];
    int status;

    for (int i = 0; i < num_cmds; i++) {
        Command *cmd = &pipeline->commands[i];

        /* 建立管道（除了最後一個命令） */
        if (i < num_cmds - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe failed");
                return -1;
            }
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            return -1;
        }

        if (pid == 0) {
            /* ===== 子進程 ===== */

            /* 重導向 stdin（如果是第一個命令或從管道來） */
            if (prev_fd != -1) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            /* 從檔案重導向 stdin */
            if (cmd->input_file != NULL) {
                int fd = open(cmd->input_file, O_RDONLY);
                if (fd < 0) {
                    fprintf(stderr, "無法開啟輸入檔案: %s\n", cmd->input_file);
                    _exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            /* 重導向 stdout（如果是最後一個命令或到管道） */
            if (i < num_cmds - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }

            /* 輸出重導向到檔案 */
            if (cmd->output_file != NULL) {
                int fd = open(cmd->output_file,
                             O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    fprintf(stderr, "無法開啟輸出檔案: %s\n", cmd->output_file);
                    _exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            /* 執行命令 */
            execvp(cmd->args[0], cmd->args);
            fprintf(stderr, "命令執行失敗: %s\n", cmd->args[0]);
            _exit(127);
        }

        /* ===== 父進程 ===== */
        pids[i] = pid;

        if (prev_fd != -1) {
            close(prev_fd);
        }
        if (i < num_cmds - 1) {
            close(pipefd[1]);  /* 關閉寫端 */
            prev_fd = pipefd[0];
        }
    }

    /* 等待所有子進程 */
    for (int i = 0; i < num_cmds; i++) {
        Command *cmd = &pipeline->commands[i];
        if (!cmd->background) {
            waitpid(pids[i], &status, 0);
        } else {
            printf("[後台執行: PID %d]\n", pids[i]);
        }
    }

    return 0;
}

int handle_builtin(Command *cmd) {
    if (cmd->args[0] == NULL) return 0;

    /* cd 命令 */
    if (strcmp(cmd->args[0], "cd") == 0) {
        const char *dir = cmd->args[1] ? cmd->args[1] : getenv("HOME");
        if (dir && chdir(dir) == 0) {
            char cwd[256];
            printf("現在目錄: %s\n", getcwd(cwd, sizeof(cwd)));
        } else {
            fprintf(stderr, "cd: 無法前往 %s\n", dir ? dir : "(null)");
        }
        return 1;
    }

    /* pwd 命令 */
    if (strcmp(cmd->args[0], "pwd") == 0) {
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
        return 1;
    }

    /* exit 命令 */
    if (strcmp(cmd->args[0], "exit") == 0 ||
        strcmp(cmd->args[0], "quit") == 0) {
        printf("再見！\n");
        exit(0);
    }

    /* help 命令 */
    if (strcmp(cmd->args[0], "help") == 0) {
        printf("可用命令：\n");
        printf("  cd <目錄>     - 切換目錄\n");
        printf("  pwd           - 顯示當前目錄\n");
        printf("  exit          - 離開 shell\n");
        printf("  < cmd         - 從檔案讀取輸入\n");
        printf("  > file        - 輸出到檔案\n");
        printf("  cmd1 | cmd2   - 管道\n");
        printf("  cmd &         - 後台執行\n");
        return 1;
    }

    return 0;  /* 不是內建命令 */
}

void print_prompt(void) {
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\n[%s] $ ", cwd);
    } else {
        printf("\n$ ");
    }
    fflush(stdout);
}

int main(void) {
    char line[MAX_LINE];
    Pipeline pipeline;

    print_banner();

    while (1) {
        print_prompt();

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        trim_newline(line);

        if (strlen(line) == 0) continue;

        /* 解析命令 */
        parse_line(line, &pipeline);

        /* 處理內建命令 */
        Command *first_cmd = &pipeline.commands[0];
        if (pipeline.num_commands == 1 &&
            !first_cmd->pipe_to &&
            !first_cmd->input_file &&
            !first_cmd->output_file &&
            !first_cmd->background) {
            if (handle_builtin(first_cmd)) {
                continue;
            }
        }

        /* 執行外部命令 */
        execute_pipeline(&pipeline);
    }

    printf("\n感謝使用！\n");
    return 0;
}
