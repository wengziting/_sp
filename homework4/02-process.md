# 程序（Process）

## 什麼是程序

程序是執行中的程式實例，每個程序都有獨立的位址空間、系統資源和執行緒。

## 建立新程序

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        printf("子程序: PID = %d, PPID = %d\n", getpid(), getppid());
    } else {
        printf("父程序: PID = %d, 子程序 PID = %d\n", getpid(), pid);
    }
    
    return 0;
}
```

## 程序狀態

- **執行中（Running）**：正在 CPU 上執行
- **就緒（Ready）**：等待 CPU 排程
- **阻塞（Blocked）**：等待 I/O 或事件
