/*
	Created on: March 25
	Author: Cameron Karlsson
*/

#ifndef _BLADE_NULL_HARDWARE_H_
#define _BLADE_NULL_HARDWARE_H_


// Because there is no hardware uart, we simulate it with some overloads.
// This entire file is only included if BLADE_NULL_HARDWARE is defined.
// This is to allow *small* versions of the hardware, like emulating a read/write.
// Nothing fancy.


// KEEP IT SIMPLE!
// Only implement the most basic functionality you need to get the state machine working.
// If the default printf in a write hardware function is enough to understand what's happening... leave it!


// DO NOT INCLUDE YOUR STATE MACHINE HERE!
// This file should know nothing about your SM.


#define UART_PKT_MAGIC          'N'
#define UART_PKT_MODE_CNT_MASK   0xF
#define UART_PKT_MODE_CNT_SHIFT  0

#define UART_PKT_MODE_DEV_MASK   0x30
#define UART_PKT_MODE_DEV_SHIFT  4
#define UART_PKT_DEV_GPIO        (0<<UART_PKT_MODE_DEV_SHIFT)
#define UART_PKT_DEV_LMS         (1<<UART_PKT_MODE_DEV_SHIFT)
#define UART_PKT_DEV_VCTCXO      (2<<UART_PKT_MODE_DEV_SHIFT)
#define UART_PKT_DEV_SI5338      (3<<UART_PKT_MODE_DEV_SHIFT)

#define UART_PKT_MODE_DIR_MASK   0xC0
#define UART_PKT_MODE_DIR_SHIFT  6
#define UART_PKT_MODE_DIR_READ   (2<<UART_PKT_MODE_DIR_SHIFT)
#define UART_PKT_MODE_DIR_WRITE  (1<<UART_PKT_MODE_DIR_SHIFT)


#define HAXOR 1337
// A list of host messages to test...
uint8_t list[][16] = {

	/*
	========================= References: =============

	typedef struct {
	unsigned char magic;
	unsigned char mode;
	//  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	//  |  dir  |  dev  |     cnt       |
	} uart_pkt;


	typedef struct {
	unsigned char addr;
	unsigned char data;
	} uart_cmd;

	{GDEV_UNKNOWN, 			-1, 0,
	{GDEV_GPIO,           	0,  4,
	{GDEV_IQ_CORR_RX_GAIN,  4,  2,
	{GDEV_IQ_CORR_RX_PHASE, 6,  2,
	{GDEV_IQ_CORR_TX_GAIN,  8,  2,
	{GDEV_IQ_CORR_TX_PHASE, 10, 2,
	{GDEV_FPGA_VERSION,  	12, 4,
	{GDEV_TIME_TAMER,    	16, 16,
	{GDEV_VCTXCO,        	34, 2,
	{GDEV_XB_LO,         	36, 4,
	{GDEV_EXPANSION,     	40, 4,
	{GDEV_EXPANSION_DIR, 	44, 4,

	=========================
	*/

	// Blank Command
	//{ 'N', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

	/*
	// Device Read Command Test
	{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_LMS	| 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_VCTCXO | 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_SI5338 | 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	*/
	// Device Write Command Test
	//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_LMS | 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_LMS | 1, 1, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_LMS | 1, 2, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_LMS | 1, 3, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },

	//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_VCTCXO | 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_SI5338 | 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },


	/*
	// FPGA VERSION
	{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_GPIO | 1, 12+0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_GPIO | 1, 12+1, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_GPIO | 1, 12+2, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_GPIO | 1, 12+3, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	*/

	// GPIO - Should call blade_GDEV_GPIO_write() once (after all four) with data argument 1337.
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 0+0, (HAXOR >> 8*0)&0xff, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 0+1, (HAXOR >> 8*1)&0xff, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 0+2, (HAXOR >> 8*2)&0xff, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 0+3, (HAXOR >> 8*3)&0xff, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },


	/*
	// TIME TAMER
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 16+0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 16+1, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 16+2, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 16+3, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 16+4, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	// ... There could be up to 16
	*/

	// Define more if you need them...
};

// Simulates the host uart functionality so we can read some data.
uint32_t my_host_uart_current_index = 0;
uint8_t my_host_uart_read(void){
	int txn = my_host_uart_current_index / 16;	//truncated
	int i = my_host_uart_current_index % 16;

	uint8_t byte = list[txn][i];
	my_host_uart_current_index++;
	return byte;
}

void my_host_uart_write(uint8_t b){
	printf("\t host_uart_write( %d )\n", b);
}

int my_host_uart_hasdata(void){
	return my_host_uart_current_index < sizeof(list);
}


// You can delete everything in this file except this function, which needs to be at least defined.
// Function called in blade.initialize();
// As a note, you can only reassign the device functions when BLADE_NULL_HARDWARE is defined.
// Otherwise, all blade device functions are const. So don't try assignments in a
// state machine implementation.
void BLADE_NULL_HARDWARE_INIT_FUNC_NAME (){
	blade.devices.host.uart_hasdata =	my_host_uart_hasdata;
	blade.devices.host.uart_read =		my_host_uart_read;
	blade.devices.host.uart_write =		my_host_uart_write;
}






#endif
