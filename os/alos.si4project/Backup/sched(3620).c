/*
 *  linux/kernel/sched.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * 'sched.c' is the main kernel file. It contains scheduling primitives
 * (sleep_on, wakeup, schedule etc) as well as a number of simple system
 * call functions (type getpid(), which just extracts a field from
 * current-task
 */
#include <linux/sched.h>
#include <linux/kernel.h>
#include <memory.h>
#include<string.h>
#include <asm/system.h>
#include <asm/io.h>

//#include <signal.h>

#define _S(nr) (1<<((nr)-1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

void show_task(int nr)
{

}

void show_stat(void)
{

}

#define LATCH (1193180/HZ)

extern void mem_use(void);

extern int timer_interrupt(void);
extern int system_call(void);

union task_union {
	task_struct task;
	char stack[PAGE_SIZE];
};

//static union task_union init_task = {};

long volatile jiffies=0;
long startup_time=0;
task_struct *current;
task_struct *last_task_used_math = NULL;

task_struct * task[NR_TASKS];


/*
 *  'math_state_restore()' saves the current math information in the
 * old math state array, and gets the new ones from the current task
 */
void math_state_restore()
{
	
}

struct tss_struct tss;


/*切换任务:
			0.保存所有状态
			1.current 指向新任务 
			2.tss->esp0 指向新任务内核堆栈
			3.切换页表(cr3)
			4.恢复esp,eip
			5.恢复所有状态
*/
 void switch_to(int next)
 {
 	/* 保存现场 */
 	asm("pusha\n"
		"pushf\n"
		"mov %%esp,%0\n"
		"movl $1f,%1\n"
		:"=m"(current->pause_stack),"=m"(current->pause_eip):);


	/* 切换任务 */
	current=task[next];		//current
	current->tss->esp0=(u32)(current)+PAGE_SIZE; //tss
	asm("mov %0,%%cr3\n"	//cr3
		"mov %1,%%esp\n"	//esp
		"jmp *%2\n"			//eip
		::"g"(current->cr3),"m"(current->pause_stack),"m"(current->pause_eip));
		

	/* 恢复现场 */
	asm("1:popf\n"
		"popa\n");

 }




void schedule(void)
{
	int i,next,c;
	task_struct ** p=task;	
	
	while(1){
		c=0;
		next=0;
		for(i=0;i<NR_TASKS;i++){
			if(task[i] && task[i]->state==TASK_RUNNING && task[i]->counter>c)
				c=task[i]->counter,next=i;
		}
		if(c)
			break;
		for(i=0;i<NR_TASKS;i++){
			if(task[i])
				task[i]->counter=task[i]->priority;
		}
	}
	if(current!=task[next])
		switch_to(next);				
}


int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

//镶嵌唤醒当前任务之前睡眠的进程
void sleep_on(task_struct **p)
{
	printk("in sleep\n");
	task_struct *tmp;
	if(!p)
		return ;
	/*
	if(current==task[0])
		panic("task[0] trying to sleep");*/
	tmp = *p;
	*p = current;

	current->state = TASK_UNINTERRUPTIBLE;
	schedule();

	if(tmp)
		tmp->state = 0;
}


/* 永远唤醒最新被睡眠的进程 （即使唤醒过程中，有新进程被睡了）*/
void interruptible_sleep_on(task_struct **p)
{
	task_struct *tmp;
	if(!p)
		return ;
	/*
	if(current==task[0])
		panic("task[0] trying to sleep");*/
	tmp = *p;
	*p = current;
	
	while(1){		/* (*p && *p!=current) 时继续睡眠 */
		current->state = TASK_INTERRUPTIBLE;
		schedule();

		if(*p==NULL || *p==current)
			break;
		(**p).state = 0;
	}
	*p=NULL;//*p=tmp
	if(tmp)
		tmp->state = 0;
	
}

void wake_up(task_struct **p)
{
	if (p && *p) {
		(**p).state=0;
		*p=NULL;
	}
}

/*
 * OK, here are some floppy things that shouldn't be in the kernel
 * proper. They are here because the floppy needs a timer, and this
 * was the easiest way of doing it.
 */
static struct task_struct * wait_motor[4] = {NULL,NULL,NULL,NULL};
static int  mon_timer[4]={0,0,0,0};
static int moff_timer[4]={0,0,0,0};
unsigned char current_DOR = 0x0C;



#define TIME_REQUESTS 64

static struct timer_list {
	long jiffies;
	void (*fn)();
	struct timer_list * next;
} timer_list[TIME_REQUESTS], * next_timer = NULL;

void add_timer(long jiffies, void (*fn)(void))
{

}

void do_timer(int cpl)
{
	if(cpl)
		current->utime++;
	else
		current->stime++;
	
	current->counter--;
	if(!current->counter)
		if(cpl)
			schedule();
		else
			current->counter++;
}


int sys_alarm(long seconds)
{
	int old = current->alarm;

	if (old)
		old = (old - jiffies) / HZ;
	current->alarm = (seconds>0)?(jiffies+HZ*seconds):0;
	return (old);
}

int sys_getpid(void)
{
	return current->pid;
}

int sys_getppid(void)
{
	return current->father;
}

int sys_getuid(void)
{
	return current->uid;
}

int sys_geteuid(void)
{
	return current->euid;
}

int sys_getgid(void)
{
	return current->gid;
}

int sys_getegid(void)
{
	return current->egid;
}

int sys_nice(long increment)
{
	panic("not support");
}


void sched_init(void)
{	
	memset(task,0,sizeof(task));	
	current=task[0]=(task_struct *)(get_free_page()+VM_START);	
	memset(current,0,PAGE_SIZE);//为了保险期间，清空
	
	current->priority=10;
	current->counter=10;
	current->state=0;		//TASK_RUNNING
	current->tty=0;			//默认tty通道0
	current->pid=0;			//进程号
	current->father=0;
	current->uid=0;			//超级用户
	current->euid=0;
	current->gid=0;

	
	asm("mov %%cr3,%%eax\n"	//当前页表设为task[0]的页表
		"mov %%eax,%0"
		:"=m"(current->cr3)::"eax");

	//所有进程共享同一个TSS	
	current->tss=&tss;
	current->tss->esp0=(unsigned long)current+PAGE_SIZE;
	current->tss->ss0=SELECTOR(KERNEL_DATA,PL_KERNEL);

	/*进程0的起始堆栈，目前是VM_START,应该设为current+PAGE_SIZE，
	但是，这样便要复制堆栈里的内容，否则，无法返回，于是设置堆栈就放在main()函数里，
	这样，就可以直接丢弃堆栈内容*/
	//current->esp=(unsigned long)current+PAGE_SIZE;
	
}
