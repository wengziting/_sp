# Unix 程序與檔案描述符 教學指南

## 目錄
1. [檔案描述符 (File Descriptor) 基礎觀念](#檔案描述符-file-descriptor-基礎觀念)
2. [標準輸入/輸出/錯誤 (stdin/stdout/stderr)](#標準輸入輸出錯誤-stdinstdoutstderr)
3. [檔案操作：open, close, read, write](#檔案操作-open-close-read-write)
4. [程序建立：fork](#程序建立-fork)
5. [程序替換：execvp](#程序替換-execvp)
6. [I/O 重導向：dup2](#io-重導向-dup2)
7. [實戰範例：簡易 shell](#實戰範例簡易-shell)

---

## 檔案描述符 (File Descriptor) 基礎觀念

### 什麼是檔案描述符？

**檔案描述符 (File Descriptor, FD)** 是一個**非負整數**，作業系統用它來**識別進程正在訪問的檔案、管道、socket 等資源**。

```
┌─────────────────────────────────────────────────────────────┐
│                     進程 (Process)                          │
│                                                             │
│   檔案描述符表 (每個進程獨立)                                  │
│   ┌────────┬────────┬────────┬────────┬────────┐           │
│   │ FD = 0 │ FD = 1 │ FD = 2 │ FD = 3 │ FD = 4 │ ...        │
│   │ stdin  │ stdout │ stderr │  opened│ opened │            │
│   └────┬───┴────┬───┴────┬───┴────┬───┴────┬───┘            │
│        │        │        │        │        │                 │
└────────┼────────┼────────┼────────┼────────┼─────────────────┘
         │        │        │        │        │
         ▼        ▼        ▼        ▼        ▼
    ┌──────────────────────────────────────────┐
    │           系統層級的「開啟檔案表」          │
    │   (Open File Table - 所有進程共享)         │
    └──────────────────────────────────────────┘
```

### 為什麼是 0, 1, 2 開始？

Unix 設計中，每個進程啟動時，**前三個檔案描述符**已經被系統預設分配：

| FD  | 符號常數  | 預設連接         | 用途           |
|-----|---------|----------------|--------------|
| 0   | `STDIN_FILENO`  | 鍵盤/終端機      | 標準輸入        |
| 1   | `STDOUT_FILENO` | 終端機/螢幕      | 標準輸出        |
| 2   | `STDERR_FILENO` | 終端機/螢幕      | 標準錯誤輸出     |

### 重要規則

```
【規則 1】FD 是非負整數，通常從 0 開始分配
【規則 2】新分配的 FD 永遠是「目前最小的可用非負整數」
【規則 3】如果關閉 FD 0，再開新檔案，新檔案會拿到 FD 0
【規則 4】每個進程有獨立的 FD 表，但共享系統層級的開啟檔案表
```

---

## 標準輸入輸出錯誤 stdin/stdout/stderr

### 三種標準串流

```c
#include <stdio.h>
#include <unistd.h>

// 以下兩種寫法完全等價：
printf("Hello\n");           // 使用 stdio 庫函數
write(STDOUT_FILENO, "Hello\n", 6);  // 直接使用系統調用（FD 1）
```

### stdin/stdout/stderr 的差異

```
┌──────────────────────────────────────────────────────────────┐
│                      終端機 (Terminal)                         │
│                                                              │
│   stdin  ◄────── 鍵盤輸入                                      │
│   stdout ──────► 正常輸出 (可重導向)                            │
│   stderr ──────► 錯誤輸出 (始終顯示，不受重導向影響)             │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 實驗：觀察輸出差异

```bash
# stdout 和 stderr 的差異
./program > output.txt    # 只有 stdout 被重導向，stderr 仍顯示在螢幕
./program 2> errors.txt   # 只有 stderr 被重導向
./program > all.txt 2>&1  # 兩者都重導向到同一個檔案
./program &> all.txt      # 同上（現代語法）
```

### 為什麼需要分開？

```
1.stdout 和 stderr 可以獨立重導向
2.錯誤訊息不會混入正常輸出，方便程式處理
3.可以用 > file.txt 2> errors.txt 分別收集
```

---

## 檔案操作：open, close, read, write

### 函數原型

```c
#include <fcntl.h>      // for open
#include <unistd.h>     // for close, read, write

// 開啟檔案
int open(const char *pathname, int flags, ... /* mode_t mode */);

// 關閉檔案
int close(int fd);

// 讀取資料
ssize_t read(int fd, void *buf, size_t count);

// 寫入資料
ssize_t write(int fd, const void *buf, size_t count);
```

### open() 的 flags

```c
O_RDONLY      // 唯讀
O_WRONLY      // 唯寫
O_RDWR        // 讀寫

// 額外選項（可用 | 組合）：
O_CREAT       // 檔案不存在時創建
O_EXCL        // 與 O_CREAT 搭配，檔案存在則失敗
O_TRUNC       // 截斷為零長度
O_APPEND      // 追加模式
O_NONBLOCK    // 非阻塞模式
```

### 完整範例：基本檔案讀寫

```c
/* demo_basic_file.c - 基本檔案操作範例 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

int main(void) {
    const char *filename = "test.txt";
    const char *message = "Hello, Unix System Programming!\n";

    /* ========== 寫入檔案 ========== */
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open write");
        exit(EXIT_FAILURE);
    }

    ssize_t written = write(fd, message, strlen(message));
    printf("寫入 %zd 位元組到 %s\n", written, filename);
    close(fd);

    /* ========== 讀取檔案 ========== */
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open read");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';  // 加入字串結尾
        printf("讀取 %zd 位元組: %s", bytes_read, buffer);
    }
    close(fd);

    return 0;
}
```

### 執行結果

```bash
$ gcc -o demo_basic_file demo_basic_file.c
$ ./demo_basic_file
寫入 32 位元組到 test.txt
讀取 32 位元組: Hello, Unix System Programming!

$ cat test.txt
Hello, Unix System Programming!
```

---

## 程序建立：fork

### fork() 的核心概念

```
fork() 是 Unix 中最重要的系統調用之一
它創建一個「幾乎完全相同的副本」作為子進程
```

### fork() 的行為

```c
pid_t fork(void);
// 返回值：
//   -1  : 失敗（無足夠記憶體或程序數量達上限）
//   0   : 在子進程中返回
//   > 0 : 在父進程中返回（返回值是新創建的子進程 PID）
```

### 記憶體關係圖

```
                     fork() 之前
        ┌─────────────────────────────────┐
        │           父進程                   │
        │   code, data, heap, stack        │
        │   PID = 1000                     │
        └─────────────────────────────────┘

     fork() ───────────────────────────────────►
                                                          
                     fork() 之後
        ┌───────────────────┐    ┌───────────────────┐
        │      父進程         │    │       子進程        │
        │   PID = 1000       │    │   PID = 1001       │
        │                    │    │                    │
        │   fork() 返回 1001  │    │   fork() 返回 0    │
        │   (子進程的 PID)     │    │   (我才是子進程)     │
        └───────────────────┘    └───────────────────┘
```

### 範例：基本的 fork

```c
/* demo_fork_basic.c - fork() 基本用法 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // 子進程
        printf("[子程序] PID = %d, fork() 返回值 = %d\n", getpid(), pid);
        printf("[子程序] 我是子進程，我會執行自己的程式碼\n");
    } else {
        // 父進程
        printf("[父程序] PID = %d, fork() 返回值 = %d\n", getpid(), pid);
        printf("[父程序] 我創建了子進程 PID = %d\n", pid);
    }

    printf("[%d] 這行程式碼，父子進程都會執行！\n", getpid());

    return 0;
}
```

### 執行結果

```
$ ./demo_fork_basic
[父程序] PID = 1234, fork() 返回值 = 1235
[父程序] 我創建了子進程 PID = 1235
[1234] 這行程式碼，父子進程都會執行！
[子程序] PID = 1235, fork() 返回值 = 0
[子程序] 我是子進程，我會執行自己的程式碼
[1235] 這行程式碼，父子進程都會執行！
```

> **注意**：輸出順序不固定，由作業系統排程決定！

### 重要特性：Copy-On-Write

```
【問】fork() 後，父子進程的記憶體是共享還是分開的？

【答】現代 Unix 系統使用「寫時複製 (Copy-On-Write)」：

  fork() 之後：
  ┌─────────────────────────────────────────────────────┐
  │  父子進程共享同一份物理記憶體（只讀）                    │
  │                                                      │
  │  當任一進程嘗試寫入時：                                │
  │  → 系統複製一份給該進程                               │
  │  → 另一進程繼續共享原記憶體（只讀）                     │
  │                                                      │
  │  優點：大幅提升效能，避免無謂的記憶體複製                │
  └─────────────────────────────────────────────────────┘
```

---

## 程序替換：execvp

### exec 家族

```c
// exec 家族有多個函數，區別在於：
//   l : 參數列表 (list)
//   v : 參數向量 (vector)
//   p : 使用 PATH 環境變數搜尋執行檔
//   e : 可以指定環境變數 (envp)

int execl(const char *path, const char *arg, ... /* (char *) NULL */);
int execlp(const char *file, const char *arg, ... /* (char *) NULL */);
int execle(const char *path, const char *arg, ... /*, (char *) NULL, char * const envp[] */);

int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execvpe(const char *file, char *const argv[], char *const envp[]);
```

### execvp() 詳解

```c
int execvp(const char *file, char *const argv[]);
//   file : 執行檔名稱（如果不含 "/"，則在 PATH 中搜尋）
//   argv : 參數列表（第一個元素通常是程式名稱，必須以 NULL 結尾）

// 成功：不會返回，直接用新程式碼替換當前進程
// 失敗：返回 -1，並設置 errno
```

### exec() 的記憶體變化

```
                     execvp() 之前
        ┌─────────────────────────────────┐
        │           當前進程                │
        │   code    : main()              │
        │   data    : 全域變數              │
        │   heap    : 動態記憶體            │
        │   stack   : 函數呼叫棧            │
        │   PID     : 保持不變              │
        └─────────────────────────────────┘

              execvp("/bin/ls", args)
                         │
                         ▼
                     execvp() 之後
        ┌─────────────────────────────────┐
        │           仍是同一進程             │
        │   PID     : 不變（關鍵！）        │
        │   code    : ls 程式的碼           │
        │   data    : ls 程式的資料          │
        │   heap    : ls 的堆記憶體         │
        │   stack   : ls 的堆疊             │
        │   FD      : 保持不變（繼承）       │
        └─────────────────────────────────┘
```

### 範例：execvp 基本使用

```c
/* demo_execvp.c - execvp() 基本用法 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    printf("[%d] 我是父進程，馬上要 exec 了...\n", getpid());

    // 準備參數：execlp("ls", "ls", "-la", NULL);
    char *args[] = {
        "ls",           // argv[0]，習慣上與程式名相同
        "-la",          // 參數
        NULL            // 參數列表必須以 NULL 結尾
    };

    // execvp 會在 PATH 中搜尋 "ls"
    execvp("ls", args);

    // 如果 execvp 返回，說明執行失敗（這行幾乎不會執行）
    perror("execvp failed");
    return 1;
}
```

### fork() + execvp() 經典組合

```c
/* demo_fork_exec.c - fork + execvp 組合 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // 子進程：執行新程式
        char *args[] = {"ls", "-la", NULL};
        execvp("ls", args);

        // 如果執行到這裡，execvp 失敗了
        perror("execvp failed");
        _exit(127);  // 子進程用 _exit() 而非 exit()
    }

    // 父進程：等待子進程結束
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("\n[父程序] 子進程正常結束，退出碼 = %d\n", WEXITSTATUS(status));
    }

    return 0;
}
```

---

## I/O 重導向：dup2

### dup2() 的作用

```c
int dup2(int oldfd, int newfd);
// 複製 oldfd 到 newfd
// 如果 newfd 已經開啟，會先關閉它
// 返回 newfd（失敗返回 -1）
```

### 核心用途

```
dup2() 的主要用途是「I/O 重導向」：
  → 把標準輸入/輸出/錯誤，替換成檔案或管道
```

### dup2 的示意圖

```
【例 1】dup2(fd, 1) — 將 fd 複製到 stdout (FD 1)
  
  之前：                    之後：
  FD 1 → 終端機              FD 1 → 檔案
  FD 3 → 檔案                FD 3 → 檔案（不變）

【例 2】dup2(fd, 0) — 將 fd 複製到 stdin (FD 0)
  
  之前：                    之後：
  FD 0 → 鍵盤              FD 0 → 檔案
  FD 3 → 檔案                FD 3 → 檔案（不變）
```

### 為什麼 dup2(fd, 1) 能重導向 stdout？

```
因為 stdout 的本質就是「寫入 FD 1」
所以只要把 FD 1 指向檔案，所有寫到 stdout 的內容都會寫入檔案
```

### 範例：stdout 重導向

```c
/* demo_redirect_stdout.c - 將 stdout 重導向到檔案 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("這行不會顯示在終端機！\n");  // 會寫入檔案

    // 將 stdout (FD 1) 替換成 fd
    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    close(fd);  // fd 已複製到 1，可以關閉原本的 fd

    // 之後所有的 stdout 操作都會寫入 output.txt
    printf("這行會寫入 output.txt！\n");
    printf("這行也是！\n");

    return 0;
}
```

### 範例：stdin 重導向（從檔案讀取）

```c
/* demo_redirect_stdin.c - 從檔案讀取作為 stdin */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd = open("input.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 將 stdin (FD 0) 替換成 fd
    dup2(fd, STDIN_FILENO);
    close(fd);

    // 之後 scanf/fgets 會從 input.txt 讀取
    char line[256];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        printf("讀到: %s", line);
    }

    return 0;
}
```

### 實戰：自己實現 `ls > file.txt`

```c
/* my_ls_redirect.c - 實現 ls > output.txt */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // 子進程

        // 開啟目標檔案（用於寫入）
        int fd = open("ls_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open");
            _exit(1);
        }

        // 將 stdout 重導向到檔案
        dup2(fd, STDOUT_FILENO);
        close(fd);

        // 執行 ls（輸出會寫入 ls_output.txt）
        execlp("ls", "ls", "-l", NULL);
        perror("execlp failed");
        _exit(127);
    }

    // 父進程：等待
    wait(NULL);
    printf("ls 的輸出已保存到 ls_output.txt\n");

    return 0;
}
```

---

## 實戰範例：簡易 Shell

這個完整範例展示了如何組合所有概念：
`fork()` + `execvp()` + `dup2()` + 管道 + I/O 重導向

### 完整程式碼

```c
/* mini_shell.c - 簡易 Unix Shell */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

