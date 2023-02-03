#include "rfp_queue.h"
#include "sleep.h"
#include "rfp_transport_decoder.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define RFP_QUEUE_EXECUTORS_COUNT 2

static pthread_t RFP_Queue_Executor = 0;
static void* RFP_WorkerLoop(void*);

static void RFP_Queue_TryToAssignNewTask(rfp_executor_status_t* executor);
static void RFP_Queue_HandleTaskStatus(rfp_executor_status_t* executor);
static void RFP_Queue_SendRequest(rfp_executor_status_t* executor, enum rfp_queue_action_t action);
static void RFP_HandlePollResult(const uint8_t idx, const uint8_t status);

static void _sleep_step();

void (*RFP_Queue_TxFunction)(void* data, int length) = 0;

static rfp_executor_status_t Executor[2] = {0,};
static uint8_t NewTaskId = 0;
static rfp_list_t* NextUnassignedTask = 0;
static volatile bool RFP_Queue_CancelFlag = false;

void RFP_Queue_Init() {
	memset(Executor, 0, sizeof(Executor));
	for (uint8_t i = 0; i < RFP_QUEUE_EXECUTORS_COUNT; i++) {
		Executor[i].ExecutorNumber = i;
	}
	NewTaskId = 0;
	NextUnassignedTask = 0;
	RFP_Transport_Decode_SetCallback(RFP_Queue_RXC_Packet);
}

void RFP_Queue_StartTask(rfp_list_t* FirstItem) {
	// TODO: Possibly wait for prev task execution

	NextUnassignedTask = FirstItem;
	RFP_Queue_CancelFlag = false;
	pthread_create(&RFP_Queue_Executor, 0, RFP_WorkerLoop, 0);
}

void RFP_Queue_SetTxFunction(void (*cb)(void* data, int length)) {
	RFP_Queue_TxFunction = cb;
}

void RFP_Queue_RXC_Buffer(void* data, int length) { // Raw TCP/UART frame with STX etc.
	RFP_Transport_Decode_Block(data, length);
}

void RFP_Queue_RXC_Packet(void* data, int length) { // Handle pre-checked parcel
	printf("I AM HERE!\n");
}

void RFP_Queue_Wait() {
	// pthread_cancel(RFP_Queue_Executor);
	pthread_join(RFP_Queue_Executor, NULL);
}


static void* RFP_WorkerLoop(void*) {
	while (1) {
		for (uint8_t i = 0; i < RFP_QUEUE_EXECUTORS_COUNT; i++) {
			RFP_Queue_TryToAssignNewTask(&Executor[i]);
			RFP_Queue_HandleTaskStatus(&Executor[i]);
		}

		enum rfp_queue_action_t PriorityAction = RFP_Queue_Action_Unassigned;
		uint8_t PriorityExecutor = 0;

		for (uint8_t i = 0; i < RFP_QUEUE_EXECUTORS_COUNT; i++) {
			if (Executor[i].Action > PriorityAction) {
				PriorityAction = Executor[i].Action;
				PriorityExecutor = i;
			}
		}

		switch (PriorityAction) {
			case RFP_Queue_Action_Unassigned:
				printf("BUG: This should not be reached! RFP_Queue_Action_Unassigned\n");
				_sleep_step();
				break;
			case RFP_Queue_Action_WaitExecution:
				_sleep_step();
				break;
			case RFP_Queue_Action_Result:
				// TODO: Remove result
				_sleep_step();
				break;
			case RFP_Queue_Action_Poll:
			case RFP_Queue_Action_Release:
			case RFP_Queue_Action_ReadData:
			case RFP_Queue_Action_SendData_Add0:
			case RFP_Queue_Action_SendData_Add1:
			case RFP_Queue_Action_SendData_Add2:
				RFP_Queue_SendRequest(&Executor[PriorityExecutor], PriorityAction);
				break;
		}
	}
	return 0;
}

static void RFP_Queue_TryToAssignNewTask(rfp_executor_status_t* executor) {
	if (!NextUnassignedTask) return;
	if (executor->ActiveTask) return;

	rfp_list_t* newTask = NextUnassignedTask;
	NextUnassignedTask = NextUnassignedTask->next;

	executor->ActiveTask = newTask;
	executor->Number_Assigned = NewTaskId;
	executor->Number_Reported = 0xE0; // Impossible dummy
	executor->Status_Estimated = RFP_Task_None;
	executor->Status_Reported = RFP_Task_Unknown;

	printf("Task #%d assigned to worker %d\n", newTask->index, executor->ExecutorNumber);

	NewTaskId = (NewTaskId + 1) & 127;
}


