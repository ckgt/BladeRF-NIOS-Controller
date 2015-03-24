/*
 * state_machine.h
 *
 *  Created on: Mar 21, 2015
 *      Author: Cameron K.
 *
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_


// To be clear, let me explain the terminology.
// 
// State Machine 'state' - is the set of internal variables/data that are stored in the state machine.
// 
// State Machine 'mode' - is the current mode of operation and will correspond to a mode 'event handler' function.
// 
// State Machine 'status' - is telling the router whether the machine is reading or not.


struct {
	const int DONE;
	const int READING;
} state_machine_status = {
	.DONE = 0,
	.READING = 1
};


// State machine prototype to be used for implementations
typedef struct _state_machine {
	// your custom container for current state machine state information
	void* state;
	// current mode of the machine
	int mode;
	// handlers for every state
	int (**mode_handlers)(struct _state_machine* sm, uint8_t b);
	// the current byte count, 0 indexed
	int byte_num;

} state_machine;

int state_machine_handle_byte(state_machine* sm, uint8_t b){
	return sm->mode_handlers[sm->mode](sm, b);
};



#endif /* STATE_MACHINE_H_ */