typedef struct {
    char *args[MAX_ARGS];      // 命令參數
    char *input_file;           // 輸入重導向 (e.g., < file)
    char *output_file;          // 輸出重導向 (e.g., > file)
    int background;             // 是否後台執行 (&)
} Command;

void parse_command(char *line, Command *cmd) {
    memset(cmd, 0, sizeof(Command));

    char *tokens[MAX_ARGS];
    int argc = 0;

    // 簡單的字串分割
    char *token = strtok(line, " \t\n");
    while (token && argc < MAX_ARGS - 1) {
        tokens[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    tokens[argc] = NULL;

    // 解析特殊符號
    for (int i = 0; i < argc; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            cmd->input_file = tokens[i + 1];
            tokens[i] = NULL;
        } else if (strcmp(tokens[i], ">") == 0) {
            cmd->output_file = tokens[i + 1];
            tokens[i] = NULL;
        } else if (strcmp(tokens[i], "&") == 0) {
            cmd->background = 1;
            tokens[i] = NULL;
        }
    }

    for (int i = 0; i < argc; i++) {
        cmd->args[i] = tokens[i];
    }
    cmd->args[argc] = NULL;
}

void execute_command(Command *cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // 子進程

        // 輸入重導向：< file
        if (cmd->input_file) {
            int fd = open(cmd->input_file, O_RDONLY);
            if (fd < 0) {
                perror("open input");
                _exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // 輸出重導向：> file
        if (cmd->output_file) {
            int fd = open(cmd->output_file,
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open output");
                _exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(cmd->args[0], cmd->args);
        perror("execvp failed");
        _exit(127);
    }

    // 父進程
    if (!cmd->background) {
        waitpid(pid, NULL, 0);
    } else {
        printf("[後台執行: PID %d]\n", pid);
    }
}

int main(void) {
    char line[MAX_LINE];
    Command cmd;

    printf("=== 迷你 Shell ===\n");
    printf("支援: < 輸入重導向, > 輸出重導向, & 後台執行\n\n");

    while (1) {
        printf("shell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n再見！\n");
            break;
        }

        if (line[0] == '\n') continue;

        parse_command(line, &cmd);

        if (cmd.args[0] == NULL) continue;

        if (strcmp(cmd.args[0], "exit") == 0) {
            printf("再見！\n");
            break;
        }

        execute_command(&cmd);
    }

    return 0;
}
```

### 使用範例

```bash
$ gcc -o mini_shell mini_shell.c

$ ./mini_shell
=== 迷你 Shell ===
支援: < 輸入重導向, > 輸出重導向, & 後台執行

shell> ls
mini_shell.c  mini_shell  ...
shell> ls > file_list.txt
shell> cat < file_list.txt
mini_shell.c
mini_shell
...
shell> exit
再見！
```

---

## 觀念總整理

### FD 分配規則

```
open() 返回的 FD = 目前最小的可用非負整數

範例：
  假設 FD 0, 1, 2 已被佔用
  呼叫 open() → 返回 3
  關閉 FD 1
  再呼叫 open() → 返回 1（覆蓋了 stdout！）
```

### fork + execvp + dup2 的典型模式

```c
pid_t pid = fork();

if (pid == 0) {
    // === 子進程 ===
    
    // 1. 開啟目標檔案
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // 2. 使用 dup2 重導向
    dup2(fd, STDOUT_FILENO);  // stdout → 檔案
    close(fd);                 // 關閉多餘的 fd

    // 3. exec 執行新程式
    execlp("ls", "ls", NULL);
    _exit(1);
}

// 父進程繼續執行
waitpid(pid, NULL, 0);
```

### 這套 API 的設計哲學

```
Unix 的「一切皆檔案」哲學：

1. 檔案、管道、終端機、socket 都是「檔案」
2. 用同一組系統調用（open/close/read/write）操作所有資源
3. FD 是連接進程和系統資源的橋樑
4. dup2() 讓你能任意替換這個連接
```

---

## 習題練習

### 練習 1：理解 FD 分配
```c
close(1);           // 關閉 stdout
int fd = open("out.txt", O_WRONLY);
printf("fd = %d\n", fd);   // 輸出什麼？
```

### 練習 2：實現 `cat < input.txt`
```c
// 用 dup2 重導向 stdin 讀取 input.txt
// 然後讀取並輸出內容
```

### 練習 3：實現 `ls | wc -l`
```c
// 使用 pipe() 建立管道
// fork 兩個子進程
// 分別重導向 stdout 和 stdin 到管道
```

### 練習 4：實現简易 `cp src.txt dst.txt`
```c
// 打開源檔案讀取
// 打開目標檔案寫入
// 讀寫迴圈複製內容
```


---

**編譯與執行**：
```bash
gcc -Wall -Wextra -o demo demo.c
./demo
```
