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

struct run *borrow(int id);

struct {
  struct spinlock lock;
  struct run *freelist;
  int pages;
} kmem[NCPU];

void kinit() {
  for (int i = 0; i < NCPU; i++) {
    initlock(&kmem[i].lock, "kmem");
    kmem[i].pages = 0;
    kmem[i].freelist = 0;
  }
  freerange(end, (void *)PHYSTOP);
  printf("cpu0 pages %d\n", kmem[0].pages);
}

void freerange(void *pa_start, void *pa_end) {
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa) {
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  push_off();
  /* acquire(&kmem[cpuid()].lock); */
  r->next = kmem[cpuid()].freelist;
  kmem[cpuid()].freelist = r;
  kmem[cpuid()].pages += 1;
  /* release(&kmem[cpuid()].lock); */
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *kalloc(void) {
  struct run *r;

  push_off();
  int id = cpuid();
  /* acquire(&kmem[id].lock); */
  r = kmem[id].freelist;
  if (r) {
    kmem[id].freelist = r->next;
    kmem[id].pages -= 1;
  } else {
    r = borrow(id);
  }
  /* release(&kmem[id].lock); */
  pop_off();

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}

struct run* borrow(int id) {
  // borrow freelist from other cpu.
  struct run *p, *r;
  p = r = 0;
  for (int i = 0; i < NCPU; i++) {
    if (i == id) {
      continue;
    } else {
      acquire(&kmem[i].lock);
      if (kmem[i].pages / 2 > 0) {
        p = kmem[i].freelist;
        int j = 1;
        for (; j < kmem[i].pages / 2; j++) {
          p = p->next;
        }

        r = p->next;
        p->next = 0;
        kmem[id].freelist = r->next;
        kmem[id].pages = kmem[i].pages - kmem[i].pages / 2 - 1;
        kmem[i].pages = kmem[i].pages / 2;
        release(&kmem[i].lock);
        break;
      }
      release(&kmem[i].lock);
    }
  }
  return r;
}
