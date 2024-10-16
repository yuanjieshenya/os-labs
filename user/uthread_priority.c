#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/* Possible states of a thread: */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  8192
#define MAX_THREAD  7  //支持新线程

int current_time = 0;//模拟系统当时时间

//定义一个存寄存器的结构
struct context{
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

//修改一下PCB表的结构，这样后续代码能更清晰，汇编代码.S也可以写得足够简洁
struct thread {
  struct context context;          //存寄存器的结构
  char       stack[STACK_SIZE]; /* the thread's stack */
  int        state;             /* FREE, RUNNING, RUNNABLE */
  int priority;  // Add priority field
  int enter_time;//进入时间
  int run_time;//运行时间
};
struct thread all_thread[MAX_THREAD];//PCB表
struct thread *current_thread;
extern void thread_switch(uint64, uint64);
              
void 
thread_init(void)
{
  // main() is thread 0, which will make the first invocation to
  // thread_schedule().  it needs a stack so that the first thread_switch() can
  // save thread 0's state.  thread_schedule() won't run the main thread ever
  // again, because its state is set to RUNNING, and thread_schedule() selects
  // a RUNNABLE thread.
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

void thread_schedule(void) {
    struct thread *next_thread = 0;
    int highest_priority = -1;

    // 遍历所有线程，选择合适的线程调度
    for (int i = 0; i < MAX_THREAD; i++) {
        struct thread *t = &all_thread[i];
        // 仅考虑可运行状态且到达的线程
        if (t->state == RUNNABLE && t->enter_time <= current_time) {
            if (t->priority > highest_priority) {
                highest_priority = t->priority;
                next_thread = t;
                printf("change thread! current thread priority is %d\n",highest_priority);
            }
        }
    }

    if (next_thread == 0) {
        printf("thread_schedule: no runnable threads\n");
        exit(-1);
    }

    if (current_thread != next_thread) {
        /* switch threads? */
        next_thread->state = RUNNING;
        struct thread *prev_thread = current_thread;
        current_thread = next_thread;
        thread_switch((uint64)prev_thread, (uint64)current_thread);
    }
}

void 
thread_create(void (*func)(), int priority,int enter_time,int run_time)
{
  struct thread *t;

  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) {
      break;
    };
  }
  // YOUR CODE HERE
  t->state = RUNNABLE;
  t->priority = priority;
  t->context.sp = (uint64)((char*)&t->stack + STACK_SIZE);//sp在栈顶
  t->context.ra = (uint64)func;
  t->enter_time = enter_time; // 设置进入时间
  t->run_time = run_time; // 设置运行时间
}

// 示例线程函数
void thread_example(void) {
    printf("Thread with priority %d\n", current_thread->priority);
    printf("Current time is %d\n",current_time);
    for (int i = 0; i <= current_thread->run_time; i++) {
        sleep(1); // 模拟运行时间
    }
    current_time += current_thread->run_time; // 增加当前线程的运行时间到系统时间
    current_thread->state = FREE; // 完成后设置为 FREE
    thread_schedule();
}

int all_threads_done(void) {
    // 检查所有线程是否都已完成
    for (int i = 0; i < MAX_THREAD; i++) {
        if (all_thread[i].state != FREE) {
            return 0; // 还有线程未完成
        }
    }
    return 1; // 所有线程都已完成
}

int 
main(int argc, char *argv[]) 
{
  thread_init();
   // 创建线程，设置不同的进入时间和运行时间
  thread_create(thread_example, 1, 0, 10); // 线程1，进入时间为0，运行时间10
  thread_create(thread_example, 2, 0, 20); // 线程2，进入时间为0，运行时间20
  thread_create(thread_example, 3, 5, 15); // 线程3，进入时间为5，运行时间15
  thread_create(thread_example, 4, 10, 5); // 线程4，进入时间为10，运行时间5
  thread_create(thread_example, 5, 15, 30); // 线程5，进入时间为15，运行时间30
  thread_create(thread_example, 6, 20, 10); // 线程6，进入时间为20，运行时间10
  thread_schedule(); 
  exit(0);
}
