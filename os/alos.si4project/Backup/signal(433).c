/* 
* linux/kernel/signal.c
*
* (C) 1991 Linus Torvalds
*/
/*
注意：signal.c和fork.c文件的编译选项内不能有vc变量优化选项/Og，因为这两个文件
	内的函数参数内包含了函数返回地址等内容。如果加了/Og选项，编译器就会在认为
	这些参数不再使用后占用该内存，导致函数返回时出错。
	math/math_emulate.c照理也应该这样，不过好像它没有把eip等参数优化掉:)
*/

#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <asm/segment.h>	// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。

#include <signal.h>		// 信号头文件。定义信号符号常量，信号结构以及信号操作函数原型。

#include<linux/sys.h>
// 获取当前任务信号屏蔽位图（屏蔽码）。
int sys_sgetmask ()
{
	return current->blocked;
}

// 设置新的信号屏蔽位图。SIGKILL 不能被屏蔽。返回值是原信号屏蔽位图。
int sys_ssetmask (int newmask)
{
	panic("not support");
}



// signal()系统调用。类似于sigaction()。为指定的信号安装新的信号句柄(信号处理程序)。
// 信号句柄可以是用户指定的函数，也可以是SIG_DFL（默认句柄）或SIG_IGN（忽略）。
// 参数signum --指定的信号；handler -- 指定的句柄；restorer –原程序当前执行的地址位置。
// 函数返回原信号句柄。
int sys_signal (int signum, long handler, long restorer)
{
	struct sigaction tmp;

	if (signum < 1 || signum > 32 || signum == SIGKILL)	// 信号值要在（1-32）范围内，
		return -1;			// 并且不得是SIGKILL。
	tmp.sa_handler = (void (*)(int)) handler;	// 指定的信号处理句柄。
	tmp.sa_mask = 0;		// 执行时的信号屏蔽码。
	tmp.sa_flags = SA_ONESHOT | SA_NOMASK;	// 该句柄只使用1 次后就恢复到默认值，
// 并允许信号在自己的处理句柄中收到。
	tmp.sa_restorer = (void (*)(void)) restorer;	// 保存返回地址。
	handler = (long) current->sigaction[signum - 1].sa_handler;
	current->sigaction[signum - 1] = tmp;
	return handler;
}
// sigaction()系统调用。改变进程在收到一个信号时的操作。signum 是除了SIGKILL 以外的任何
// 信号。[如果新操作(action)不为空]则新操作被安装。如果oldaction 指针不为空，则原操作
// 被保留到oldaction。成功则返回0，否则为-1。
int sys_sigaction (int signum, const struct sigaction *action,
					struct sigaction *oldaction)
{
	panic("not support");
	return 0;
}
/*
 系统调用中断处理程序中真正的信号处理程序（在kernel/system_call.s,119 行）。
 该段代码的主要作用是将信号的处理句柄插入到用户程序堆栈中，并在本系统调用结束
 返回后立刻执行信号句柄程序，然后继续执行用户的程序。*/
void do_signal (long eax, long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	unsigned long sa_handler;
	long signr=0,sign;
	long old_eip = eip;
	struct sigaction *sa; 
	int longs;
	unsigned long *tmp_esp;
	//如果不是将要返回用户模式，也即内核模式时，发生中断的返回，不处理信号
	if((u16)cs!=0x1b)
		return ;
	//无非屏蔽信号，则返回(应该添加非返回用户模式，则返回)
	sign = current->signal & ~current->blocked;
	if(!sign)
		return ;
	
	//查找信号位图最前面的信号，并转换为序列号,这段可以用bsf和btr嵌入汇编优化，但这样看起来简单
	signr = 0;
	while(!(sign&0x1)){
		signr++;
		sign >>= 1;
	}
	current->signal &= ~(1<<signr);//复位处理信号

	/*				处理信号							*/
	sa=&current->sigaction[signr];
	sa_handler=(unsigned long)sa->sa_handler;
	if (sa_handler == 1)//忽视
		return;
	if (!sa_handler)//默认
	{
		if (signr == SIGCHLD)
			return;
		else
			sys_exit (1 << (signr));//终止
	}
	// 如果该信号句柄只需使用一次，则将该句柄置空(该信号句柄已经保存在sa_handler 指针中)。
	if (sa->sa_flags & SA_ONESHOT)
		sa->sa_handler = NULL;
	
	/*			填充用户堆栈和内核堆栈						*/
	eip = sa_handler;//返回地址改为信号处理函数
	
// 如果允许信号自己的处理句柄收到信号自己，则也需要将进程的阻塞码压入堆栈。
	//longs = (sa->sa_flags & SA_NOMASK) ? 7 : 8;
	longs=8;
	esp -= longs;
	verify_area (esp, longs * 4);
// 在用户堆栈中从下到上存放sa_restorer, 信号signr, 屏蔽码blocked(如果SA_NOMASK 置位),
// eax, ecx, edx, eflags 和用户程序原代码指针。
	tmp_esp = esp;

	put_fs_long ((u32) sa->sa_restorer, tmp_esp++);	//sa_resorer
	put_fs_long (signr, tmp_esp++);			//signr
	//if (!(sa->sa_flags & SA_NOMASK))
		//put_fs_long (current->blocked, tmp_esp++);
	put_fs_long (eax, tmp_esp++);			//eax
	put_fs_long (ebx,tmp_esp++);			//源程序不知道为什么不保存ebx
	put_fs_long (ecx, tmp_esp++);			//ecx
	put_fs_long (edx, tmp_esp++);			//edx
	put_fs_long (eflags, tmp_esp++);		//eflags
	put_fs_long (old_eip, tmp_esp++);		//old_eip
	//current->blocked |= sa->sa_mask;	// 进程阻塞码(屏蔽码)添上sa_mask 中的码位。
}

int sys_sigreturn()
{
	panic("in sigreturn \n");

}

void sys_sigreturn (long eax, long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	//恢复为信号发生前的状态
	int longs;
	unsigned long *tmp_esp;

	eax=get_fs_long()
	
}

