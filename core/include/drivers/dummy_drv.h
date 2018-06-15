#ifndef __DUMMY_DRV_H__
#define __DUMMY_DRV_H__

#define NUM_PAGES 1

struct dummy_drv{
	unsigned long* data;
	unsigned long size;
	int length; 	/* WRITE cursor
			 * number of characters that are currently stored in our memory page.*/
	int read_len; 	/* READ cursor
			 *  keep track of number of 'char' that has been read*/
} dummy_drv;

struct dummy_data{
	unsigned long w[4];
};  
extern struct dummy_data dummy_data;

struct dummy_data* tee_dummy_open(void);
int tee_dummy_close(void);
int tee_dummy_read(void);
int tee_dummy_write(void);
int tee_dummy_reset(int cmd_type);

#endif
