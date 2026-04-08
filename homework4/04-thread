# 執行緒（Thread）

## 執行緒概念

執行緒是程序內部的執行單位，多個執行緒共享同一程序的位址空間和資源。

## POSIX 執行緒範例

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *thread_function(void *arg) {
    int id = *(int *)arg;
    printf("執行緒 %d 正在執行\n", id);
    return NULL;
}

int main() {
    pthread_t threads[3];
    int ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, thread_function, &ids[i]) != 0) {
            perror("pthread_create failed");
            return 1;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("所有執行緒已完成\n");
    return 0;
}
```

## 執行緒 vs 程序

| 特性 | 程序 | 執行緒 |
|------|------|--------|
| 資源 | 獨立位址空間 | 共享位址空間 |
| 建立速度 | 較慢 | 較快 |
| 溝通 | IPC | 直接記憶體共享 |
| 穩定性 | 較高（隔離） | 較低（共享風險） |
