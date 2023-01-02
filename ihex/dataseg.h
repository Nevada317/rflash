#ifndef _DATASEG_H
#define _DATASEG_H

#include <stdint.h>

typedef struct datasegment_t datasegment_t;

struct datasegment_t {
	datasegment_t* prev;
	datasegment_t* next;
	uint32_t Offset;
	uint32_t Length;
	void* Payload;
};

// Create new root instance on datasegment_t and return pointer to it
datasegment_t* DATASEG_CreateRoot();
// Add new record to the end of list and return pointer to it
datasegment_t* DATASEG_Append(datasegment_t* root);
// Add new record to the end of list  and return pointer to it (or create new list)
datasegment_t* DATASEG_AutoAppend(datasegment_t** ptr_root);

// Returns real root pointer given any element of list
datasegment_t* DATASEG_GetRoot(datasegment_t* record);
// Returns real tail pointer given any element of list
datasegment_t* DATASEG_GetTail(datasegment_t* record);

// Allocate data segment in given list item
datasegment_t* DATASEG_Alloc(datasegment_t* record, uint32_t Offset, uint32_t Length);
// Deallocate data segment in given list item
datasegment_t* DATASEG_Dealloc(datasegment_t* record);

// Get array of pointers for all elements of linked-list.
// Array is NULL-pointer terminated and
// count is also returned to given variable, if it is a pointer.
// NOTE: you NEED to free returned array
datasegment_t** DATASEG_Enumerate(datasegment_t* root, uint32_t* count);

// Destroy non-allocated data segments from list
datasegment_t* DATASEG_Cleanup(datasegment_t* root);
// Removes item from list (even if it is not first or last one) and
// returns pointer to prev (or next) item
// NOTE: Be careful not to remove root item
datasegment_t* DATASEG_Remove(datasegment_t* record);

#endif /* _DATASEG_H */
