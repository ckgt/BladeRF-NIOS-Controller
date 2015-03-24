/*
 * sm_fx3uart.h
 *
 *  Created on: Mar 21, 2015
 *      Author: Cameron Karlsson
 */

#ifndef SMFX3UART_H_
#define SMFX3UART_H_

#include "state_machine.h"
#include "blade_hardware.h"


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


//========================================================
//========================================================
//========= Specialize for FX3 UART host comm ============
//========================================================
//========================================================
//               sm_fx3uart
//

// Declare the available MODEs here to avoid naming collisions with enums.
static struct {
	const int READY;
	const int CONFIGMODE;
	const int READING;
	const int FINISHED;
} sm_fx3uart_mode = {
		.READY 		= 0,
		.CONFIGMODE	= 1,
		.READING 	= 2,
		.FINISHED 	= 3,
};
#define SM_FX3UART_NUM_MODES (sizeof(sm_fx3uart_mode)/sizeof(int))






// Declare state container for the state machine
typedef struct {
	// Private data should go in here!
	uint8_t isread;
	uint8_t iswrite;
	uint8_t dev_id;
	uint8_t cmd_count;
	uart_pkt pkt;
	uart_cmd cmds[7];
	uint8_t bytes[16];

	int (*mode_handlers[SM_FX3UART_NUM_MODES])(state_machine* sm, uint8_t b);		// <--- !! Make sure there are enough spaces for the handlers!
} sm_fx3uart_state;

// hide them from outside (except they are not in a c file... do it later)
static int sm_fx3uart_mode_READY_handler(state_machine* sm, uint8_t b);
static int sm_fx3uart_mode_CONFIGMODE_handler(state_machine* sm, uint8_t b);
static int sm_fx3uart_mode_READING_handler(state_machine* sm, uint8_t b);
static int sm_fx3uart_mode_FINISHED_handler(state_machine* sm, uint8_t b);

static void sm_fx3uart_EXECUTE_COMMANDS(state_machine* sm);


// These should really go in another c file...
//========================================================
//========================================================
//===================== Definitions ======================
//========================================================
//========================================================



state_machine sm_fx3uart_new(sm_fx3uart_state* container){

	// reset state machine
	state_machine sm;
	sm.state = (void*)container;
	sm.mode = sm_fx3uart_mode.READY;	//ready, mode 1 = finished
	sm.byte_num = 0;

	// reset container
	container->cmd_count = 0;
	container->dev_id = 0;
	container->isread = 0;
	container->iswrite = 0;
	container->pkt.magic = 0;
	container->pkt.mode = 0;


	// ASSIGN THE HANDLERS FOR EACH MODE!!!!!
	container->mode_handlers[sm_fx3uart_mode.READY] = sm_fx3uart_mode_READY_handler;
	container->mode_handlers[sm_fx3uart_mode.CONFIGMODE] = sm_fx3uart_mode_CONFIGMODE_handler;
	container->mode_handlers[sm_fx3uart_mode.READING] = sm_fx3uart_mode_READING_handler;
	container->mode_handlers[sm_fx3uart_mode.FINISHED] = sm_fx3uart_mode_FINISHED_handler;

	// copy the handlers into the state machine
	sm.mode_handlers = container->mode_handlers;

	return sm;
}


