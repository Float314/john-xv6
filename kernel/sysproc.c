#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_nice(void)
{
  int n;
  argint(0, &n);
  struct proc *p = myproc();
  acquire(&p->lock);
  p->priority += n;
  if (p->priority < 0)
    p->priority = 0;
  if (p->priority > 20)
    p->priority = 20;
  release(&p->lock);
  return 0;
}

uint64
sys_renice(void)
{
  int pid, priority;
  struct proc *p;

  argint(0, &pid);
  argint(1, &priority);

  if (priority < 0)
    priority = 0;
  if (priority > 20)
    priority = 20;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->pid == pid) {
      p->priority = priority;
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

uint64
sys_meminfo(void)
{
  return kfree_mem();
}

uint64
sys_getprocs(void)
{
  uint64 addr;
  int max;
  argaddr(0, &addr);
  argint(1, &max);

  struct procinfo info;
  struct proc *p;
  int count = 0;

  for (p = proc; p < &proc[NPROC] && count < max; p++) {
    acquire(&p->lock);
    if (p->state != UNUSED) {
      info.pid = p->pid;
      info.ppid = p->parent ? p->parent->pid : 0;
      safestrcpy(info.name, p->name, sizeof(info.name));
      info.state = p->state;
      info.priority = p->priority;
      release(&p->lock);

      if (copyout(myproc()->pagetable, myproc()->sz,
                  addr + count * sizeof(struct procinfo), (char *)&info,
                  sizeof(struct procinfo)) < 0)
        return -1;
      count++;
    } else {
      release(&p->lock);
    }
  }
  return count;
}

uint64
sys_shutdown(void) 
{
  volatile uint32 *p = (volatile uint32 *) TEST;
  *p = 0x5555;
  return 0;
}
