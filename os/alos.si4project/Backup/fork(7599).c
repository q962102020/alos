#include<linux/sched.h>
#include<string.h>
#include<memory.h>
#include<linux/kernel.h>

int sys_fork();
asm(".text\n.global sys_fork\n"
		"sys_fork:"
		"push %ebp\n"	//����״̬
		"push %esi\n"
		"push %edi\n"
		"call copy_process\n"//���ƽ������ݽṹ
		"push %eax\n"
		"push %esp\n"
		"call set_ret_stat\n"//�����ӽ��̶ϵ�esp��eip���������ӽ��̷���ֵ
		"add $4,%esp\n"		//�ӽ��̴Ӵ˽���
		"pop %eax\n"		//����ֵ
		"pop %edi\n"
		"pop %esi\n"
		"pop %ebp\n"
		"ret");

static int copy_process()
{	
	int i,ret;
	task_struct *newTask=NULL;
	for(i=0;i<NR_TASKS;i++){
		if(!task[i])
			break;
	}
	if(i==NR_TASKS)
		return -1;

	if((newTask=(task_struct *)(get_free_page()+VM_START))==(task_struct *)VM_START)
		return -1;
	task[i]=newTask;

	memcpy(newTask,current,PAGE_SIZE);	
	ret=copy_page_table(newTask);
	
	if(ret<0)
		panic("copy_page_table");
	
	return i;
}

static void set_ret_stat(u32 esp,int next){
	if(next<0)
		return ;
	
	//�ӽ��̶ϵ�ip
	asm("mov %1,%0"
		:"=g"(task[next]->pause_eip):"m"(*(&esp-1)));
	//�ӽ����ں˶�ջ
	esp=(u32)task[next]+esp%PAGE_SIZE-4;
	asm("mov %1,%0"
		:"=m"(task[next]->pause_stack):"g"(esp));
	//�ӽ��̷���ֵ0
	asm("movl $0,%0"
		:"=m"(*((u32 *)esp+1)):);
}  

