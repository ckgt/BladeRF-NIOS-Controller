/*
 * packet_router.h
 *
 *  Created on: Mar 21, 2015
 *      Author: Cameron Karlsson
 *
 *
 */

/*  ** What's the point?!?! **
 *
 * With many groups using the Blade, some will want to add their own
 * expansion hardware, etc. In that case, custom modifications to the NIOS
 * will need to be made. Hopefully, this will make modifications more
 * modular and easier to implement. It did for me.
 *
 * This router allows different state machines to exist that each have
 * different 'magic' starting values.
 *
 * This basically allows multiple specialized parsing, 
 * instead of a single do-all parsing state machine.
 * 
 */

#ifndef PACKET_ROUTER_H_
#define PACKET_ROUTER_H_

#include "simple_types.h"
#include "state_machine.h"

#define MAX_STATE_MACHINES 1
// Make sure to change MAX_STATE_MACHINES when you plan on adding others.

typedef struct {
	state_machine* current_machine;
	char magics[MAX_STATE_MACHINES];
	state_machine* machines[MAX_STATE_MACHINES];			//machine handler returns true - working, false, just ended.
} packet_router;



packet_router packet_router_new(){

	packet_router router = {0};
	router.current_machine = NULL;

	int i = 0;
	for(i = 0; i < MAX_STATE_MACHINES; i++){
		router.machines[i] = 0;
	}
	for(i = 0; i < MAX_STATE_MACHINES; i++){
		router.magics[i] = 0;
	}

	return router;
}
void packet_router_register_machine(packet_router* r, state_machine* sm, int ID, char magic){
	if(ID >= MAX_STATE_MACHINES){
		return;
	}

	r->machines[ID] = sm;
	r->magics[ID] = magic;
}

void packet_router_step_machine(packet_router* r, state_machine* sm, uint8_t b){

	// Calling the current state handler according to mode index.
	// No silly switch statement.
	int sts = state_machine_handle_byte(sm, b);
	sm->byte_num++;
	if(sts == state_machine_status.DONE){	//machine is finished reading and expects no more data
		sm->byte_num = 0;
		r->current_machine = NULL;
	}

}
void packet_router_handle_byte(packet_router* r, uint8_t b){

	//no machine active... looking for magic
	if(r->current_machine == NULL){
		int i = 0;
		for(i = 0; i < MAX_STATE_MACHINES; ++i){
			if(r->magics[i] != 0 && b == r->magics[i] ){

				// make sure machine is registered for that magic
				if(r->machines[i] != NULL){
					r->current_machine = r->machines[i];
					packet_router_step_machine( r, r->current_machine, b );
				}
				break;
				// next evaluation of handle_byte will go to else statement
				// to call the current_machine handler.
			}
		}
	} else {	//reading bytes

		packet_router_step_machine( r, r->current_machine, b );

	}

}




#endif /* PACKET_ROUTER_H_ */