// Called on magic byte
int sm_fx3uart_mode_READY_handler(state_machine* sm, uint8_t b){

	DLOG("%s() - [start]\n", __FUNCTION__);

	sm_fx3uart_state* state = (sm_fx3uart_state*)sm->state;

	state->cmd_count = 0;
	state->dev_id = 0;
	state->isread = 0;
	state->iswrite = 0;

	uart_cmd zerocmd = {0};
	int i = 0;
	for(i = 0; i < 7; ++i){
		state->cmds[i] = zerocmd;
	}
	for(i = 0; i < 16; ++i){
		state->bytes[i] = 0;
	}
	state->bytes[0] = b;

	// save it for execute
	state->pkt.magic = b;

	//alt_printf("magic byte: %x -- %x\n",(int)sm->byte_num, (int)b);

	// Set current mode to configmode
	sm->mode = sm_fx3uart_mode.CONFIGMODE;
	// Tell router we want to continue reading.

	DLOG("%s() - [end]\n", __FUNCTION__);
	return state_machine_status.READING;
}
// Called on byte after magic byte, the config/mode byte
int sm_fx3uart_mode_CONFIGMODE_handler(state_machine* sm, uint8_t b){
	DLOG("%s() - [start]\n", __FUNCTION__);

	sm_fx3uart_state* state = (sm_fx3uart_state*)sm->state;
	state->bytes[sm->byte_num] = b;


	// save it for execute
	state->pkt.mode = b;

	// extract mode information
	state->isread = (b & UART_PKT_MODE_DIR_MASK) == UART_PKT_MODE_DIR_READ;
	state->iswrite = (b & UART_PKT_MODE_DIR_MASK) == UART_PKT_MODE_DIR_WRITE;
	state->dev_id = (b & UART_PKT_MODE_DEV_MASK);
	state->cmd_count = b & UART_PKT_MODE_CNT_MASK;

	DSHOW(state->isread);
	DSHOW(state->iswrite);
	DSHOW(state->dev_id);
	DSHOW(state->cmd_count);

	if(state->cmd_count > 7){
		state->cmd_count = 7;
	}

	//alt_printf("mode byte: %x -- %x\n",(int)sm->byte_num, (int)b);


	// Set current mode to reading
	sm->mode = sm_fx3uart_mode.READING;
	DLOG("%s() - [end]\n", __FUNCTION__);
	// Tell router we want to continue reading.
	return state_machine_status.READING;
}
// Called on every byte after the mode byte
int sm_fx3uart_mode_READING_handler(state_machine* sm, uint8_t b){
	DLOG("%s() - [start]\n", __FUNCTION__);

	sm_fx3uart_state* state = (sm_fx3uart_state*)sm->state;
	state->bytes[sm->byte_num] = b;

	//alt_printf("byterr: %x\n",(int)sm->byte_num);


	// basically copying the next incoming bytes into the command buffer.
	// -2 because we skipped the magic and mode bytes.
	uint8_t* loc = ((uint8_t*)state->cmds) + (sm->byte_num-2);
	*loc = b;


	// got them all... from 0, 15 is last one
	if(sm->byte_num >= 15){
		sm->mode = sm_fx3uart_mode.FINISHED;
	}

	//alt_printf("read byte: %x -- %x\n",(int)sm->byte_num, (int)b);

	if (sm->mode == sm_fx3uart_mode.FINISHED){
		DLOG("%s() - [end-finished]\n", __FUNCTION__);
		return sm_fx3uart_mode_FINISHED_handler(sm, b);		// is called on the *same* last byte
	} else {
		// Tell router we want to continue reading.
		DLOG("%s() - [end-continue]\n", __FUNCTION__);
		return state_machine_status.READING;
	}
}
// Called after last byte
int sm_fx3uart_mode_FINISHED_handler(state_machine* sm, uint8_t b){
	DLOG("%s() - [start]\n", __FUNCTION__);

	sm_fx3uart_state* state = (sm_fx3uart_state*)sm->state;
	state->bytes[sm->byte_num] = b;

	sm_fx3uart_EXECUTE_COMMANDS(sm);

	uart_cmd* cmds = state->cmds;
	DLOG("%s...Echo cmds\n","");

	blade.devices.host.uart_write(state->pkt.magic);
	blade.devices.host.uart_write(state->pkt.mode);

	int i = 0;
	for (i = 0; i < state->cmd_count; i++) {
		blade.devices.host.uart_write(cmds[i].addr);
		blade.devices.host.uart_write(cmds[i].data);
	}
	for (i = 2 + state->cmd_count * sizeof(uart_cmd); (i % 16); i++) {
		blade.devices.host.uart_write(0xff);
	}

	// Set state as ready for another byte, if it shows up later.
	sm->mode = sm_fx3uart_mode.READY;
	// Tell router we are done with the current stream.

	DLOG("%s() - [end]\n", __FUNCTION__);
	return state_machine_status.DONE;
}


