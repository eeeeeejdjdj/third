#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

// Producer-Consumer Buffer
#define BUF_SIZE 5
struct {
  struct spinlock lock;
  int data[BUF_SIZE];
  int count;
  int head;
  int tail;
  int init;
} pc_buf;

void init_pc_buf() {
  if (pc_buf.init == 0) {
    initlock(&pc_buf.lock, "pc_buf");
    pc_buf.count = 0;
    pc_buf.head = 0;
    pc_buf.tail = 0;
    pc_buf.init = 1;
  }
}

int sys_produce(void) {
  int val;
  if(argint(0, &val) < 0) return -1;
  
  init_pc_buf();
  acquire(&pc_buf.lock);
  
  while (pc_buf.count == BUF_SIZE) {
    sleep(&pc_buf.count, &pc_buf.lock); // Wait if full
  }
  
  pc_buf.data[pc_buf.tail] = val;
  pc_buf.tail = (pc_buf.tail + 1) % BUF_SIZE;
  pc_buf.count++;
  
  cprintf("[Kernel] Produced %d, count: %d\n", val, pc_buf.count);
  
  wakeup(&pc_buf.count); // Wake up consumers
  release(&pc_buf.lock);
  return 0;
}

int sys_consume(void) {
  int val;
  
  init_pc_buf();
  acquire(&pc_buf.lock);
  
  while (pc_buf.count == 0) {
    sleep(&pc_buf.count, &pc_buf.lock); // Wait if empty
  }
  
  val = pc_buf.data[pc_buf.head];
  pc_buf.head = (pc_buf.head + 1) % BUF_SIZE;
  pc_buf.count--;
  
  cprintf("[Kernel] Consumed %d, count: %d\n", val, pc_buf.count);
  
  wakeup(&pc_buf.count); // Wake up producers
  release(&pc_buf.lock);
  return val;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_hello(void)
{
  cprintf("Hello, xv6!\n");
  return 0;
}
