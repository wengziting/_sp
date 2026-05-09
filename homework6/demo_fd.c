#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

void print_separator(const char *title) {
    printf("\n");
    printf("========================================\n");
    printf("%s\n", title);
    printf("========================================\n");
}

int main(void) {
    const char *filename = "demo_test.txt";
    char buffer[256];

    print_separator("1. 寫入檔案 (使用 open/write)");

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }
    printf("開啟檔案成功，FD = %d\n", fd);
    printf("（注意：0,1,2 已被 stdin/stdout/stderr 佔用，所以是 3）\n");

    const char *msg = "Hello, Unix!\n這是使用 write() 寫入的資料。\n";
    ssize_t n = write(fd, msg, strlen(msg));
    printf("寫入 %zd 位元組到檔案\n", n);

    close(fd);
    printf("關閉 FD %d\n", fd);

    print_separator("2. 讀取檔案 (使用 open/read)");

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }
    printf("以唯讀模式開啟，FD = %d\n", fd);

    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("讀取 %zd 位元組：\n%s", bytes, buffer);
    }

    close(fd);

    print_separator("3. 觀察 FD 分配順序");

    printf("先關閉 stdout (FD 1)...\n");
    close(1);

    int new_fd = open(filename, O_RDONLY);
    printf("新開啟檔案的 FD = %d\n", new_fd);
    printf("（因為 FD 1 剛關閉，所以新 FD 就是 1）\n");

    
    printf("這行會寫入 demo_test.txt 而不是螢幕！\n");

    close(new_fd);

    printf("\n程式結束。\n");

    return 0;
}
