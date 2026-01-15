# The xv6 kernel starts executing in this file. This file is linked with
# the kernel C code, so it can refer to kernel symbols such as main().
# The boot block (bootasm.S and bootmain.c) jumps to entry below.
# xv6 内核从这个文件开始执行。此文件与内核 C 代码链接，
# 因此它可以引用内核符号，例如 main()。
# 引导块 (bootasm.S 和 bootmain.c) 跳转到下面的 entry。

# Multiboot header, for multiboot boot loaders like GNU Grub.
# http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
#
# Using GRUB 2, you can boot xv6 from a file stored in a
# Linux file system by copying kernel or kernelmemfs to /boot
# and then adding this menu entry:
#
# menuentry "xv6" {
# 	insmod ext2
# 	set root='(hd0,msdos1)'
# 	set kernel='/boot/kernel'
# 	echo "Loading ${kernel}..."
# 	multiboot ${kernel} ${kernel}
# 	boot
# }

#include "asm.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"

# Multiboot header.  Data to direct multiboot loader.
# Multiboot 头部。用于指导 multiboot 引导加载程序的数据。
.p2align 2
.text
.globl multiboot_header
multiboot_header:
  #define magic 0x1badb002
  #define flags 0
  .long magic
  .long flags
  .long (-magic-flags)

# By convention, the _start symbol specifies the ELF entry point.
# Since we haven't set up virtual memory yet, our entry point is
# the physical address of 'entry'.
# 按照约定，_start 符号指定 ELF 入口点。
# 由于我们尚未设置虚拟内存，我们的入口点是 'entry' 的物理地址。
.globl _start
_start = V2P_WO(entry)

# Entering xv6 on boot processor, with paging off.
# 在引导处理器上进入 xv6，此时分页已关闭。
.globl entry
entry:
  # Turn on page size extension for 4Mbyte pages
  # 开启页面大小扩展 (Page Size Extension, PSE)，允许使用 4MByte 的大页面
  movl    %cr4, %eax
  orl     $(CR4_PSE), %eax
  movl    %eax, %cr4
  
  # Set page directory
  # 设置页目录 (Page Directory)
  # 加载 entrypgdir 的物理地址到 CR3 寄存器。
  # entrypgdir 在 main.c 中定义，提供了初始的简单的页表映射。
  movl    $(V2P_WO(entrypgdir)), %eax
  movl    %eax, %cr3
  
  # Turn on paging.
  # 开启分页 (Paging)。
  # 设置 CR0 的 PG (Paging) 和 WP (Write Protect) 位。
  # 此时，虚拟地址映射开始生效。
  movl    %cr0, %eax
  orl     $(CR0_PG|CR0_WP), %eax
  movl    %eax, %cr0

  # Set up the stack pointer.
  # 设置栈指针。
  # 分配 KSTACKSIZE 大小的栈空间。
  movl $(stack + KSTACKSIZE), %esp

  # Jump to main(), and switch to executing at
  # high addresses. The indirect call is needed because
  # the assembler produces a PC-relative instruction
  # for a direct jump.
  # 跳转到 main()，并切换到高地址执行。
  # 需要间接调用，因为汇编器会为直接跳转生成 PC 相对指令，
  # 而此时我们还在低地址运行，直接跳转可能导致计算出的地址错误。
  mov $main, %eax
  jmp *%eax

.comm stack, KSTACKSIZE
