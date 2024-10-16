#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

int main(){
    int fd[2] = {0};//创建共享管道
    int n = pipe(fd);
    if(n < 0)
        return 1;//管道创建失败，则直接结束程序
    int pid = fork();//创建子进程
    if(pid < 0)//创建失败结束程序
        return 2;
    if(pid == 0)//子进程，发起通信
    {
        close(fd[0]);
        int nums[5] = {1, 2, 3, 4, 5};
        printf("Producer: Sending numbers...\n");
        write(fd[1], nums, sizeof(nums));
        for(int i = 1; i <= 5; i++){
            printf("Producer: Sent %d\n", i);
        }
        close(fd[1]);
        exit(0);
    }
    else{//父进程，接受通信信息
        wait(0);//等待子进程输出完成，防止输出错乱
        close(fd[1]);
        int nums[5];
        printf("Consumer: Receiving numbers...\n");
        read(fd[0], nums, sizeof(nums));
        for(int i = 1; i <= 5; i++){
            printf("Consumer: Received %d\n", i);
        }
        close(fd[0]);

        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += nums[i];
        }
        printf("Consumer:Total sum = %d\n", sum);
    }
    return 0;
}
