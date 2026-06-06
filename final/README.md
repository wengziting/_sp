目錄
* [# HOMEWORK 1：編譯器四大階段精煉](#-homework-1編譯器四大階段精煉)
* [# HOMEWORK 2：FluxScript 資料流語言](#-homework-2fluxscript-資料流語言)
* [# HOMEWORK 3：自製 8-Bit 虛擬電腦硬體架構](#-homework-3自製-8-bit-虛擬電腦硬體架構)
* [# HOMEWORK 4：系統程式與核心觀念（程序與執行緒）](#-homework-4系統程式與核心觀念程序與執行緒)
* [# HOMEWORK 5：執行緒同步三大經典問題](#-homework-5執行緒同步三大經典問題)
* [# HOMEWORK 6：Unix 檔案描述符與系統調用實戰](#-homework-6unix-檔案描述符與系統調用實戰)
* [# HOMEWORK 7：軟硬體邊界與多工同步懶人包](#-homework-7軟硬體邊界與多工同步懶人包)

---

## 📝 # HOMEWORK 1：編譯器四大階段精煉

### 1. 詞法分析 (Lexer) —— 切碎與標籤
* **做什麼**：逐字掃描代碼，把整串字串切碎並貼上標籤（Token）。
* **重點**：自動過濾空格和註解。
* **例子**：`while` ➡️ `[關鍵字: while]` ； `123` ➡️ `[數字: 123]`。

### 2. 語法分析 (Parser) —— 檢查句型結構
* **做什麼**：用「遞歸下降法」檢查單字順序是否符合文法。
* **重點**：
  * **優先權**：確保「先乘除後加減」。
  * **嚴謹性**：少寫括號 `(` 或分號 `;` 馬上報錯攔截。

### 3. 中間代碼生成 (IR) —— 拆解成流水帳
* **做什麼**：把複雜語法拆解成極簡的 **「四元組 (Quadruples)」** 格式：`(操作, 參數1, 參數2, 結果)`。
* **例子**：`a = b + 5` ➡️ 拆成 `ADD b 5 t1` 接著 `STORE t1 a`。
* **目的**：讓後面的虛擬機器（VM）只需要按表操課。

### 4. 虛擬機器 (VM) —— 模擬大腦執行
* **做什麼**：模擬一台電腦，按順序執行第三階段產生的四元組。

[返回目錄](#-快速導覽目錄)

---

## 📝 # HOMEWORK 2：FluxScript 資料流語言

### 1. 概念簡介
FluxScript 是一種極簡、意圖導向 (Intent-oriented) 的微型資料流語言。它把複雜的程式邏輯簡化為「水管接水」的過程，讓資料處理變得極度直觀。

```text
 [輸入數據] ──► ( |> 過濾/轉換 ) ──► ( -> 最終歸宿 ) ──► [輸出結果]
2. 三大核心特性
沒有變數宣告：不保留複雜的狀態與短期記憶，只有資料的流動。

管道運算子 (|>)：模擬資料經過一連串過濾器與轉換器的過程。

終端意圖 (->)：明確定義資料最終要流向哪裡（Sink / 輸出）。

3. 語法範例
Plaintext
[1, 2, 3, 4, 5] |> sum -> output
白話解讀：拿著這一串陣列，丟進（|>）求和漏斗，最後流向（->）螢幕輸出。

4. 執行階段 (VM) 的實作差異
相較於前面需要「堆疊幀 (Stack Frame)」來處理複雜函數的 VM，這款 FluxScript 的解譯器（Interpreter）在 Python 裡會變成極致的連鎖呼叫 (Method Chaining)：

Python
# 後台 VM 的極簡運作本質 (按表操課)
data = [1, 2, 3, 4, 5]
result = sum(data)      # 處理 |> sum
print(result)           # 處理 -> output
返回目錄

📝 # HOMEWORK 3：自製 8-Bit 虛擬電腦硬體架構
這台虛擬電腦透過軟體（Node.js），在後台精準模擬了一顆真實硬體晶片的運作邏輯。

1. 三大角色分工
記憶體 (Memory)：一個擁有 256 個格子的櫃子（定址 0x00 ~ 0xFF）。環境放一個 8-bit 數字（0~255），同時存放「指令」與「資料」。

中央處理器 (CPU)：電腦大腦。內含程式計數器 (PC) 記錄當前執行的格子位置，以及暫存器 (Registers) 當作大腦的短期記憶。

指令集 (Instruction Set)：機器的溝通語言（Opcode）。例如規定 0x01 叫 LOAD（搬資料）、0x02 叫 ADD（加法）、0x00 叫 HALT（停機）。

2. 三大步驟循環 (Fetch-Decode-Execute)
當執行 node src/main.js 時，CPU 就會陷入以下無限循環，直到遇見停機指令：

擷取 (Fetch)：CPU 根據 PC 的地址，從記憶體櫃子裡拿出一條數字指令，隨後 PC 自動往後移。

解碼 (Decode)：CPU 查看該數字，透過 switch-case 開關電路解析出它的行為（例如：讀到 0x02 知道要做加法）。

執行 (Execute)：CPU 真正呼叫內部電路做運算（例如把兩個暫存器的數字加起來），存回結果，並準備進入下一個循環。

返回目錄

📝 # HOMEWORK 4：系統程式與核心觀念（程序與執行緒）
1. 系統程式（System Programming）
核心定義：直接與作業系統核心（Kernel）及硬體互動的底層程式設計。

主要範疇：資源管理、效能優化、高穩定性（多用 C/C++ 或組合語言）。

2. 程序（Process）與記憶體
定義：執行中的程式實例。每個程序擁有完全隔離的獨立位址空間與資源。

建立方法：使用 fork()。

傳回值 == 0：目前在子程序中執行。

傳回值 > 0：目前在父程序中執行（拿到的是子程序的 PID）。

程序三狀態：執行中（Running）、就緒（Ready）、阻塞（Blocked，如等待 I/O）。

💡 記憶體佈局（Layout）
作業系統透過虛擬記憶體映射實體記憶體，每個程序的內部結構分為：

Text / Data / BSS：存放程式碼與全域/靜態變數。

Heap（堆）：向上增長，用 malloc() 動態配置，必須用 free() 手動釋放。

Stack（棧）：向下增長，自動管理函數呼叫、傳入參數與區域變數。

3. 執行緒（Thread）
定義：程序內部的基本執行單位（輕量級程序），多個執行緒共享同一程序的位址空間。

動態操作：用 pthread_create() 建立，用 pthread_join() 等待回收。

返回目錄

📝 # HOMEWORK 5：執行緒同步三大經典問題
1. 銀行存提款問題（Race Condition 競爭條件）
核心問題：多個執行緒同時讀寫同一個變數（帳戶餘額），導致計算結果互相覆蓋、資料遺失。

解決方案：使用 Mutex（互斥鎖） 把修改餘額的程式碼鎖起來，變成「臨界區段」，一次只允許一個執行緒進入。

關鍵程式碼：

C
pthread_mutex_lock(&mutex);   // 加鎖
balance += 100;               // 臨界區段操作
pthread_mutex_unlock(&mutex); // 解鎖
2. 生產者消費者問題（Buffer 同步與互斥）
核心問題：

緩衝區滿時，生產者必須等待（不能再放）。

緩衝區空時，消費者必須等待（沒有東西拿）。

多人同時存取緩衝區會打架（需要互斥）。

解決方案：Mutex（管互斥）+ 2個 Semaphore（信號量，管計數與等待）。

empty：記錄空位（初始值 = 緩衝區大小）。

full：記錄產品（初始值 = 0）。

核心邏輯：

生產者：sem_wait(&empty) ➡️ 加鎖 ➡️ 放資料 ➡️ 解鎖 ➡️ sem_post(&full)。

消費者：sem_wait(&full) ➡️ 加鎖 ➡️ 取資料 ➡️ 解鎖 ➡️ sem_post(&empty)。

3. 哲學家用餐問題（Deadlock 死結）
核心問題：5 個哲學家圍成一圈，如果每個人都同時拿起「左手邊」的叉子，所有人就會陷入永遠等待右手邊叉子的無限卡死狀態，即 Deadlock（死結）。

解決方案：破壞循環等待。改用交替順序策略：

偶數 ID 哲學家：先拿左，再拿右。

奇數 ID 哲學家：先拿右，再拿左。

返回目錄

📝 # HOMEWORK 6：Unix 檔案描述符與系統調用實戰
1. 檔案描述符 (File Descriptor, FD)
定義：核心用來識別進程正在存取資源（檔案、管道、Socket）的非負整數。每個進程都有獨立的 FD 表。

預設分配：

0 (STDIN_FILENO)：標準輸入（鍵盤）

1 (STDOUT_FILENO)：標準輸出（螢幕）

2 (STDERR_FILENO)：標準錯誤（螢幕，不受一般 > 重導向影響）

分配核心規則：新分配的 FD 永遠是目前最小的可用非負整數。

2. 核心系統調用 (System Calls)
A. 檔案操作：

open(path, flags, mode)：開啟檔案。常用 Flags 有 O_RDONLY、O_WRONLY、O_CREAT、O_TRUNC、O_APPEND。

close(fd)：關閉資源，釋放 FD。

read(fd, buf, count) / write(fd, buf, count)：底層 I/O 讀寫。

B. 程序控制：

fork()：複製當前進程，建立子進程。現代作業系統採 寫時複製 (Copy-On-Write) 技術。子程序內傳回 0，父程序內傳回 子程序的 PID。

execvp(file, argv)：用新程式碼替換當前進程。PID 與已開啟的 FD 保持不變（被繼承）。成功時絕不返回。

C. I/O 重導向：

dup2(oldfd, newfd)：核心功能為複製 oldfd 並覆蓋 newfd。printf 的本質是寫入 FD 1，透過 dup2(fd, 1) 即可將輸出重新導向至檔案。

🛠️ 核心實戰樣式 (Pattern)
樣式一：fork + execvp + dup2（實現 ls > output.txt）

C
int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (fork() == 0) {
    dup2(fd, STDOUT_FILENO); // stdout (1) 重導向到檔案
    close(fd);               // 關閉多餘的 fd
    
    char *args[] = {"ls", "-l", NULL};
    execvp("ls", args);      // 替換程式，FD 1 仍指向檔案
    _exit(127);
}
close(fd);
wait(NULL); // 父程序等待子程序結束
樣式二：簡易串接 Shell 核心架構

C
// 迷你 Shell 內執行命令與重導向的底層邏輯
void execute_command(Command *cmd) {
    if (fork() == 0) {
        if (cmd->input_file) {  // < input.txt
            int fd = open(cmd->input_file, O_RDONLY);
            dup2(fd, STDIN_FILENO); close(fd);
        }
        if (cmd->output_file) { // > output.txt
            int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO); close(fd);
        }
        execvp(cmd->args[0], cmd->args);
        _exit(127);
    }
    if (!cmd->background) wait(NULL); // 若無 & 則等待
}
返回目錄

📝 # HOMEWORK 7：軟硬體邊界與多工同步懶人包
🛠️ 第一部分：自製 CPU 核心
三大件：

Memory：256 格子，同時裝指令與資料。

CPU：有 PC（記地址）和 Registers（短期記憶）。

Opcode：指令代碼（如 0x02 = 加法）。

執行循環：Fetch（拿指令、PC+1）➡️ Decode（用 switch 判斷）➡️ Execute（ALU 計算）。

🔀 第二部分：作業系統多工與同步
匿名管道 (Pipe)

本質：Kernel 記憶體內的 FIFO 緩衝區，限有 fork() 血緣的進程單向通訊。

致命地雷：fork() 後寫入端 (fd[1]) 沒關乾淨 ➡️ 讀取端永不結束 ➡️ 進程卡死 (Hang)。

條件變數 (Condition Variable)

本質：避免忙碌等待（Busy Waiting）的通知機制。

原子三部曲：放鎖 ➡️ 睡眠 ➡️ 被喚醒時重搶鎖。

致命地雷：多核會有「虛假喚醒」，必須用 while 迴圈檢查條件。
