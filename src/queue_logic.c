#include "queue_logic.h"
#include <stdlib.h>

mem_task_t* QUEUE_NewRecord(mem_task_t** root_ptr) {
	if (!root_ptr) return 0;

	mem_task_t* newrecord = calloc(1, sizeof(mem_task_t));

	mem_task_t* cur = *root_ptr;
	if (cur) {
		while (cur->next) {
			cur = cur->next;
		}
		cur->next = newrecord;
	} else {
		*root_ptr = newrecord;
	}
	return newrecord;
}

void QUEUE_Destroy(mem_task_t** root_ptr) {
	if (!root_ptr) return;
	if (!*root_ptr) return;
	while (*root_ptr) {
		mem_task_t* next = (*root_ptr)->next;
		if ((*root_ptr)->arg_string) free(((*root_ptr)->arg_string));
		if ((*root_ptr)->root_own) free(((*root_ptr)->root_own));
		free(*root_ptr);
		*root_ptr=next;
	}

}
