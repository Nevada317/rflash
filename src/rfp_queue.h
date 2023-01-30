#ifndef _RFP_QUEUE_H
#define _RFP_QUEUE_H

#include "rfp.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct rfp_executor_status_t rfp_executor_status_t;
struct rfp_executor_status_t {
	rfp_list_t* ActiveTask;
	rfp_task_status_t Estimated;
	rfp_task_status_t Returned;
	bool TaskNumberMismatch;

};

void RFP_Queue_Init();
void RFP_Queue_StartTask(rfp_list_t* FirstItem);


#endif /* _RFP_QUEUE_H */
