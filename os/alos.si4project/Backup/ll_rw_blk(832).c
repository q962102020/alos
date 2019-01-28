/*
 *  linux/kernel/blk_dev/ll_rw.c
 *
 * (C) 1991 Linus Torvalds
 */

/*
 * This handles all read/write requests to block devices
 */
#include <errno.h>
#include <linux/kernel.h>
#include <asm/system.h>


#include<linux/fs.h>
#include<drivers/ide.h>
/*
 * used to wait on when there are no free requests
 */
task_struct * wait_for_request = NULL;


static inline void lock_buffer(struct buffer_head * bh)
{
	cli();
	while (bh->b_lock)
		sleep_on(&bh->b_wait);
	bh->b_lock=1;
	sti();
}

static inline void unlock_buffer(struct buffer_head * bh)
{
	if (!bh->b_lock)
		printk("ll_rw_block.c: buffer not locked\n\r");
	bh->b_lock = 0;
	wake_up(&bh->b_wait);
}

/*
 * add-request adds a request to the linked list.
 * It disables interrupts so that it can muck with the
 * request-lists in peace.
 */
/*static void add_request(struct blk_dev_struct * dev, struct request * req)
{

}

static void make_request(int major,int rw, struct buffer_head * bh)
{
	
}
*/
#include<fat.h>
#define VM_START 0xc0000000
void ll_rw_block(int rw, struct buffer_head * bh)
{
	if(rw==READ){
		if(bh->b_dev==10){
			if(file_fat_read("/EXT.IMG", bh->b_data,bh->b_blocknr*1024,1024)<0)
				panic("file read ext.img");
			bh->b_uptodate=1;
		}
		if(bh->b_dev==1){
			lock_buffer(bh);//目前锁没用，因为会直接读磁盘
			if(ide_read(bh->b_data,bh->b_blocknr*2,1024)<0)
				panic("ll_rw_block--read");
			unlock_buffer(bh);//同上
			bh->b_uptodate=1;
		}
	}
	if(rw==WRITE){
			panic("ll_rw_block(write)");
	}
}

/*void blk_dev_init(void)
{
	int i;

	for (i=0 ; i<NR_REQUEST ; i++) {
		request[i].dev = -1;
		request[i].next = NULL;
	}
}*/
