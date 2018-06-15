#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <drivers/dummy_drv.h>
#include <mm/core_mmu.h>	// define SMALL_PAGE_SIZE
#include <kernel/panic.h>
#include <kernel/tee_time.h>
#include <kernel/thread.h>
#include <platform_config.h>
#include <util.h>

struct dummy_data * read_dummy;		//Structure to hold data passed through SMC calls

/*
*       Func Name: tee_dummy_open
*
*       Allocate memory and initialize values for size, cursors. 
*
*	Return value: Return the dummy_data structure that will be used to hold data
*		      either received through the registers from NW or
*		      that will be passed back through registers to NW.
*/
struct dummy_data* tee_dummy_open(void){
	int i;

	/*Initialize and allocate memory*/
	dummy_drv.size = 0;
	dummy_drv.length = 0;
	dummy_drv.read_len = 0;
	dummy_drv.data =  malloc(SMALL_PAGE_SIZE * NUM_PAGES);

	if(dummy_drv.data == NULL){
		return NULL;
	}
	dummy_drv.size = SMALL_PAGE_SIZE * NUM_PAGES;  	
	memset(dummy_drv.data, 0, dummy_drv.size);	//Fill the memory(page) with 0

	/*Initialize data structure for passing data within OPTEE*/
	read_dummy = malloc(sizeof(dummy_data));
	for(i = 0; i < 4; i++){
		read_dummy->w[4] = 0;
	}
	return read_dummy; 
}

/*
*       Func Name: tee_dummy_close
*
*       dummy_drv.read_len:: keep track of number of 'char' that has been read
*/
int tee_dummy_close(void){
	int i = 0;
	if(dummy_drv.size > 0){

		/* Free allocated memory */
    		free(dummy_drv.data);
		dummy_drv.size = 0;
		dummy_drv.length = 0;
		dummy_drv.read_len = 0;
		dummy_drv.data = NULL;
		
		/* Free memory of the structure used to store data of the smc call registers */
		free(read_dummy);
		for(i = 0; i < 4; i++){
			read_dummy->w[4] = 0;
		}
		read_dummy = NULL;
		
		return 0;
  	}else{
		return -1;
	}
}

/*
*	Func Name: tee_dummy_read
*
*	dummy_drv.read_len: keep track of number of 'char' that has been read
*			= equivalent to 'READ cursor'
*/
int tee_dummy_read(void){
	int start_idx;
	start_idx = dummy_drv.read_len/8;

	read_dummy->w[0] = dummy_drv.data[start_idx];
	read_dummy->w[1] = dummy_drv.data[start_idx+1];
	read_dummy->w[2] = dummy_drv.data[start_idx+2];

	//8 (char) * 3 (register) = 24
	dummy_drv.read_len = dummy_drv.read_len + 24;

	return 0;
}

/*
*       Func Name: tee_dummy_write
*
*  	dummy_drv.length: keep track of the total number of 'char' written
* 			  in the allocated memory
*			= equivalent to 'WRITE cursor'
*/
int tee_dummy_write(void){
	int i;
	int start_idx;

	start_idx = dummy_drv.length;

	//8 (char) * 4 (register) = 32
	for(i = 0; i < 32; i++){
		if(((char*)(read_dummy->w))[i] != 0){
			dummy_drv.length = dummy_drv.length + i + 1;
		}
		((char*)(dummy_drv.data))[start_idx + i] = ((char*)(read_dummy->w))[i];
	}

	return 0;
}

/*
*       Func Name: tee_dummy_reset
*
*       cmd values: 0 - READ cursor reset
*		    1 - WRITE cursor reset
*	Users can start overwrite data by moving the cursor to the 
*	beginning of the memory page with RESET SMC call. 
*/
int tee_dummy_reset(int cmd){
	if(cmd == 0){
		dummy_drv.read_len = 0;
	}else{
		dummy_drv.length = 0;
	}

	return 0;
}
