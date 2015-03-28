/*

	Created on: March 23, 2015
		Author: Cameron Karlsson


	lms_spi_controller

	>> Running on the NIOS?
	Just compile.

	>> Running on a PC?
	Make sure to #define BLADE_NULL_HARDWARE before "blade_hardware.h".
	Done.

	Run this in your favorite debugger.
	MSVC,GDB,Xcode... whatever.
	With the NULL hardware specified, you can debug the 
	state machine on a PC with relative ease. No JTAG needed.
	Note: If you need some hardware functionality while running on the PC,
	you can define it in "blade_null_hardware_impl.h"


*/



// So we can run test on a PC, not NIOS
//#define BLADE_NULL_HARDWARE

// Will send debug alt_printf info to JTAG console while running on NIOS.
// This can slow performance and cause timing issues... be careful.
//#define BLADE_NIOS_DEBUG

// blade device access functions / wrappers
#include "blade_hardware.h"

// Byte Stream State Machines / utils
//#include "fifo.h"
#include "state_machine.h"
#include "packet_router.h"
#include "sm_fx3uart.h"



// Entry point
int main()
{
	// initialize hardware settings, should always be first
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
	// When assigning, make sure MAX_STATE_MACHINES (packet_router.h) is > your slot number.
	packet_router_register_machine(&router, &smfx3uart, 0, UART_PKT_MAGIC);



	uint8_t buffer[16] = { 0 };
	volatile uint32_t count = 0;

	while (true)
	{
		// Check if anything is in the UART from host (fx3)
		if (blade.devices.host.uart_hasdata())
		{
			buffer[count] = blade.devices.host.uart_read();
			++count;
		}
		else {
			// the first byte is a magic, and the magic byte shouldn't be 0.
			// something happened...
			if (buffer[0] == 0){
				count = 0;
			}
		}
		if (count == 16){
			for (count = 0; count < 16; ++count){
				packet_router_handle_byte(&router, buffer[count]);
				buffer[count] = 0;
			}
			count = 0;
		}


		// Don't want to run forever on PC!
		// You can modify this to fit your testing case.
		#ifdef BLADE_NULL_HARDWARE
		if ( ! blade.devices.host.uart_hasdata() ){
			break;
		}
		#endif


	}// while


	// pause...
	#ifdef BLADE_NULL_HARDWARE
	getchar();
	#endif

  return 0;
}





