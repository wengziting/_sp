#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];
    
    if (pipe(fd) < 0) { 
        perror("pipe 建立失敗"); 
        exit(EXIT_FAILURE); 
    }

    if (fork() == 0) {
        dup2(fd[1], STDOUT_FILENO);
        
        close(fd[0]); 
        close(fd[1]); 

        char *args[] = {"ls", NULL};
        execvp(args[0], args);
        
        perror("exec ls 失敗");
        _exit(EXIT_FAILURE);
    }

    if (fork() == 0) {
        dup2(fd[0], STDIN_FILENO);
        
        close(fd[0]); 
        close(fd[1]); 

        char *args[] = {"wc", "-l", NULL};
        execvp(args[0], args);
        
        perror("exec wc 失敗");
        _exit(EXIT_FAILURE);
    }

    close(fd[0]); 
    close(fd[1]);

    wait(NULL); 
    wait(NULL); 
    
    printf("[父程序] 管道指令 ls | wc -l 執行成功並安全回收。\n");
    return 0;
}
