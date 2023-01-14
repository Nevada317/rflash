#ifndef _QUEUE_LOGIC_H
#define _QUEUE_LOGIC_H

#include "dataseg.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum mem_type_t {
	MEM_NONE   = 0,
	MEM_LFUSE  = 1,
	MEM_HFUSE  = 2,
	MEM_EFUSE  = 3,
	MEM_LOCK   = 4,
	MEM_FLASH  = 5,
	MEM_EEPROM = 6
} mem_type_t;

typedef enum mem_oper_t {
	MEM_OPER_NONE,
	MEM_OPER_ERASE,
	MEM_OPER_WRITE,
	MEM_OPER_VERIFY,
	MEM_OPER_READ,
} mem_oper_t;

typedef struct mem_task_t mem_task_t;
struct mem_task_t {
	mem_task_t* next;
	// Handle this flag as execution skip - data block is already joined
	bool continuation;

	mem_type_t memory_type;
	// Redundant bool flags
	bool isFuse;
	bool isArray;
	mem_oper_t memory_operation;
	uint8_t arg_byte;
	char* arg_string;
	datasegment_t* root_own;
	datasegment_t** root_ptr;
};

mem_task_t* QUEUE_NewRecord(mem_task_t** root_ptr);
void QUEUE_Destroy(mem_task_t** root_ptr);

#endif /* _QUEUE_LOGIC_H */
