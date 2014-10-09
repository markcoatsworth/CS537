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


// Mark a process as a reserved process (level 1), and guarantees 
// (probabilistically) the given portion of CPU time. 
int sys_reserve(void)
{	
	// Declare variables
	int percent;
	
	// Set the value of percent from the stack
	if(argint(0, &percent) < 0)
	{
    	return -1;
    }
    
    // Process fails if percent is not between 0 and 100
    if(percent < 0 || percent > 100)
    {
    	return -1;
    }
    
    // Reserve code needs to happen in proc.c which has access to the system process table. Run and return result.
    int success = proc_reserve(proc->pid, percent);
    return success;
}

// Mark a process for spot computing (level 2). 
// The bid is in nanodollars per millisecond of CPU time. 
int sys_spot(void)
{
	// Declare variables
	int bid;
	
	// Set the value of percent from the stack
	if(argint(0, &bid) < 0)
	{
    	return 1;
    }
    
    // Process fails if bid is less than 0
    if(bid < 0)
    {
    	return 1;
    }
    
    // The values of process level and bid price are in the main process table. Set them here.
    int i;
    for(i = 0; i < NPROC; i ++)
    {
    	if(proc->pid == syspstat->pid[i])
    	{
    		syspstat->level[i] = 2;
    		syspstat->bid[i] = bid;
    		// Also add to the main process table (to save lookup time in the scheduler, at the expense of memory)
			proc->level = 2;
			proc->bid = bid;
    	}
    }
    
    // Return success!
    return 0;
}

// Return some basic information about each active process, including its 
// process ID, how many times it has been chosen to run, how much time it 
// has run (ms), how much it has been charged (dollars).
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
	
	// Set a pointer to the user pstat location (which has already been allocated)
	userpstat = (struct pstat*)userpstat_loc;
	
	// Now populate the user pstat with data from the system pstat
	int i;
	for(i = 0; i < NPROC; i ++)
	{
		userpstat->inuse[i] 	= syspstat->inuse[i];
		userpstat->pid[i] 		= syspstat->pid[i];
		strncpy(userpstat->pname[i], syspstat->pname[i], 16);
		userpstat->level[i]		= syspstat->level[i];
	  	userpstat->percent[i]	= syspstat->percent[i];
	  	userpstat->bid[i]		= syspstat->bid[i];
		userpstat->chosen[i] 	= syspstat->chosen[i];
		userpstat->time[i] 		= syspstat->time[i];
		userpstat->charge[i] 	= syspstat->charge[i];
	}
	
	// All done, exit
	return 0;
}

// Turns process statistical output on or off.
int sys_pstats(void)
{
	int display;
	
	// Retrieve the memory location of display from the stack
	if(argint(0, &display) < 0)
	{
    	return -1;
    }
    
    // Turn statistics on or off
    DisplayStatistics = display;
    
    return 0;
}