// ===========================================
// ===========================================
// ===========================================


// executes all commands that are saved in the state command buffer
void sm_fx3uart_EXECUTE_COMMANDS(state_machine* sm){
	DLOG("%s() - [start]\n", __FUNCTION__);

	sm_fx3uart_state* state = (sm_fx3uart_state*)sm->state;

	if(state->cmd_count == 0){
		return;
	}

	static uint32_t collect_bytes = 0;		// used in GPIO write
	uart_cmd* cmds = state->cmds;

	int i = 0;
	switch (state->dev_id){
	// --------------------------------------------------
	case UART_PKT_DEV_LMS:

		if(state->isread){
			for(i = 0; i < state->cmd_count; ++i){
				blade.devices.lms.spi_read( cmds[i].addr, &(cmds[i].data) );
			}
		} else if(state->iswrite){
			for(i = 0; i < state->cmd_count; ++i){
				blade.devices.lms.spi_write( cmds[i].addr, cmds[i].data );
				cmds[i].data = 0;
			}
		} else {
			for(i = 0; i < state->cmd_count; ++i){
				cmds[i].addr = 0;
				cmds[i].data = 0;
			}
		}

		break;
	// --------------------------------------------------
	case UART_PKT_DEV_SI5338:

		if(state->isread){
			for(i = 0; i < state->cmd_count; ++i){
				blade.devices.si5338.read( cmds[i].addr, &(cmds[i].data) );
			}
		} else if(state->iswrite){
			for(i = 0; i < state->cmd_count; ++i){
				blade.devices.si5338.write( cmds[i].addr, cmds[i].data );
				cmds[i].data = 0;
			}
		} else {
			for(i = 0; i < state->cmd_count; ++i){
				cmds[i].addr = 0;
				cmds[i].data = 0;
			}
		}

		break;
	// --------------------------------------------------
	case UART_PKT_DEV_GPIO:

		if(state->isread){
			// lookup the gpio procedure from the given address
			blade_device_procedure proc = blade_find_gpio_procedure(  cmds[0].addr  );
			for(i = 0; i < state->cmd_count; ++i){
				if(!proc.read){
					continue;
				}
				// Converts the proc address to a byte offset.
				// Check backend/usb.c -> access_peripheral() & gpio_read() for explanation.
				uint8_t offset = cmds[i].addr - proc.address;

				// Read the property and shift the data into the response command.
				uint32_t result = proc.read(offset, cmds[i].data);
				cmds[i].data = result & 0xff;
				cmds[i].addr = offset;
			}
		} else if(state->iswrite){
			// lookup the gpio procedure from the given address
			blade_device_procedure proc = blade_find_gpio_procedure(  cmds[0].addr  );

			// Collect the bytes into a 32bit variable, over multiple 'magic' transactions.
			// Needed because write hardware functions need to be called once with up to a full 32bit param.
			for(i = 0; i < state->cmd_count; ++i){
				if(!proc.write){
					continue;
				}

				uint8_t offset = cmds[i].addr - proc.address;
				bool last_byte = offset == proc.parameter_bytecount-1;

				if(proc.requires_collected_write_bytes){
					// copy the byte in this command to the corresponding offset in collect_bytes
					collect_bytes &= ~ ( 0xff << ( 8 * offset));
					collect_bytes |= cmds[i].data << (8 * offset);

					// after all bytes needed for the procedure have been collected,
					if (last_byte) {
						proc.write(0, collect_bytes);
						collect_bytes = 0;
					}
				} else {
					proc.write(offset, cmds[i].data);
				}

				cmds[i].data = 0;
				cmds[i].addr = offset;
			}

		} else {
			for(i = 0; i < state->cmd_count; ++i){
				cmds[i].addr = 0;
				cmds[i].data = 0;
			}
		}

		break;
	// --------------------------------------------------
	default:
		break;	//nothing
	}// switch

	// --------------------------------------------------
	//echo the commands back to the host
	DLOG("%s() - [end]\n", __FUNCTION__);

	return;
}


#endif /* SMFX3UART_H_ */
