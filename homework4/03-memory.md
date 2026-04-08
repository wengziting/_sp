# 記憶體管理

## 虛擬記憶體

作業系統為每個程序提供虛擬位址空間，將虛擬位址映射到實體記憶體。

## 記憶體配置

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    // 動態記憶體配置
    int *arr = (int *)malloc(10 * sizeof(int));
    
    if (arr == NULL) {
        fprintf(stderr, "記憶體配置失敗\n");
        return 1;
    }
    
    for (int i = 0; i < 10; i++) {
        arr[i] = i * i;
    }
    
    printf("配置的記憶體位址: %p\n", (void *)arr);
    
    free(arr);
    return 0;
}
```

## 程序位址空間佈局

| 區段 | 說明 |
|------|------|
| Text | 程式碼 |
| Data | 已初始化的全域/靜態變數 |
| BSS | 未初始化的全域/靜態變數 |
| Heap | 動態配置記憶體（向上增長） |
| Stack | 函數呼叫、區域變數（向下增長） |
