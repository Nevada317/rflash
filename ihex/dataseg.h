#ifndef _DATASEG_H
#define _DATASEG_H

#include <stdint.h>
#include <stdbool.h>

typedef struct datasegment_t datasegment_t;

struct datasegment_t {
	datasegment_t** ptr_root;
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
// Inserts new record after given and returns pointer to it
datasegment_t* DATASEG_InsertAfter(datasegment_t* record);

// Returns real root pointer given any element of list
datasegment_t* DATASEG_GetRoot(datasegment_t* record);
// Returns real tail pointer given any element of list
datasegment_t* DATASEG_GetTail(datasegment_t* record);

// Allocate data segment in given list item
datasegment_t* DATASEG_Alloc(datasegment_t* record, uint32_t Offset, uint32_t Length);
// Change length of data segment
datasegment_t* DATASEG_Extend(datasegment_t* record, uint32_t NewLength);
// Reduce length of data segment
datasegment_t* DATASEG_CutLeft(datasegment_t* record, uint32_t CutLength);
// Deallocate data segment in given list item
datasegment_t* DATASEG_Dealloc(datasegment_t* record);

void DATASEG_SplitRecord(datasegment_t* record, uint32_t Offset);


// Get array of pointers for all elements of linked-list.
// Array is NULL-pointer terminated and
// count is also returned to given variable, if it is a pointer.
// NOTE: you NEED to free returned array
datasegment_t** DATASEG_Enumerate(datasegment_t* root, uint32_t* count);

// Join all segments, that are neighbours
// Returns true if any merges were done
bool DATASEG_Merge(datasegment_t* root);
// Sort all segments
// Returns true if any swaps were done
bool DATASEG_Sort(datasegment_t* root);
// Fuses all segments by removing all repeats
// Returns true if any swaps were done
bool DATASEG_Fuse(datasegment_t* root);
// Destroy non-allocated data segments from list
datasegment_t* DATASEG_Cleanup(datasegment_t* root);
// Removes item from list (even if it is not first or last one) and
// returns pointer to prev (or next) item
datasegment_t* DATASEG_Remove(datasegment_t* record);

#endif /* _DATASEG_H */