static void RFP_Queue_HandleTaskStatus(rfp_executor_status_t* E) {
	if (!E->ActiveTask) return;
	rfp_task_status_t reported = E->Status_Reported;
	if (E->Number_Reported != E->Number_Assigned) {
		reported = RFP_Task_None;
		// TODO: Uncomment after testing
		// E->Status_Estimated = RFP_Task_None;
	}

	do { // Breakable block
		if (E->Status_Estimated == RFP_Task_Unknown) {
			E->Status_Estimated = RFP_Task_None;
			// TODO: Add task reset
		}
		if (E->Status_Estimated == RFP_Task_None) {
			E->Action = RFP_Queue_Action_SendData_Add0;
			break;
		}
		if (E->Status_Estimated == RFP_Task_GotAdd0) {
			E->Action = RFP_Queue_Action_SendData_Add1;
			break;
		}
		if (E->Status_Estimated == RFP_Task_GotAdd1) {
			E->Action = RFP_Queue_Action_SendData_Add2;
			break;
		}
		if ((E->Status_Estimated == RFP_Task_Filled) && (reported < RFP_Task_Filled)) {
			E->Action = RFP_Queue_Action_Poll;
			break;
		}
		if ((E->Status_Estimated >= RFP_Task_Filled) && (reported >= RFP_Task_Filled) && (reported < RFP_Task_Finished)) {
			E->Action = RFP_Queue_Action_WaitExecution;
			E->Status_Estimated = RFP_Task_Process;
			break;
		}
		if ((E->Status_Estimated >= RFP_Task_Filled) && (reported >= RFP_Task_Finished)) {
			E->Action = RFP_Queue_Action_Result;
			break;
		}
		printf("Task #%d NOT handled - status est %02x, rpt %02x\n", E->Number_Assigned, E->Status_Estimated, reported);
	} while (0);
	// printf("Task #%d handled. It is %d now\n", E->ActiveTask->index, E->Action);
}


static void RFP_Queue_SendRequest(rfp_executor_status_t* executor, enum rfp_queue_action_t action) {
	if (!executor) return;
	rfp_flexbuffer_t * flex = 0;
	uint8_t tasknum = RFP_TaskNumber_Unassigned;
	rfp_buffer_t * bfr = 0;
	if (executor && executor->ActiveTask) bfr = &executor->ActiveTask->Buffer;
	if (executor) tasknum = executor->Number_Assigned;

	switch (action) {
		case RFP_Queue_Action_Result:
		case RFP_Queue_Action_Unassigned:
		case RFP_Queue_Action_WaitExecution:
			// Do nothing
			break;
		case RFP_Queue_Action_Poll:
			printf("Worker %d: RFP_Queue_Action_Poll\n", executor->ExecutorNumber);
			flex = RFP_CreateParcel(RFP_CMD_POLL, tasknum, bfr);
			_sleep_step();
			break;
		case RFP_Queue_Action_Release:
			printf("Worker %d: RFP_Queue_Action_Release\n", executor->ExecutorNumber);
			_sleep_step();
			break;
		case RFP_Queue_Action_ReadData:
			printf("Worker %d: RFP_Queue_Action_ReadData\n", executor->ExecutorNumber);
			_sleep_step();
			break;
		case RFP_Queue_Action_SendData_Add0:
			executor->Status_Estimated = RFP_Task_GotAdd0;
			printf("Worker %d: RFP_Queue_Action_SendData_Add0\n", executor->ExecutorNumber);
			flex = RFP_CreateParcel(RFP_CMD_Add0, tasknum, bfr);
			_sleep_step();
			break;
		case RFP_Queue_Action_SendData_Add1:
			executor->Status_Estimated = RFP_Task_GotAdd1;
			printf("Worker %d: RFP_Queue_Action_SendData_Add1\n", executor->ExecutorNumber);
			flex = RFP_CreateParcel(RFP_CMD_Add1, tasknum, bfr);
			_sleep_step();
			break;
		case RFP_Queue_Action_SendData_Add2:
			executor->Status_Estimated = RFP_Task_Filled;
			printf("Worker %d: RFP_Queue_Action_SendData_Add2\n", executor->ExecutorNumber);
			flex = RFP_CreateParcel(RFP_CMD_Add2, tasknum, bfr);
			_sleep_step();

			// TODO: REMOVE THIS DUMMY!
			RFP_HandlePollResult(executor->Number_Assigned, RFP_Task_Filled);
			break;
	}
	if (flex) {
		// Here we should try to send data
		if (RFP_Queue_TxFunction) {
			RFP_Queue_TxFunction(flex->Data, flex->Length);
		}

		free(flex);
	}

}

static void _sleep_step() {
	fflush(stdout);
	sleep_ms(100, true);
}

static void RFP_HandlePollResult(const uint8_t idx, const uint8_t status) {
	printf("Poll request response: tid %02x now has status %02x\n", idx, status);
	rfp_executor_status_t* E = 0;
	for (uint8_t i = 0; i < RFP_QUEUE_EXECUTORS_COUNT; i++) {
		if (Executor[i].Number_Assigned == idx) {
			E = &Executor[i];
		}
	}
	if (E) {
		printf("Executor found: %p. %02x -> %02x\n", E, E->Status_Estimated, status);
		E->Status_Reported = status;
		E->Number_Reported = idx;
		if (E->Number_Reported == E->Number_Assigned) {
			if (E->Status_Estimated > E->Status_Reported) {
				E->Status_Estimated = E->Status_Reported;
			}
		}
	} else {
		printf("Executor NOT found: task %02x\n", idx);
	}

}
