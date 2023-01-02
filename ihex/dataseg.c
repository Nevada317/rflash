#include "dataseg.h"
#include <stdlib.h>
#include <string.h>

// Create new root instance on datasegment_t and return pointer to it
datasegment_t* DATASEG_CreateRoot() {
	return calloc(1, sizeof(datasegment_t));
}

// Add new record to the end of list and return pointer to it
datasegment_t* DATASEG_Append(datasegment_t* root) {
	datasegment_t* tail = DATASEG_GetTail(root);
	datasegment_t* added = calloc(1, sizeof(datasegment_t));
	tail->next = added;
	added->prev = tail;
	added->ptr_root = tail->ptr_root;
	return added;
}

// Add new record to the end of list  and return pointer to it (or create new list)
datasegment_t* DATASEG_AutoAppend(datasegment_t** ptr_root) {
	if (!ptr_root) return 0;

	if (*ptr_root) {
		// Append
		return DATASEG_Append(*ptr_root);
	} else {
		// New root
		*ptr_root = DATASEG_CreateRoot();
		(*ptr_root)->ptr_root = ptr_root;
		return *ptr_root;
	}
}

// Returns real root pointer given any element of list
datasegment_t* DATASEG_GetRoot(datasegment_t* record) {
	if (!record) return 0;
	datasegment_t* ptr = record;
	while (ptr->prev) {ptr = ptr->prev;}
	return ptr;
}

// Returns real tail pointer given any element of list
datasegment_t* DATASEG_GetTail(datasegment_t* record) {
	if (!record) return 0;
	datasegment_t* ptr = record;
	while (ptr->next) {ptr = ptr->next;}
	return ptr;
}


// Allocate data segment in given list item
datasegment_t* DATASEG_Alloc(datasegment_t* record, uint32_t Offset, uint32_t Length) {
	DATASEG_Dealloc(record); // If it was already allocated
	record->Offset = Offset;
	record->Payload = calloc(Length, 1);
	record->Length = Length;
	return record;
}

// Change length of data segment
datasegment_t* DATASEG_Extend(datasegment_t* record, uint32_t NewLength) {
	if (!record) return 0;
	if (!NewLength) return DATASEG_Dealloc(record);

	uint32_t OldLength = record->Length;
	if (OldLength == NewLength) return record;

	if (!record->Payload) OldLength = 0;
	record->Payload = realloc(record->Payload, NewLength);
	if (OldLength < NewLength) {
		memset(record->Payload + OldLength, 0, NewLength - OldLength);
	}
	record->Length = NewLength;
	return record;
}

// Reduce length of data segment
datasegment_t* DATASEG_ShiftLeft(datasegment_t* record, uint32_t CutLength) {
	if (!record) return 0;
	if (record->Length <= CutLength) return DATASEG_Dealloc(record);
	if (!record->Payload) return DATASEG_Dealloc(record);
	memmove(record->Payload, record->Payload + CutLength, record->Length - CutLength);
	record->Length -= CutLength;
	record->Offset += CutLength;
	record->Payload = realloc(record->Payload, record->Length);
	return record;
}


// Deallocate data segment in given list item
datasegment_t* DATASEG_Dealloc(datasegment_t* record) {
	if (!record) return 0;
	if (record->Payload) {
		free(record->Payload);
		record->Payload = 0;
	}
	record->Length = 0;
	return record;
}

// Get array of pointers for all elements of linked-list.
// Array is NULL-pointer terminated and
// count is also returned to given variable, if it is a pointer.
datasegment_t** DATASEG_Enumerate(datasegment_t* root, uint32_t* count) {
	datasegment_t* real_root = DATASEG_GetRoot(root);
	if (count) *count = 0;
	if (!real_root) return 0;
	uint32_t int_count = 0;

	datasegment_t* ptr = real_root;
	while (ptr) {
		int_count++;
		ptr = ptr->next;
	}

	datasegment_t** arr = calloc(int_count+1, sizeof(datasegment_t*));

	uint32_t i = 0;
	ptr = real_root;
	while (ptr) {
		arr[i++] = ptr;
		ptr = ptr->next;
	}

	if (count) *count = int_count;
	return arr;
}

// Destroy non-allocated data segments from list
datasegment_t* DATASEG_Cleanup(datasegment_t* root) {
	datasegment_t* real_root = DATASEG_GetRoot(root);
	datasegment_t* ptr = real_root;
	while (ptr) {
		if (!ptr->Payload) {
			ptr = DATASEG_Remove(ptr);
			continue;
		}
		ptr = ptr->next;
	}
	return real_root;
}

// Removes item from list (even if it is not first or last one) and
// returns pointer to prev (or next) item
datasegment_t* DATASEG_Remove(datasegment_t* record) {
	if (!record) return 0;
	datasegment_t* prev = record->prev;
	datasegment_t* next = record->next;
	if (next) next->prev = prev;
	if (prev) prev->next = next;

	if (*(record->ptr_root) == record) {
		// Removing root record
		*(record->ptr_root) = next;
	}

	DATASEG_Dealloc(record);
	free(record);

	if (prev) {
		return prev;
	} else {
		return next;
	}
}
