# Thread 程式說明文件

## 1. 銀行存提款模擬程式 (bank.c)

### 問題描述
模擬銀行帳戶的存款和提款操作。同一帳戶進行 100,000 次存提款（各 50,000 次），最終餘額應與初始餘額相同。

### 問題分析
若無同步機制，多執行緒同時讀寫餘額時會發生 Race Condition：
- 執行緒 A 讀取餘額 = 1000000
- 執行緒 B 讀取餘額 = 1000000
- 執行緒 A 計算後寫入餘額 = 900000
- 執行緒 B 計算後寫入餘額 = 900000
- 正確應為 800000，產生資料遺失

### 解決方案
使用 Mutex 保護臨界區段：
```c
pthread_mutex_lock(&mutex);
balance += 100;  // 或 balance -= 100
pthread_mutex_unlock(&mutex);
```

### 執行結果
```
Initial balance: 1000000
Threads: 8, Operations per thread: 12500
Total operations: 100000
Final balance: 1000000
Result: CORRECT
```

---

## 2. 生產者消費者問題 (producer_consumer.c)

### 問題描述
生產者產生資料放入緩衝區，消費者從緩衝區取出資料。緩衝區大小為 5，需要生產和消費 20 個項目。

### 問題分析
需要解決：
1. 緩衝區滿時，生產者必須等待
2. 緩衝區空時，消費者必須等待
3. 多個執行緒同時訪問緩衝區需要互斥

### 解決方案
使用 Semaphore 和 Mutex：
- `empty`：記錄空槽數量，初始化為 BUFFER_SIZE
- `full`：記錄可用項目數量，初始化為 0
- `mutex`：保護緩衝區的互斥訪問

```c
// 生產者
sem_wait(&empty);        // 等待空槽
pthread_mutex_lock(&mutex);
buffer[in] = item;
in = (in + 1) % BUFFER_SIZE;
pthread_mutex_unlock(&mutex);
sem_post(&full);        // 增加可用項目

// 消費者
sem_wait(&full);         // 等待項目
pthread_mutex_lock(&mutex);
item = buffer[out];
out = (out + 1) % BUFFER_SIZE;
pthread_mutex_unlock(&mutex);
sem_post(&empty);      // 增加空槽
```

### 執行結果
生產者和消費者正確交替執行，最終緩衝區為空，計數為 0。

---

## 3. 哲學家用餐問題 (dining_philosophers.c)

### 問題描述
5 位哲學家圍坐圓桌，每位哲學家左右各有一把叉子。哲學家需要同時拿起左右兩把叉子才能用餐。

### 問題分析
若每位哲學家按相同順序拿叉子，會發生 Deadlock：
- 哲學家 0 拿起叉子 0
- 哲學家 1 拿起叉子 1
- ...
- 哲學家 4 拿起叉子 4
- 所有哲學家都在等待另一把叉子

### 解決方案
採用交替順序策略：
- 偶數 ID 哲學家：先拿左邊叉子，再拿右邊
- 奇數 ID 哲學家：先拿右邊叉子，再拿左邊

這破壞了循環等待條件，避免 Deadlock。

### 執行結果
```
Philosopher 0 ate 3 meals
Philosopher 1 ate 3 meals
Philosopher 2 ate 3 meals
Philosopher 3 ate 3 meals
Philosopher 4 ate 3 meals
```

所有哲學家都成功用餐 3 次，無死結發生。

---

## 4. 程式比較與總結

| 程式 | 同步機制 | 解決的問題 |
|------|----------|------------|
| bank.c | Mutex | Race Condition |
| producer_consumer.c | Semaphore + Mutex | 生產者-消費者同步 |
| dining_philosophers.c | Mutex (交替順序) | Deadlock |

### Key Takeaways

1. **Race Condition**：多執行緒非同步訪問共享資源，採用 Mutex 保護
2. **Deadlock**：循環等待locks造成的阻塞，採用固定順序或交替順序避免
3. **Semaphore**：適合生產者-消費者問題的信號量同步

