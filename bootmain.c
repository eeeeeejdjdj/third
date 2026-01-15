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

  // 0x10000 是一个暂存区域，用来存放读取的 ELF 头
  elf = (struct elfhdr*)0x10000;  // scratch space

  // Read 1st page off disk
  // 从磁盘读取第一页数据（4096字节），这通常包含了 ELF 头和程序头表
  readseg((uchar*)elf, 4096, 0);

  // Is this an ELF executable?
  // 检查是否为合法的 ELF 可执行文件（通过检查魔数）
  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error

  // Load each program segment (ignores ph flags).
  // 加载每个程序段（忽略 ph 标志）。
  // elf->phoff 是程序头表在文件中的偏移量
  ph = (struct proghdr*)((uchar*)elf + elf->phoff);
  eph = ph + elf->phnum;
  for(; ph < eph; ph++){
    pa = (uchar*)ph->paddr;
    // 将段从磁盘读入到物理地址 ph->paddr
    readseg(pa, ph->filesz, ph->off);
    
    // 如果内存大小大于文件大小，说明包含 BSS 段（未初始化数据），需要清零
    if(ph->memsz > ph->filesz)
      stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
  }

  // Call the entry point from the ELF header.
  // Does not return!
  // 调用 ELF 头中指定的入口点函数（即内核入口）。
  // 这个函数调用之后将不再返回！
  entry = (void(*)(void))(elf->entry);
  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready.
  // 等待磁盘准备好。读取 0x1F7 端口的状态寄存器。
  // 0xC0 掩码检查 BSY (Busy) 和 RDY (Ready) 位。
  // 等待直到 BSY 为 0 且 RDY 为 1 (即结果为 0x40)。
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
// 读取 offset 处的一个扇区到 dst 地址。
void
readsect(void *dst, uint offset)
{
  // Issue command.
  waitdisk();
  outb(0x1F2, 1);   // count = 1 (读取扇区数)
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0); // LBA 模式，主盘
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors (读取扇区命令)

  // Read data.
  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4); // 从数据端口 0x1F0 读取数据
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
// 从内核文件偏移 offset 处读取 count 字节到物理地址 pa。
// 可能会读取比请求更多的字节（因为按扇区读取）。
void
readseg(uchar* pa, uint count, uint offset)
{
  uchar* epa;

  epa = pa + count;

  // Round down to sector boundary.
  // 向下取整到扇区边界，因为磁盘读取是以扇区为单位的。
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  // 将字节偏移转换为扇区号；内核从扇区 1 开始（扇区 0 是引导扇区）。
  offset = (offset / SECTSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  // 循环读取扇区直到读完所需的字节数。
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
