/*
 * Copyright (c) 2013 Nuand LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//#include "priv/alt_busy_sleep.h"
//#include "priv/alt_file.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

//#include "sys/alt_dev.h"

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
	// initialize hardware settings
	blade.initialize();

	// clear uart buffer
	while( blade.devices.host.uart_hasdata() )
	{
		blade.devices.host.uart_read();
	}


	// -- create and configure byte router & processing state machine --
	
	// The byte router will send the byte to whatever state machine is active based on
	// its 'magic' byte. So yes, many state machines can be available with different magics.
	packet_router 		router 				= packet_router_new();
	
	// The custom state machine state container, aka. internal data container.
	sm_fx3uart_state 	smfx3uart_container = {0};
	// The custom state machine designed to emulate the old fpga 0.1.2 behavior.
	state_machine 		smfx3uart 			= sm_fx3uart_new(&smfx3uart_container);

	// assign sm_fx3uart state machine object to slot 0 of the router with the old magic value. 'N'
	packet_router_register_machine(&router, &smfx3uart, 0, UART_PKT_MAGIC);


	uint8_t buffer[16] = {0};
	volatile uint32_t count = 0;

	while(true)
	{
		// Check if anything is in the UART from host (fx3)
		if( blade.devices.host.uart_hasdata() )
		{
			buffer[count] = blade.devices.host.uart_read();
			++count;
		} else {
			// the first byte is a wizard, and wizards can't be 0.
			// something happened...
			if(buffer[0] == 0){
				count = 0;
			}
		}
		if( count == 16 ){
			for(count = 0; count < 16; ++count){
				packet_router_handle_byte(&router, buffer[count]);
				buffer[count] = 0;
			}
			count = 0;
		}

	}// while

	return 0;


}






