#include<sys/types.h>
#define move_to_user_mode() \
__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x17\n\t" \
	"pushl %%eax\n\t" \
	"pushfl\n\t" \
	"pushl $0x0f\n\t" \
	"pushl $1f\n\t" \
	"iret\n" \
	"1:\tmovl $0x17,%%eax\n\t" \
	"movw %%ax,%%ds\n\t" \
	"movw %%ax,%%es\n\t" \
	"movw %%ax,%%fs\n\t" \
	"movw %%ax,%%gs" \
	:::"ax")

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)
#define nop() __asm__ ("nop"::)

#define iret() __asm__ ("iret"::)


/*#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))*/
	
#define _set_gate(gate_addr,type,dpl,addr) do{ \
	*(u32 *)(gate_addr)=((u32)(addr) & 0xffff) | 0x00080000;	\
	*((u32 *)(gate_addr)+1)=0x8000 | ((u32)(addr)&0xffff0000) | \
			(type)<<8 | (dpl)<<13;	\
	}while(0)


#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],14,3,addr)



#define _set_seg_desc(gate_addr,type,dpl,base,limit) do{ \
	*(u32 *)(gate_addr)=((u32)(base) << 16) | ((limit) & 0xffff); \
	*((u32 *)(gate_addr)+1)=((u32)(base) & 0xff000000) | \
		(((u32)(base) >> 16) & 0xff) | \
		((dpl)<<13) | ((type)<<8) | ((limit) & 0x0f0000) | \
		(0x00c09000); }while(0)


#define _set_seg_tss(gate_addr,type,dpl,base,limit) do{ \
	*(u32 *)(gate_addr)=((u32)(base) << 16) | ((limit) & 0xffff); \
	*((u32 *)(gate_addr)+1)=((u32)(base) & 0xf0000000) | \
		(((u32)(base) >> 16) & 0xff) | \
		((dpl)<<13) | ((type)<<8) | ((limit) & 0x0f0000) | \
		(0x00808000); }while(0)	

#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)

#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x89")
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x82")

