// Boot loader.
//
// Part of the boot block, along with bootasm.S, which calls bootmain().
// bootasm.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.

#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"

#define SECTSIZE  512

void readseg(uchar*, uint, uint);

void
bootmain(void)
{
  struct elfhdr *elf;
  struct proghdr *ph, *eph;
  void (*entry)(void);
  uchar* pa;

  // [Task 3] 极简模式：只输出字符 '1'
  // 省略了 [] 和换行，为了塞进 512 字节
  outb(0x3f8, '1'); 

  elf = (struct elfhdr*)0x10000;  // scratch space

  readseg((uchar*)elf, 4096, 0);

  // [Task 3] 极简模式：只输出字符 '2'
  outb(0x3f8, '2'); 

  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error

  ph = (struct proghdr*)((uchar*)elf + elf->phoff);
  eph = ph + elf->phnum;
  for(; ph < eph; ph++){
    pa = (uchar*)ph->paddr;
    readseg(pa, ph->filesz, ph->off);
    if(ph->memsz > ph->filesz)
      stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
  }

  entry = (void(*)(void))(elf->entry);
  
  // [Task 3] 极简模式：只输出字符 '3'
  outb(0x3f8, '3'); 

  entry();
}

void
waitdisk(void)
{
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

void
readsect(void *dst, uint offset)
{
  waitdisk();
  outb(0x1F2, 1);   // count = 1
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4);
}

void
readseg(uchar* pa, uint count, uint offset)
{
  uchar* epa;
  epa = pa + count;
  pa -= offset % SECTSIZE;
  offset = (offset / SECTSIZE) + 1;
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}