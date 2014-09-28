#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"
#include "sysfunc.h"

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
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
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
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_reserve(void)
{	
	syspstat->inuse[0] = 666;
	syspstat->pid[0] = 666;	
	return (int)&syspstat;
}

int sys_spot(void)
{
	return 0;
}

int sys_getpinfo(void)
{
	// Declare variables
	int userpstat_loc;
	struct pstat *userpstat;
	
	// Retrieve the memory location of user pstat struct from the stack
	if(argint(0, &userpstat_loc) < 0)
	{
    	return -1;
    }
	
	// Set up the pointer to the user pstat location
	userpstat = (struct pstat*)userpstat_loc;
	
	
	
	// Now populate the user pstat with data from the system pstat
	int i;
	for(i = 0; i < NPROC; i ++)
	{
		userpstat->inuse[i] 	= syspstat->inuse[i];
		userpstat->pid[i] 		= syspstat->pid[i];
		userpstat->chosen[i] 	= syspstat->chosen[i];
		userpstat->time[i] 		= syspstat->time[i];
		userpstat->charge[i] 	= syspstat->charge[i];
	}
	
	// All done, exit
	return (int)&syspstat;
}
