#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void print_separator(const char *title) {
    printf("\n");
    printf("========================================\n");
    printf("%s\n", title);
    printf("========================================\n");
}

void demo_stdout_redirect(void) {
    printf("[測試] 將 stdout 重導向到檔案...\n");

    int fd = open("stdout_redirect.txt",
                 O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }

    /* dup2(fd, 1) 將 fd 複製到 FD 1 (stdout) */
    dup2(fd, STDOUT_FILENO);
    close(fd);  /* fd 已在 FD 1，不需要了 */

    printf("這行文字應該在檔案裡，不在螢幕上！\n");
    printf("這也是。\n");
    printf("確認重導向成功！\n");

    fflush(stdout);  /* 確保緩衝區內容寫入 */
}

void demo_stdin_redirect(void) {
    printf("\n[測試] 將 stdin 重導向從檔案讀取...\n");

    const char *input = "input_for_stdin.txt";
    FILE *f = fopen(input, "w");
    if (f) {
        fprintf(f, "第一行來自檔案\n");
        fprintf(f, "第二行也來自檔案\n");
        fprintf(f, "第三行最後一行\n");
        fclose(f);
    }

    int fd = open(input, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    dup2(fd, STDIN_FILENO);
    close(fd);

    char line[256];
    int line_num = 0;
    while (fgets(line, sizeof(line), stdin) != NULL) {
        line_num++;
        printf("讀到第 %d 行: %s", line_num, line);
    }
}

void demo_stderr_redirect(void) {
    printf("\n[測試] 將 stderr 重導向...\n");

    int fd = open("stderr_redirect.txt",
                 O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }

    /* 先恢復 stdout */
    fflush(stdout);

    dup2(fd, STDERR_FILENO);
    close(fd);

    fprintf(stderr, "這是錯誤訊息，寫入 stderr_redirect.txt\n");
    fprintf(stderr, "stderr 重導向測試完成\n");
}

void demo_multiple_redirect(void) {
    print_separator("同時重導向 stdin 和 stdout");

    const char *input_file = "multi_input.txt";
    FILE *f = fopen(input_file, "w");
    if (f) {
        fprintf(f, "3\n5\n7\n");
        fclose(f);
    }

    int in_fd = open(input_file, O_RDONLY);
    dup2(in_fd, STDIN_FILENO);
    close(in_fd);

    int out_fd = open("multi_output.txt",
                     O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(out_fd, STDOUT_FILENO);
    close(out_fd);

    int a, b, c;
    scanf("%d %d %d", &a, &b, &c);
    printf("總和 = %d\n", a + b + c);
    printf("乘積 = %d\n", a * b * c);
}

int main(void) {
    printf("=== dup2() I/O 重導向示範 ===\n\n");

    printf("檔案描述符對照表：\n");
    printf("  STDIN  (FD 0) - 標準輸入\n");
    printf("  STDOUT (FD 1) - 標準輸出\n");
    printf("  STDERR (FD 2) - 標準錯誤\n");
    printf("\ndup2(oldfd, newfd) 的作用：\n");
    printf("  將 oldfd 複製到 newfd\n");
    printf("  讓 newfd 指向與 oldfd 相同的檔案\n");

    print_separator("1. stdout 重導向");
    demo_stdout_redirect();

    print_separator("2. stdin 重導向");
    demo_stdin_redirect();

    print_separator("3. stderr 重導向");
    demo_stderr_redirect();

    print_separator("4. 同時重導向 stdin 和 stdout");
    demo_multiple_redirect();

    print_separator("測試結果");
    printf("檢查生成的檔案：\n\n");

    printf("--- stdout_redirect.txt ---\n");
    int fd = open("stdout_redirect.txt", O_RDONLY);
    if (fd >= 0) {
        char buf[1024];
        ssize_t n;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, n);
        }
        close(fd);
    }

    printf("\n--- stderr_redirect.txt ---\n");
    fd = open("stderr_redirect.txt", O_RDONLY);
    if (fd >= 0) {
        char buf[1024];
        ssize_t n;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, n);
        }
        close(fd);
    }

    printf("\n--- multi_output.txt ---\n");
    fd = open("multi_output.txt", O_RDONLY);
    if (fd >= 0) {
        char buf[1024];
        ssize_t n;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, n);
        }
        close(fd);
    }

    printf("\n程式結束。\n");
    return 0;
}
