#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int global_counter = 0;

void print_separator(const char *title) {
    printf("\n");
    printf("========================================\n");
    printf("%s\n", title);
    printf("========================================\n");
}

int main(void) {
    print_separator("fork() 基本示範");

    printf("[父] fork() 之前的程式碼\n");
    printf("[父] 此時只有一個進程，PID = %d\n\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        /* ==================== 子進程 ==================== */
        printf("[子] 我是子進程！\n");
        printf("[子] fork() 返回值 = %d（子進程看，返回 0）\n", pid);
        printf("[子] 子進程 PID = %d\n", getpid());
        printf("[子] 父進程 PID = %d\n", getppid());

        global_counter = 100;
        printf("[子] 修改全域變數 global_counter = %d\n", global_counter);

        printf("[子] 子進程睡眠 1 秒...\n");
        sleep(1);

        printf("[子] 子進程結束\n");
        _exit(0);  /* 子進程用 _exit() 而非 exit() */

    } else {
        /* ==================== 父進程 ==================== */
        printf("[父] 我是父進程！\n");
        printf("[父] fork() 返回值 = %d（子進程的 PID）\n", pid);
        printf("[父] 父進程 PID = %d\n", getpid());
        printf("[父] 子進程 PID = %d\n", pid);

        global_counter = 200;
        printf("[父] 修改全域變數 global_counter = %d\n", global_counter);

        printf("[父] 父進程等待子進程結束...\n");

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("[父] 子進程正常結束，退出碼 = %d\n", WEXITSTATUS(status));
        }

        printf("[父] global_counter = %d（與子進程無關，記憶體獨立）\n", global_counter);
    }

    print_separator("fork() 的 Copy-On-Write 概念");

    printf("當 fork() 時，父子進程共享同一份記憶體（唯讀）\n");
    printf("直到任一方嘗試寫入時，系統才複製一份給寫入方\n");
    printf("這大幅提升了 fork() 的效能\n");

    printf("\n最後這行程式碼，父子進程都會執行！\n");
    printf("執行這行程式的進程 PID = %d\n", getpid());

    return 0;
}
