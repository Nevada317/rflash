#ifndef _RFP_QUEUE_H
#define _RFP_QUEUE_H

#include "rfp.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct rfp_executor_status_t rfp_executor_status_t;
struct rfp_executor_status_t {
	uint8_t ExecutorNumber;
	rfp_list_t* ActiveTask;
	uint8_t Number_Assigned;
	rfp_task_status_t Status_Estimated;
	volatile uint8_t Number_Reported;
	volatile rfp_task_status_t Status_Reported;
	enum rfp_queue_action_t {
		// Below 128 - passive
		RFP_Queue_Action_Unassigned = 0,
		RFP_Queue_Action_WaitExecution,
		// Equal 128 - active poll request
		RFP_Queue_Action_Poll = 128,
		// Above 128 - action
		RFP_Queue_Action_Result,
		RFP_Queue_Action_Release,
		RFP_Queue_Action_ReadData,
		RFP_Queue_Action_SendData_Add0,
		RFP_Queue_Action_SendData_Add1,
		RFP_Queue_Action_SendData_Add2,
		// Higher the action value = higher priority of execution
	} Action;
};

void RFP_Queue_Init();
void RFP_Queue_StartTask(rfp_list_t* FirstItem);
void RFP_Queue_Wait();

void RFP_Queue_SetTxFunction(void (*cb)(void* data, int length));
void RFP_Queue_RXC_Buffer(void* data, int length); // Raw TCP/UART frame with STX etc.
void RFP_Queue_RXC_Packet(void* data, int length); // Handle pre-checked parcel


#endif /* _RFP_QUEUE_H */
