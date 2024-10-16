#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

int main() {
    // 打开两个文件用于输出
    close(1);//关闭标准输出 
    int fd1 = open("dup_output.txt", O_WRONLY | O_CREATE);
    if (fd1 < 0) {
        printf("Error: Failed to open dup_output.txt\n");
        exit(1);
    }

    // 使用 dup2 重定向标准输出到第一个文件 dup_output.txt
    dup2(fd1, 1);  // 1 表示标准输出 stdout
    printf("This text is written to dup_output.txt using dup2.\n");
    printf("This demonstrates redirecting standard output to a file.\n");

    // 关闭第一个文件描述符
    close(fd1);

    int fd2 = open("dup2_output.txt", O_WRONLY | O_CREATE);
    if (fd2 < 0) {
        printf("Error: Failed to open dup2_output.txt\n");
        exit(1);
    }

    // 使用 dup2 重定向标准输出到第二个文件 dup2_output.txt
    dup2(fd2, fd1);
    printf("This text is written to dup2_output.txt using dup2.\n");
    printf("This demonstrates the second redirection using dup2.\n");

    // 关闭第二个文件描述符
    close(fd2);

    // 完成后返回
    exit(0);
}