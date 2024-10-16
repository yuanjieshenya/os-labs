#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/* Possible states of a thread: */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  8192
#define MAX_THREAD  7  // 支持新线程

int current_time = 0; // 模拟系统当时时间

// 定义存寄存器的结构
struct context {
    uint64 ra;
    uint64 sp;
    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};

// 修改 PCB 表的结构
struct thread {
    struct context context;          // 存寄存器的结构
    char stack[STACK_SIZE]; /* 线程的栈 */
    int state;             /* FREE, RUNNING, RUNNABLE */
    int id;                // 队列 id
    int enter_time;       // 进入时间
    int run_time;         // 运行时间
};

struct thread all_thread[MAX_THREAD]; // PCB 表
struct thread *current_thread;
extern void thread_switch(uint64, uint64);

void thread_init(void) {
    current_thread = &all_thread[0];
    current_thread->state = RUNNING;
}

void thread_schedule(void) {
    struct thread *next_thread = 0;
    float max_response_ratio = -1.0; // 初始化最大响应比

    // 遍历所有线程，选择合适的线程调度
    for (int i = 0; i < MAX_THREAD; i++) {
        struct thread *t = &all_thread[i];
        // 仅考虑可运行状态且到达的线程
        if (t->state == RUNNABLE && t->enter_time <= current_time) {
            // 计算响应比
            int waiting_time = current_time - t->enter_time; // 等待时间
            float response_ratio = (waiting_time + t->run_time) / (float)t->run_time; // 响应比

            // 如果该线程的响应比大于当前最大响应比，则选择该线程
            if (response_ratio > max_response_ratio) {
                max_response_ratio = response_ratio;
                next_thread = t;
            }
        }
    }

    if (next_thread == 0) {
        // 如果没有可运行线程，直接返回
        printf("thread_schedule: no runnable threads\n");
        exit(-1);
    }

    if (current_thread != next_thread) {
        /* 切换线程 */
        next_thread->state = RUNNING;
        struct thread *prev_thread = current_thread;
        current_thread = next_thread;
        thread_switch((uint64)prev_thread, (uint64)current_thread);
    }
}

void thread_create(void (*func)(), int id, int enter_time, int run_time) {
    struct thread *t;

    for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
        if (t->state == FREE) {
            break;
        }
    }

    if (t == all_thread + MAX_THREAD) {
        // 超出最大线程数
        printf("Error: Maximum thread limit reached\n");
        return;
    }

    t->state = RUNNABLE;
    t->id = id;
    t->context.sp = (uint64)((char*)&t->stack + STACK_SIZE); // sp 在栈顶
    t->context.ra = (uint64)func;
    t->enter_time = enter_time; // 设置进入时间
    t->run_time = run_time; // 设置运行时间
}

// 示例线程函数
void thread_example(void) {
    printf("Thread with id %d starts\n", current_thread->id);
    printf("Last mem time is %d\n", current_time);
    
    // 模拟运行时间
    for (int i = 0; i < current_thread->run_time; i++) {
        sleep(1); // 模拟运行时间
    }
    
    current_time += current_thread->run_time; // 增加当前线程的运行时间到系统时间
    current_thread->state = FREE; // 完成后设置为 FREE
    thread_schedule();
}

int main(int argc, char *argv[]) {
    thread_init();
    
    // 创建线程，设置不同的进入时间和运行时间
    thread_create(thread_example, 1, 0, 20); // 线程第1个到，进入时间为0，运行时间20
    thread_create(thread_example, 2, 0, 10); // 线程第2个到，进入时间为0，运行时间10
    thread_create(thread_example, 3, 5, 15); // 线程第3个到，进入时间为5，运行时间15
    thread_create(thread_example, 4, 10, 5); // 线程第4个到，进入时间为10，运行时间5
    thread_create(thread_example, 5, 15, 30); // 线程第5个到，进入时间为15，运行时间30
    thread_create(thread_example, 6, 20, 10); // 线程第6个到，进入时间为20，运行时间10
    thread_schedule();
    
    exit(0);
}
