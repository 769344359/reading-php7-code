什么是跳转?  
如果回答了这个问题可能会对什么是协程有所了解.  

我们可以逐步了解什么是协程.  
第一步:进程  
什么是进程? 我也不是很懂,在我看来,或者在Linux实现上看来,就是隔离资源的东西.不共享内存的情况下大部分资源都是独立的.  
第二步: 线程  
什么是线程?我也不是很懂,在我看来是一个共享资源的进程(页表,打开文件fd什么的).  

进程和线程有什么共同点?  
在我看来,就是由内核调度,由内核切换.切换时机貌似是现在Linux有公平的时间片算法调度,触发时机还没确定.或许是时间中断吧.

函数调用是什么呢?  
函数调用和上面的进程或者协程有关系吗? 我感觉是没有的,但是还是要说一下,函数调用靠的其实是跳转.汇编应该是jump 或者call.

说完`进程 线程` 和 `函数调用`,下面就可以说一下协程了.  
协程是什么?   
在我看来,就是一个特别的函数调用,调度时机也不是线程或者进程那样内核调度,一般的调度时机是什么呢?一般调度时机是系统调用后(退出系统调用后的回调),退出系统调用后的回调会去
做一个调用调度函数判断是直接切换到另外一个协程还是继续本来这个协程

- 例子  
`swoole-src/thirdparty/boost/asm/make_x86_64_sysv_elf_gas.S`

```
/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|        R12        |         R13        |        R14        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    8    |    9    |   10    |   11    |    12    |    13   |    14   |    15   |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x20  |   0x24  |   0x28  |  0x2c   |   0x30   |   0x34  |   0x38  |   0x3c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        R15        |        RBX        |         RBP        |        RIP        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    16   |   17    |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x40  |   0x44  |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        EXIT       |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/

.text
.globl make_fcontext
.type make_fcontext,@function
.align 16
make_fcontext:
    /* first arg of make_fcontext() == top of context-stack */
    movq  %rdi, %rax

    /* shift address in RAX to lower 16 byte boundary */
    andq  $-16, %rax

    /* reserve space for context-data on context-stack */
    /* size for fc_mxcsr .. RIP + return-address for context-function */
    /* on context-function entry: (RSP -0x8) % 16 == 0 */
    leaq  -0x48(%rax), %rax

    /* third arg of make_fcontext() == address of context-function */
    movq  %rdx, 0x38(%rax)

    /* save MMX control- and status-word */
    stmxcsr  (%rax)
    /* save x87 control-word */
    fnstcw   0x4(%rax)

    /* compute abs address of label finish */
    leaq  finish(%rip), %rcx
    /* save address of finish as return-address for context-function */
    /* will be entered after context-function returns */
    movq  %rcx, 0x40(%rax)

    ret /* return pointer to context-data */

finish:
    /* exit code is zero */
    xorq  %rdi, %rdi
    /* exit application */
    call  _exit@PLT
    hlt
.size make_fcontext,.-make_fcontext

/* Mark that we don't need executable stack. */
.section .note.GNU-stack,"",%progbits
```
