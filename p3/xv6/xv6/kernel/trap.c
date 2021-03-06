#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "syscall.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

void
trap(struct trapframe *tf)
{
	if(tf->trapno == T_PGFLT)
	{
		uint RequestedAddress = rcr2();
		uint NextPageBound = USERTOP - proc->stacksz - PGSIZE;
		//cprintf("[trap] Page fault at address 0x%x, proc->name=%s, proc->stacksz=%d, proc->tf->eip=0x%x\n", rcr2(), proc->name, proc->stacksz, proc->tf->eip);
		if(RequestedAddress >= NextPageBound && RequestedAddress < USERTOP)
		{
			//cprintf("[trap] Requested address is within reach of the stack! RequestedAddress=0x%x, NextPageBound=0x%x\n", RequestedAddress, NextPageBound);
			if(RequestedAddress > (proc->sz + (proc->sz % PGSIZE) + PGSIZE))
			{
				//cprintf("[trap] Requested address is far enough from the heap. Allocate @ NextPageBound=0x%x\n", NextPageBound);
				allocuvm(proc->pgdir, NextPageBound, NextPageBound + PGSIZE);
				proc->stacksz += PGSIZE;
				//cprintf("[trap] Now proc->stacksz=%d. Revive the process proc->name=%s! proc->tf->eip=0x%x, proc->tf->eax=%d\n", proc->stacksz, proc->name, proc->tf->eip, proc->tf->eax);
				proc->tf->trapno = T_SYSCALL;				
				proc->tf->eax = 3;				
				switchuvm(proc);
				//swtch(&cpu->scheduler, proc->context);	
      	//switchkvm();
				
			}
		}	
		/// Add code to gracefully handle this
		/// Check what address it is faulting on
		/// And make sure that you allocate the page
		/// Don't allocate the page if it's not near the stack
		/// If we're segfaulting because we're trying to access a totally bogus page (ie 0) then still page fault
		/// Should only allocate if we're getting the page fault close to the end of the stack
		/// Also keep in mind, the stack can grow and ocllide with the heap
		/// So we should allocate a "null" page between the stack and heap. We always want to have one null page between them.
	}
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpu->id, tf->cs, tf->eip);
    lapiceoi();
    break;
   
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip 0x%x (cr2=0x%x)\n",
              tf->trapno, cpu->id, tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
	/// This part is extremely important to us.
	cprintf("[trap] A process in user space misbehaved!\n");
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x (%d) addr 0x%x (%d) -> kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip, (int)tf->eip,
            rcr2(), rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running 
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}
