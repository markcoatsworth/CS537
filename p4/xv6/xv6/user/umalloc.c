#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void
free(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  free((void*)(hp + 1));
  return freep;
}

void*
malloc(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    if(p == freep)
      if((p = morecore(nunits)) == 0)
        return 0;
  }
}


// Wrap the clone function.
// In the child, thread_create should call the function pointer, passing it the provided argument
// When the provided function returns, thread_create should free the stack and call exit
int thread_create(void (*fn) (void *), void *arg)
{
	// Allocate two pages of memory for the stack.
	// We cannot guarantee this is page aligned, so need to grab some extra space.
	// We'll align it to the pages during the clone() system call.
	int* ThreadStack;
	ThreadStack = (int*)malloc(1024 * sizeof(int*));
	printf(1, "[thread_create] called, &fn=0x%x, ThreadStack=0x%x\n", fn, ThreadStack);

	printf(1, "\n\n[thread_create] Process table before clone call:\n");
	unlock(0);

	// Now call the clone function to start the new thread	
	int NewID = clone(ThreadStack);
	printf(1, "[thread_create] NewID=%d\n", NewID);

	printf(1, "\n\n[thread_create] Process table after clone call:\n");
	unlock(0);



	// NewID will be 0 for the thread, and original PID for the parent process
	if(NewID == 0)
	{
		printf(1, "[thread_create(%d)] Thread here\n", getpid());
		fn(arg);
		exit();
	}
	else
	{
		printf(1, "[thread_create(%d)] Parent process here\n", getpid());

	}
	
	
	return NewID;
}
