// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

static int refs[PXP(PHYSTOP)];
static struct spinlock refs_lock;

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&refs_lock, "pte_refs");
  acquire(&refs_lock);
  memset(refs, 0, sizeof(PXP(PHYSTOP)));
  release(&refs_lock);
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void kinc(void *pa) {
  /* printf("call kinc: refs=%d\n", refs[PXP(pa)]); */

  acquire(&refs_lock);
  refs[PXP(pa)] += 1;
  release(&refs_lock);
}

void kdec(void *pa) {

  acquire(&refs_lock);
  /* printf("call kdec: refs %d\n", refs[PXP(pa)]); */
  if (refs[PXP(pa)] == 0) {
    /* printf("kdec call kfree\n"); */
    kfree(pa);
  } else if (refs[PXP(pa)] > 0) {
    refs[PXP(pa)] -= 1;
  } else {
    panic("kdec2");
  }
  release(&refs_lock);
}
