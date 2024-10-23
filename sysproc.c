#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

extern struct proc* findproc(int pid);  // Declaration for findproc
extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;  // Declare ptable for process table access

extern int sched_policy;

int sys_fork(void) {
  return fork();
}

int sys_exit(void) {
  exit();
  return 0;  // not reached
}

int sys_wait(void) {
  return wait();
}

int sys_kill(void) {
  int pid;  // Declare pid here

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void) {
  return myproc()->pid;
}

int sys_sbrk(void) {
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void) {
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (myproc()->killed) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

int sys_uptime(void) {
  uint xticks;
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_shutdown(void) {
  outw(0xB004, 0x0 | 0x2000); // old qemu
  outw(0x604, 0x0 | 0x2000);  // newer qemu
  return 0;
}

extern int sched_trace_enabled;
extern int sched_trace_counter;

int sys_enable_sched_trace(void) {
  if (argint(0, &sched_trace_enabled) < 0) {
    cprintf("enable_sched_trace() failed!\n");
  }

  sched_trace_counter = 0;
  return 0;
}

int sys_fork_winner(void) {
  int winner;

  if (argint(0, &winner) < 0)
    return -1;

  if (winner != 0 && winner != 1)
    return -1; 

  myproc()->fork_policy = winner; 
  return 0;
}

int sys_set_sched(void) {
  int sched_type;

  if (argint(0, &sched_type) < 0)
    return -1;

  sched_policy = sched_type; 
  return 0;
}

int sys_transfer_tickets(void) {
    int pid, tickets;
    struct proc *p, *curr;

    if (argint(0, &pid) < 0 || argint(1, &tickets) < 0)
        return -1;
    if (tickets <= 0)
        return -1;

    curr = myproc();
    if (tickets >= curr->tickets)
        return -2; 

    p = findproc(pid);
    if (!p)
        return -3;

    curr->tickets -= tickets;
    p->tickets += tickets;

    p->stride = (STRIDE_TOTAL_TICKETS * 10) / p->tickets;
    curr->stride = (STRIDE_TOTAL_TICKETS * 10) / curr->tickets;

    return curr->tickets;
}


int sys_tickets_owned(void) {
    int pid;
    if (argint(0, &pid) < 0)
        return -1;

    struct proc *p = findproc(pid);
    if (p)
        return p->tickets;
    return -1;  
}


