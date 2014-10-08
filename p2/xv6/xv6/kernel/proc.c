#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "pstat.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

static int rnd_seed;

int SchedulerCycles = 0;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Initialize the level, bid and percent
  p->level = 2;
  p->percent = 0;
  p->bid = 0;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  
  // Also create an entry in the pstat table for this process. Take first slot that is not in use.
  for(i = 0; i < NPROC; i++)
  {
  	if(syspstat->inuse[i] == 0)
  	{
  	  syspstat->inuse[i] 	= 1;
  	  syspstat->pid[i] 		= pid;
  	  safestrcpy(syspstat->pname[i], np->name, sizeof(np->name));
	  syspstat->level[i]	= 2;
	  syspstat->percent[i]	= 0;
	  syspstat->bid[i]		= 0;
  	  syspstat->chosen[i] 	= 0;
  	  syspstat->time[i]		= 0;
  	  syspstat->charge[i] 	= 0;
  	  break;
  	}
  }
  //cprintf("Added new process, np->pid=%d, np->name=%s, proc->pid=%d, proc->name=%s\n", np->pid, np->name, proc->pid, proc->name);
  
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
      
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

/// Do not call the scheduler code! It should be allowed to work and run all on its own.

void scheduler(void)
{
	int LotterySeed = 1; // we don't need a truly random seed
	int LotteryCounter;
	int LotteryWinningTicket = 0;
	int HighestBid;

	struct proc *p;
	struct proc *HighestBidProcess = NULL;
	struct proc *SelectedProcess = NULL;
	struct proc *RandomProcess = NULL;
	
	// Seed the random number generator
	set_rnd_seed(LotterySeed);
	
	// Turn on statistics display for now. Users can turn it on with a system call.
	DisplayStatistics = 0;
	
	// Allocate memory for the pstat process table, and initialize it all to 0
	syspstat = (struct pstat*)kalloc();
	memset(syspstat, 0, sizeof(struct pstat));
	
	// The random number generator takes some time to get seeded
	// Do not start the scheduler until it returns real values
	while(LotteryWinningTicket == 0)
	{
		LotteryWinningTicket = rand_int() % 100;
	}
	
	// Start the main scheduler loop
	for(;;)
	{
  		// Increment the scheduler cycle count -- we'll use this later for generating statistics
  		SchedulerCycles++;
  
	    // Enable interrupts on this processor.
	    sti();
	
	    // Loop over process table looking for process to run.
	    acquire(&ptable.lock);
	    
	    // Reset everything
	    LotteryCounter = 0;
	    HighestBid = 0;
	    HighestBidProcess = 0;
	    RandomProcess = 0;
	    SelectedProcess = 0;
	    
	    // Run the lottery for this round
	    LotteryWinningTicket = rand_int() % 100;
	    
	    // Iterate through all processes in the system. Set the SelectedProcess pointer based on lottery scheduling policy.
	    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
	    {
	    	/*
	    	if(DisplayStatistics == 1)
	    	{
	    		cprintf("[scheduler] loop: cpu=%d, p=%d, name=%s, percent=%d, p->state=%d, LotteryWinningTicket=%d\n", cpu->id, p, p->name, p->percent, p->state, LotteryWinningTicket);
	    	}
	    	*/
	    
			// This is where it checks if the process state is runnable; if not, it just continues the loop
			if(p->state != RUNNABLE)
				continue;
				
			// Keep track of the process with the highest bid. We'll need to know this later if nobody wins the lottery.
			// If multiple processes tie for highest bid, it gets awarded to the first one in the process table
			// which also happens to be the first process started.
			if(p->bid > HighestBid)
			{
				HighestBid = p->bid;
				HighestBidProcess = p;
			}

			// We know this process is runnable. Now set the lottery counter. Assume reserved time is shared between all cpus.
			LotteryCounter += (p->percent / ncpu);
			
			// If we get a lottery winner, set the proc and SelectedProcess pointers, then bail out of the for loop.
			if(LotteryCounter > LotteryWinningTicket)
			{
				//cprintf("[scheduler] LotteryWinningTicket=%d, %s won the lottery on cpu %d\n", LotteryWinningTicket, p->name, cpu->id);
				proc = p;
				SelectedProcess = p;
				break;
			}
			
			// Finally, set up a random pointer in case nothing gets selected via lottery or highest bid
			if(RandomProcess == NULL)
			{
				RandomProcess = p;
			}
			else
			{
				if(rand_int() % 2 == 0)
				{
					RandomProcess = p;
				}
			}
		}
		
		// If we got to the end of the process table without a process selected, pick the spot process with the highest big
		if(SelectedProcess == NULL)
		{
			//cprintf("Nobody won the lottery, picking spot process with highest bid.\n");
			if(HighestBidProcess != NULL)
			{
				proc = HighestBidProcess;
				SelectedProcess = HighestBidProcess;				
			}
			else
			{
				//cprintf("Nobody had a highest bid, picking a random process.\n");
				proc = RandomProcess;
				SelectedProcess = RandomProcess;
			}
		}
		
		// Make sure that SelectedProcess is not null before proceeding!
		// If it is still set to null, then no processes are ready and we start the scheduler loop over again. 
		if(SelectedProcess != NULL)	
		{	
			//cprintf("[scheduler] p=%d, proc=%d, SelectedProcess=%d\n", p, proc, SelectedProcess);
			cprintf("[scheduler] Going to run %s [pid %d] on cpu %d! Scheduler cycle %d\n", SelectedProcess->name, SelectedProcess->pid, cpu->id, ScheduleCycles);
			
			/// Make sure we use the correct stack for this process
			/// We need to make sure this is set up correctly before we can start executing this process
			switchuvm(SelectedProcess);
			
			/// Now set the process state to running -- this is the point where we decide to run this process
			SelectedProcess->state = RUNNING;
			
			// Now identify the matching entry in the system pstat table, and update it
			int i;
			for(i = 0; i < NPROC; i ++)
			{
				if(syspstat->pid[i] == SelectedProcess->pid)
				{
					syspstat->chosen[i]++;
					syspstat->charge[i] += (10 * syspstat->bid[i]);
					syspstat->time[i] += 10;
					// Update the process name in the pstat table. This is such a cheap hack, but I'll remove it after debugging...
					safestrcpy(syspstat->pname[i], SelectedProcess->name, sizeof(SelectedProcess->name));
					//cprintf("[scheduler] cycles=%d, selected process=%s, chosen=%d\n", SchedulerCycles, syspstat->pname[i], syspstat->chosen[i]);					
				}
			}
			
			/// Now we have to move from the kernel context to the user content
			/// This next call basically says to save the CPU scheduled context, and switch to the process context
			/// This is done with assembly line code in swtch.S
			/// This is sort of the same as doing an exec. swtch never returns! It stops running this particular code, then switches over to the new process
			/// Note: the cpu variable is a global in proc.h
			swtch(&cpu->scheduler, SelectedProcess->context);
			
			/// Tbe following line is what happens after it gets switched back via timer interrupt and yield
			/// It gets called back in /kernel/trap.c
			switchkvm();
			
			// Process is done running for now.
			// It should have changed its p->state before coming back.
			proc = 0;
		}				
			
		// Release the lock on the process table
		release(&ptable.lock);
		
		// Finally, if scheduler statistics is turned on, display some stats every 1000000 scheduler cycles
		// Since data is aggregated for all cpus, only display the results for CPU id=0. Otherwise we get lots of duplicate entries.
		// Display data in a comma-delimited format that can easily be imported into a graphing tool
		if(DisplayStatistics == 1)
		{
			if((int)SchedulerCycles % 100000 == 0)
			{
				cprintf("\n%d", SchedulerCycles);

				int i;
				for(i = 0; i < NPROC; i ++)
				{
					if(syspstat->inuse[i] == 1)
					{
						cprintf(",%d", syspstat->chosen[i]);
					}
				}			
			}
		}
	}
}


// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  /// All of the following assumptions should hold! If they don't, panic and explain what went wrong
  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  /// The arguments are reversed from in the previous function
  /// Switches back to the CPU scheduler
  swtch(&proc->context, cpu->scheduler);
  
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
/// Changes the state on this process from running to runnable
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

// Reserve time for a process
int proc_reserve(int pid, int percent)
{
	// Declare variables
	int MaxCpuTime = 100 * ncpu;
	int ReservedCpuTime = 0;
    struct proc* p;
	
    // First make sure this will not exceed maximum allowable cpu time. Only count against active processes!
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
	{
    	if(p->state == SLEEPING || p->state == RUNNABLE || p->state == RUNNING)
    	{
    		ReservedCpuTime += p->percent;
	    }
    }

	// Reserve cpu time on the process
    if((ReservedCpuTime + percent) <= MaxCpuTime)
    {
    	// Add reservation in the pstat table
    	int i;
    	for(i = 0; i < NPROC; i ++)
    	{
    		if(syspstat->pid[i] == pid)
    		{
		    	syspstat->level[i] = 1;
		    	syspstat->percent[i] = percent;
		    	syspstat->bid[i] = 100;   
		    	break; 		
    		}
    	}
    	
    	// Also add to the main process table (to save lookup time in the scheduler, at the expense of memory)
		proc->level = 1;
		proc->percent = percent;
		proc->bid = 100;
    }
    else
    {
    	return 1;
    }
    
    return 0;
}

// Pseudo random number generator by Christian Pinder
// http://www.christianpinder.com/articles/pseudo-random-number-generation/

void set_rnd_seed(int new_seed)
{
    rnd_seed = new_seed;
}

int rand_int(void)
{
    int k1;
    int ix = rnd_seed;
	
    k1 = ix / 127773;
    ix = 16807 * (ix - k1 * 127773) - k1 * 2836;
    if (ix < 0)
        ix += 2147483647;
    rnd_seed = ix;
    return rnd_seed;
}

