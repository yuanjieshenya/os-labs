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
  int id;  // Add queue id
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
    int first_id = MAX_THREAD;

    // 遍历所有线程，选择合适的线程调度
    for (int i = 0; i < MAX_THREAD; i++) {
        struct thread *t = &all_thread[i];
        // 仅考虑可运行状态且到达的线程
        if (t->state == RUNNABLE && t->enter_time <= current_time) {
            if (t->id <= first_id) {//比较线程到达顺序，先到先服务
            //如果存在多个进程同时到的话比较运行时间长短，时间短的进程先执行
                if(t->id == first_id){
                    if(t->run_time > next_thread->run_time){
                        continue;//直接跳过
                    }
                    else{
                        printf("The shorter thread is selected to be served!");
                        first_id = t->id;
                        next_thread = t;
                        printf("change thread! current thread id is %d, runtime is %d\n",first_id, next_thread->run_time);
                    }
                }
                else{
                    first_id = t->id;
                    next_thread = t;
                    printf("change thread! current thread id is %d, runtime is %d\n",first_id, next_thread->run_time);}
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
thread_create(void (*func)(), int id,int enter_time,int run_time)
{
  struct thread *t;

  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) {
      break;
    };
  }
  // YOUR CODE HERE
  t->state = RUNNABLE;
  t->id = id;
  t->context.sp = (uint64)((char*)&t->stack + STACK_SIZE);//sp在栈顶
  t->context.ra = (uint64)func;
  t->enter_time = enter_time; // 设置进入时间
  t->run_time = run_time; // 设置运行时间
}

// 示例线程函数
void thread_example(void) {
    printf("Thread with id %d\n", current_thread->id);
    printf("Last mem time is %d\n",current_time);
    for (int i = 0; i <= current_thread->run_time; i++) {
        sleep(1); // 模拟运行时间
    }
    current_time += current_thread->run_time; // 增加当前线程的运行时间到系统时间
    current_thread->state = FREE; // 完成后设置为 FREE
    thread_schedule();
}

int 
main(int argc, char *argv[]) 
{
  thread_init();
   // 创建线程，设置不同的进入时间和运行时间
  thread_create(thread_example, 1, 0, 20); // 线程第1个到，进入时间为0，运行时间20
  thread_create(thread_example, 1, 0, 10); // 线程第1个到，进入时间为0，运行时间10
  thread_create(thread_example, 3, 5, 15); // 线程第3个到，进入时间为5，运行时间15
  thread_create(thread_example, 4, 10, 5); // 线程第4个到，进入时间为10，运行时间5
  thread_create(thread_example, 5, 15, 30); // 线程第5个到，进入时间为15，运行时间30
  thread_create(thread_example, 6, 20, 10); // 线程第6个到，进入时间为20，运行时间10
  thread_schedule(); 
  exit(0);
}