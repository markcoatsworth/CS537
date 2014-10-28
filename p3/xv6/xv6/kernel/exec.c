#include "types.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  /// In xv6, uses address space as a contiguous array of bytes
  /// However there is a problem because xv6 thinks that sz is the limit of the stack, if we try to go beyond it then it throws tantrums
  /// So in the new xv6, it's okay to go beyond the value in the sz variable so long as it's in our stack
  /// (might be some mistakes in that...)
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;

  if((ip = namei(path)) == 0)
    return -1;
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  /// Checking if the binary we're loading is in the correct format or not
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) < sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

	/// Setting up the kernel stack for this process
	/// In our project, we're not going to touch this at all -- don't need to modify this
  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
	/// Remember that your program code can have several sections
	/// This code is reading in your binary and looking at the offsets for your program headers
	/// Doing this for the number of program headers you can find in the binary, and loading the
	/// For each part of the program: let's load it into our process, one piece at a time
  sz = 0;
  
  // Allocate the first page of memory as a bogus page. This will make sure nothing useful goes in here. 
  if((sz = allocuvm(pgdir, sz, PGSIZE)) == 0)
      goto bad;
	cprintf("[exec] After allocating bogus page, sz=%d\n", sz);
  
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
	/// Take a region of the virtual address space and create mappings for it
	/// From size ph.va to ph.memsz, create mappings using allocuvm
	/// This function is crucial to this project!
	/// sz (size) must not be 0 if we want to have a segfault on the first page
	/// If we want to produce a segfault on the first page, we want to shift this forward
	/// The reason xv6 does not throw a segfault for address 0 is because it has allocated it
	//cprintf("[exec] pgdir=%x, sz=%d, ph.va=%d, ph.mymsz=%d\n", pgdir, ph.va, ph.memsz);
    if((sz = allocuvm(pgdir, sz, ph.va + ph.memsz)) == 0)
      goto bad;
	//cprintf("\t[exec] => sz=%d\n", sz);
	/// At this point we have memory in our virtual address space, we can read and write to it
	/// Now load the ph.va program header into our address space
    if(loaduvm(pgdir, (char*)ph.va, ip, ph.offset, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  ip = 0;

  // Allocate a one-page stack at the next page boundary
  /// In xv6, we only have one page for the stack
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + PGSIZE)) == 0)
    goto bad;

  // Push argument strings, prepare rest of stack in ustack.
  /// This loop just copies the argv array onto the user stack
  /// If we just relocated the stack (away from size, and towards some other address in the address space) then the rest of it should still work
  /// Because it doesn't really care where the stack is
  /// So we can relocate it, just make sure we allocate space there, and make sure the address pointer points there
  /// However this will still not allow us to grow the stack beyond a single page
  sp = sz;
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp -= strlen(argv[argc]) + 1;
    sp &= ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(proc->name, last, sizeof(proc->name));

  // Commit to the user image.
  /// Now we're done and switching to the new page table
  oldpgdir = proc->pgdir;
  proc->pgdir = pgdir;
  proc->sz = sz;
  proc->tf->eip = elf.entry;  // main
  proc->tf->esp = sp;
  /// Now the page table that we've carefully created gets used, and we free up the old one
  /// Note that we haven't done anything for the heap; is implicit that the heap starts after the stack
  switchuvm(proc);
  freevm(oldpgdir);

  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip)
    iunlockput(ip);
  return -1;
}
