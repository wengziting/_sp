#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void print_separator(const char *title) {
    printf("\n");
    printf("========================================\n");
    printf("%s\n", title);
    printf("========================================\n");
}

int main(void) {
    print_separator("fork() + execvp() 組合");

    printf("[父] PID = %d，即將 fork()...\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        /* ==================== 子進程 ==================== */
        printf("[子] 子進程 PID = %d\n", getpid());

        printf("[子] 即將執行 execvp(\"ls\", [\"ls\", \"-la\", NULL])\n");
        printf("[子] 注意：exec 後，進程 PID 不變，但程式碼完全被替換\n\n");

        char *args[] = {"ls", "-la", NULL};

        /* execvp() 會在 PATH 中搜尋 "ls" 並執行 */
        execvp("ls", args);

        /* 如果 execvp() 返回，說明執行失敗（這行不會執行） */
        perror("[子] execvp failed");
        _exit(127);

    } else {
        /* ==================== 父進程 ==================== */
        printf("[父] 父進程 PID = %d\n", getpid());
        printf("[父] 已創建子進程 PID = %d，等待它結束...\n", pid);

        int status;
        waitpid(pid, &status, 0);

        printf("\n[父] 子進程已結束\n");
        if (WIFEXITED(status)) {
            printf("[父] 退出碼 = %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[父] 被訊號 %d 終止\n", WTERMSIG(status));
        }
    }

    print_separator("自己實現 ls > output.txt");

    pid = fork();
    if (pid == 0) {
        /* 子進程：執行 ls 並將輸出寫入檔案 */

        int fd = open("ls_output.txt",
                     O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open");
            _exit(1);
        }

        /* 將 stdout (FD 1) 複製到 fd，實現重導向 */
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2");
            _exit(1);
        }
        close(fd);  /* fd 已經複製到 1，可以關閉 */

        /* 執行 ls（輸出會進入 ls_output.txt） */
        execlp("ls", "ls", "-l", NULL);
        perror("execlp failed");
        _exit(1);
    }

    waitpid(pid, NULL, 0);
    printf("\n[父] ls 輸出已保存到 ls_output.txt\n");
    printf("[父] 顯示內容：\n\n");

    int fd = open("ls_output.txt", O_RDONLY);
    char buf[1024];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, n);
    }
    close(fd);

    return 0;
}
