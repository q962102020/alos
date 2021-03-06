#ifndef _SCHED_H
#define _SCHED_H

#include <linux/fs.h>
#include <signal.h>

#if (NR_OPEN > 32)
#error "Currently the close-on-exec-flags are in one word, max 32 files/proc"
#endif


#define NR_TASKS 64
#define HZ 100
#define CURRENT_TIME (startup_time+jiffies/HZ)

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4


typedef struct task_struct {
/* these are hardcoded - don't touch */
	long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	long counter;
	long priority;
	long signal;
	struct sigaction sigaction[32];
	long blocked;	/* bitmap of masked signals */
/* various fields */
	int exit_code;
	unsigned long start_code,end_code,end_data,brk,start_stack;
	unsigned long mem_size;
	long pid,father,pgrp,session,leader;
	unsigned short uid,euid,suid;
	unsigned short gid,egid,sgid;
	long alarm;
	long utime,stime,cutime,cstime,start_time;
	unsigned short used_math;
/* file system info */
	int tty;		/* -1 if no tty, so it must be signed */
	unsigned short umask;
	struct m_inode * pwd;
	struct m_inode * root;
	struct m_inode * executable;
	unsigned long close_on_exec;
	struct file * filp[NR_OPEN];
/*	sched  */
	struct tss_struct *tss;	//所有进程共享一个tss(指向同一个tss)
	unsigned int cr3;
	unsigned int pause_stack;
	unsigned int pause_eip;
} task_struct ;

typedef struct tss_struct {
	long	back_link;	/* 16 high bits zero */
	long	esp0;
	long	ss0;		/* 16 high bits zero */
	long	esp1;
	long	ss1;		/* 16 high bits zero */
	long	esp2;
	long	ss2;		/* 16 high bits zero */
	long	cr3;
	long	eip;
	long	eflags;
	long	eax,ecx,edx,ebx;
	long	esp;
	long	ebp;
	long	esi;
	long	edi;
	long	es;		/* 16 high bits zero */
	long	cs;		/* 16 high bits zero */
	long	ss;		/* 16 high bits zero */
	long	ds;		/* 16 high bits zero */
	long	fs;		/* 16 high bits zero */
	long	gs;		/* 16 high bits zero */
	long	ldt;		/* 16 high bits zero */
	long	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
}tss_struct;

extern task_struct *task[NR_TASKS];
extern task_struct *current;
extern long volatile jiffies;
extern long startup_time;
extern long pid;		//用于新进程的pid，每次+1
extern struct tss_struct tss;	//所有进程共享同一个tss

void sched_init(void);
void schedule(void);
void add_timer(long jiffies, void (*fn)(void));
void sleep_on(task_struct ** p);
void interruptible_sleep_on(task_struct ** p);
void wake_up(task_struct ** p);
void show_task (int nr);


#endif
