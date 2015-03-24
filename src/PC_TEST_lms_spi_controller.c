/*

	Created on: March 23, 2015
		Author: Cameron Karlsson


	lms_spi_controller Test Project

	Run this in your favorite debugger.
	MSVC,GDB,Xcode... whatever.
	With the NULL hardware specified, you can debug the 
	state machine on a PC with relative ease. No JTAG needed.


*/

// So we can run on a PC, not NIOS
#define BLADE_NULL_HARDWARE

// Blade device access functions / wrappers
#include "blade_hardware.h"

// Byte Stream State Machines / utils
#include "state_machine.h"
#include "packet_router.h"
#include "sm_fx3uart.h"




// Entry point
int main()
{

	// initialize hardware settings
	blade.initialize();

	// -- create and configure byte router, processing state machine --
	
	// The byte router will send the byte to whatever state machine is active based on
	// its 'magic' byte. So yes, many state machines can be available with different magics.
	packet_router 		router 				= packet_router_new();
	
	// The custom state machine state container, aka. internal data container.
	sm_fx3uart_state 	smfx3uart_container = {0};
	// The custom state machine designed to emulate the old fpga 0.1.2 behavior.
	state_machine 		smfx3uart 			= sm_fx3uart_new(&smfx3uart_container);

	// assign sm_fx3uart state machine object to slot 0 of the router with the old magic value. 'N'
	packet_router_register_machine(&router, &smfx3uart, 0, UART_PKT_MAGIC);

	/*
	// References:

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
	
	*/

	int h4x0r = 1337;
	// A list of host messages to test...
	uint8_t list[][16] = {

			// Blank Command
			//{ 'N', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

			/*
			// Device Read Command Test
			{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_LMS	| 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_VCTCXO | 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			{ 'N', UART_PKT_MODE_DIR_READ | UART_PKT_DEV_SI5338 | 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			*/
			// Device Write Command Test
			{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_LMS	 | 1, 0, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_LMS	 | 1, 1, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_LMS    | 1, 2, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_LMS    | 1, 3, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },

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
			//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 0+0, (h4x0r >> 8*0)&0xff, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 0+1, (h4x0r >> 8*1)&0xff, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 0+2, (h4x0r >> 8*2)&0xff, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			//{ 'N', UART_PKT_MODE_DIR_WRITE | UART_PKT_DEV_GPIO | 1, 0+3, (h4x0r >> 8*3)&0xff, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
			

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

	DSHOW(sizeof(list));

	int i = 0;
	while (i++ < sizeof(list) / 16)
	{
		printf("------ transaction --------\n");
		int k = 0;
		for (k = 0; k < 16; k++)
		{
			// read fx3 UART connection from host
			uint8_t uart_byte = list[i-1][k];

			printf(">>recv byte: %d\n", uart_byte);

			// send byte into byte handler / state machine router
			packet_router_handle_byte(&router, uart_byte);

		}

	}// while

	printf("Done.\n");
	getchar();

  return 0;
}





