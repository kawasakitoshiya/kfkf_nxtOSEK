/*
####################################################################################################
	name: kfkfModel.c
	Description: ???
	---
	update: 2013.06.13
####################################################################################################
*/

#include <stdlib.h>

#include "kfkfModel.h"

/*
===============================================================================================
	Definition
===============================================================================================
*/
/* Buffer size for Bluetooth */
#define BT_RCV_BUF_SIZE 32
/* The number of event */
#define RESERVED_EVENT_SIZE 3000
/* The number of state */
#define RESERVED_STATES_SIZE 900

/* Definition of Structure of state machine */
typedef struct tag_StateMachine {
	S16 num_of_events;
	S16 num_of_states;
	EvtType_e *events;
	State_t *states;
	S16 current_state;
} StateMachine_t;

/*
===============================================================================================
	Variables
===============================================================================================
*/
/* Buffer for Bluetooth */
U8 bt_receive_buf[BT_RCV_BUF_SIZE];
/* State Machine for kfkf Model */
static StateMachine_t g_StateMachine;

/*
===============================================================================================
	Functions
===============================================================================================
*/
void InitStateMachine(void)
{
	g_StateMachine.num_of_events = 0;
	g_StateMachine.num_of_states = 0;
	g_StateMachine.current_state = 0;

	free( g_StateMachine.events );
	g_StateMachine.events = NULL;

	free( g_StateMachine.states );
	g_StateMachine.states = NULL;
}

/*
===============================================================================================
	name: receive_BT
	Description: ??
	Parameter: no
	Return Value: no
===============================================================================================
*/
void ReceiveBT(void){
	
	S16 events[RESERVED_EVENT_SIZE];
	S16 states[RESERVED_STATES_SIZE];
	
    U16 packet_no = 1;	//packet number
    U16 ptr = 0;

    U16 i = 0;

    display_clear(0);
    display_goto_xy(0, 1);
    display_string("Pre:Bluetooth");
    display_update();
    ecrobot_sound_tone(8800, 50, 30);
    
// packet type:1 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    while(1){
    	// Receive first packet.
    	systick_wait_ms(100);

        ecrobot_read_bt_packet(bt_receive_buf, BT_RCV_BUF_SIZE);
		
        if(bt_receive_buf[0] == packet_no && bt_receive_buf[1] == 1)
        {
        	g_StateMachine.num_of_states = bt_receive_buf[2];
        	g_StateMachine.num_of_events = bt_receive_buf[3];
			
            packet_no++;
            break;
        }
    }

// packet type:2 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    display_clear(0);
    display_goto_xy(0, 1);
    display_string("Pre:Bluetooth");
    display_goto_xy(0, 2);
    display_string("packet:event");
    display_update();

    ptr = 0;

    while(1/*ptr+14 <num_of_states*num_of_events*/)
    {
        systick_wait_ms(100);
        
        ecrobot_read_bt_packet(bt_receive_buf, BT_RCV_BUF_SIZE);

        if(bt_receive_buf[1] == 3)
        {
            break;
        }

        if(bt_receive_buf[0] == packet_no && bt_receive_buf[1] == 2)
        {
            for(i=2;i<16;i++)
            {
                *(events+ptr) = *(bt_receive_buf+i);
                ptr++;
            }
            packet_no++;
        }
    }

    g_StateMachine.events = (EvtType_e *)malloc(ptr);
    if(g_StateMachine.events == NULL)
    {
        display_clear(0);
        display_goto_xy(0, 1);
        display_string("Pre:Bluetooth");
        display_goto_xy(0, 2);
        display_string("Malloc Err:event");
        display_update();
    }

    for(i=0;i<ptr;i++)
    {
    	g_StateMachine.events[i] = (EvtType_e)events[i];
    }
    

// packet type:3 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    display_clear(0);
    display_goto_xy(0, 1);
    display_string("Pre:Bluetooth");
    display_goto_xy(0, 2);
    display_string("end packet:event");
    display_goto_xy(0, 3);
    display_string("packet:state");
    display_update();

    ptr = 0;

    g_StateMachine.states = (State_t *)malloc(g_StateMachine.num_of_states);
    if(g_StateMachine.states == NULL)
    {
        display_clear(0);
        display_goto_xy(0, 1);
        display_string("Pre:Bluetooth");
        display_goto_xy(0, 2);
        display_string("end packet:event");
        display_goto_xy(0, 3);
        display_string("Malloc Err:state");
        display_update();
    }

    while(1/*ptr+14 <(2+4)*num_of_states*/){

        int i=0;
        systick_wait_ms(100);

        if(ptr!=0)
        {
            ecrobot_read_bt_packet(bt_receive_buf, BT_RCV_BUF_SIZE);
        }
        if(bt_receive_buf[1] == 255)
        {
            break;
        }
        
        if(bt_receive_buf[0] == packet_no && bt_receive_buf[1] == 3)
        {
            for(i=2;i<16;i++)
            {
                *(states+ptr) = *(bt_receive_buf+i);
                ptr++;
            }
            packet_no++;
        }
    }
    
    for(i=0;i<ptr;i=i+6)
    {
    	g_StateMachine.states[i].state_no = states[i];
    	g_StateMachine.states[i+1].action_no = (ActType_e)states[i+1];
    	g_StateMachine.states[i+2].value0 = states[i+2];
    	g_StateMachine.states[i+3].value1 = states[i+3];
    	g_StateMachine.states[i+4].value2 = states[i+4];
    	g_StateMachine.states[i+5].value3 = states[i+5];
    }

    display_clear(0);
    display_goto_xy(0, 1);
    display_string("Pre:Bluetooth");
    display_goto_xy(0, 2);
    display_string("end packet:event");
    display_goto_xy(0, 3);
    display_string("end packet:state");
    display_update();

    g_StateMachine.current_state = 0;

}

/*
===============================================================================================
	name: get_CurrentState
	Description: ??
	Parameter: no
	Return Value: g_StateMachine.current_state
===============================================================================================
*/
S16 getCurrentState()
{
	return g_StateMachine.current_state;
}

/*
===============================================================================================
	name: set_NextState(befote:sendevent)
	Description: ??
	Parameter: event_id:S8
	Return Value:
===============================================================================================
*/
#define NO_STATE -1

State_t setNextState(EvtType_e event_id) {
	S8 next_state = NO_STATE;
	//S16 i = 0;
	State_t state = {-1,-1,0,0,0,0};

	if(g_StateMachine.events != NULL)
	{
		next_state = g_StateMachine.events[event_id + g_StateMachine.current_state * g_StateMachine.num_of_events];
	}

	if(next_state != NO_STATE)
	{
		g_StateMachine.current_state = next_state;
		state = g_StateMachine.states[g_StateMachine.current_state];
	}

/*
	for(i = 0;i < g_StateMachine.num_of_states;i++) {
		if(i == g_StateMachine.current_state) {
			ControllerSet(&g_StateMachine.states[i]);
			return 1;
		}
	}
*/
	return state;
}

/*
===============================================================================================
	name: BluetoothStart
	Description: ??
	Parameter: no
	Return Value: S8
===============================================================================================
*/
S8 BluetoothStart(void)
{
	U8 btstart = OFF;

	ecrobot_read_bt_packet(bt_receive_buf, BT_RCV_BUF_SIZE);
	if(bt_receive_buf[1] == 254 )
	{
		btstart = ON;
	}
	else
	{
		btstart = OFF;
	}

	return btstart;
}
