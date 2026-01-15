#include "asm.h"
#include "memlayout.h"
#include "mmu.h"

# 启动第一个 CPU: 切换到 32 位保护模式，跳转到 C 代码。
# BIOS 将此代码从硬盘的第一个扇区加载到
# 物理地址 0x7c00 的内存中，并以实模式开始执行
# 初始状态 %cs=0 %ip=7c00。

.code16                       # 汇编为 16 位模式
.globl start
start:
  cli                         # BIOS 启用了中断；这里先禁用中断，以确保引导过程不被打断

  # 将数据段寄存器 DS, ES, 和 SS 清零。
  xorw    %ax,%ax             # 将 %ax 置零
  movw    %ax,%ds             # -> 数据段 (Data Segment)
  movw    %ax,%es             # -> 附加段 (Extra Segment)
  movw    %ax,%ss             # -> 栈段 (Stack Segment)

  # 为了向后兼容，早期 PC 的物理地址线 A20 被强制拉低（置 0），
  # 这导致超过 1MB 的内存地址会被回绕。我们需要启用 A20 线
  # 才能访问所有物理内存。
seta20.1:
  inb     $0x64,%al               # 等待键盘控制器不忙
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 发送 0xd1 命令到端口 0x64 (写输出端口)
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # 等待键盘控制器不忙
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 发送 0xdf 到端口 0x60 (启用 A20)
  outb    %al,$0x60

  # 从实模式切换到保护模式。使用一个临时的 GDT (全局描述符表)，
  # 该 GDT 将虚拟地址直接映射到物理地址，
  # 这样在转换过程中内存映射不会改变。
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE, %eax           # 设置 CR0 的 PE (Protection Enable) 位
  movl    %eax, %cr0

//PAGEBREAK!
  # 通过长跳转 (long jump) 完成到 32 位保护模式的转换。
  # 这会重新加载 %cs 和 %eip。段描述符已设置为无转换，
  # 所以映射仍然是恒等映射。
  ljmp    $(SEG_KCODE<<3), $start32

.code32  # 告诉汇编器现在生成 32 位代码。
start32:
  # 设置保护模式下的数据段寄存器
  movw    $(SEG_KDATA<<3), %ax    # 我们的数据段选择子
  movw    %ax, %ds                # -> DS: 数据段
  movw    %ax, %es                # -> ES: 附加段
  movw    %ax, %ss                # -> SS: 栈段
  movw    $0, %ax                 # 将未使用的段寄存器置零
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS

  # 设置栈指针并调用 C 代码。
  # 栈从 0x7c00 向下增长。
  movl    $start, %esp
  call    bootmain                # 跳转到 bootmain.c 中的 bootmain 函数

  # 如果 bootmain 返回（虽然不应该发生），
  # 如果在 Bochs 下运行，则触发断点，然后死循环。
  movw    $0x8a00, %ax            # 0x8a00 -> 端口 0x8a00
  movw    %ax, %dx
  outw    %ax, %dx
  movw    $0x8ae0, %ax            # 0x8ae0 -> 端口 0x8a00
  outw    %ax, %dx
spin:
  jmp     spin

# 引导 GDT (全局描述符表)
.p2align 2                                # 强制 4 字节对齐
gdt:
  SEG_NULLASM                             # 空段描述符
  SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # 代码段: 可执行, 可读, 基址 0, 界限 4GB
  SEG_ASM(STA_W, 0x0, 0xffffffff)         # 数据段: 可写, 基址 0, 界限 4GB

gdtdesc:
  .word   (gdtdesc - gdt - 1)             # GDT 大小 - 1
  .long   gdt                             # GDT 地址
