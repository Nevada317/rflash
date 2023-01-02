#include "dataseg.h"
#include <stdlib.h>
#include <string.h>

static bool _try_to_merge(datasegment_t* A);

static bool _DATASEG_Sort(datasegment_t* root);
static void _DATASEG_Swap(datasegment_t* B);
static void _DATASEG_Splice(datasegment_t* root);
static void _DATASEG_Shadow(datasegment_t* root);

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

// Inserts new record after given and returns pointer to it
datasegment_t* DATASEG_InsertAfter(datasegment_t* record) {
	datasegment_t* A = record;
	datasegment_t* B = calloc(1, sizeof(datasegment_t));
	datasegment_t* C = record->next;

	B->ptr_root = A->ptr_root;
	A->next = B;
	B->next = C;
	if (C) C->prev = B;
	B->prev = A;

	return B;
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
	if (Length) {
		record->Payload = calloc(Length, 1);
	}
	record->Length = record->Payload ? Length : 0;
	return record;
}

// Change length of data segment
datasegment_t* DATASEG_Extend(datasegment_t* record, uint32_t NewLength) {
	if (!record) return 0;
	if (!NewLength) return DATASEG_Dealloc(record);

	uint32_t OldLength = record->Length;
	if (OldLength == NewLength) return record;

	if (!record->Payload) OldLength = 0;
	void* p = realloc(record->Payload, NewLength);
	if (p) {
		record->Payload = p;
	} else if (OldLength < NewLength) {
		return 0;
	}
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

// Join all segments, that are neighbours
bool DATASEG_Merge(datasegment_t* root) {
	datasegment_t* real_root = DATASEG_GetRoot(root);

	bool result = false;
	datasegment_t* ptr1 = 0;
	datasegment_t* ptr2 = real_root;
	while (ptr2) {
		if (ptr1 && ptr2) {
			if (_try_to_merge(ptr1)) {
				result = true;
				ptr2 = ptr1->next;
				continue;
			}
		}

		ptr1 = ptr2;
		ptr2 = ptr2->next;
	}
	return result;
}

// Sort all segments
// Returns true if any swaps were done
bool DATASEG_Sort(datasegment_t* root) {
	bool res = false;
	while (_DATASEG_Sort(root)) {
		res = true;
	}
	return res;
}

// Fuses all segments by removing all repeats
// Returns true if any swaps were done
void DATASEG_Fuse(datasegment_t* root) {
	datasegment_t** ptr_root = root->ptr_root;
	DATASEG_Sort(*ptr_root);
	_DATASEG_Splice(*ptr_root);
	DATASEG_Sort(*ptr_root);
	_DATASEG_Shadow(*ptr_root);
	DATASEG_Sort(*ptr_root);
	DATASEG_Merge(*ptr_root);
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





static bool _try_to_merge(datasegment_t* A) {
	if (!A) return false;
	datasegment_t* B = A->next;
	if (!B) return false;

	// Trivial cases
	if (!A->Payload) return false;
	if (!B->Payload) {
		DATASEG_Remove(B);
		return true;
	}

	// Common case
	if ((A->Offset + A->Length) != B->Offset) return false;
	uint32_t OldLength = A->Length;
	datasegment_t* p = DATASEG_Extend(A, A->Length + B->Length);
	if (!p) return false;
	memcpy(p->Payload + OldLength, B->Payload, B->Length);
	DATASEG_Remove(B);
	return true;
}

static bool _DATASEG_Sort(datasegment_t* root) {
	datasegment_t* real_root = DATASEG_GetRoot(root);

	datasegment_t* ptr1 = 0;
	datasegment_t* ptr2 = real_root;
	while (ptr2) {
		do {
			if (!ptr1) break;
			if (!ptr2) break;
			if (ptr1->Offset <= ptr2->Offset) break;
			if (ptr1->Offset < (ptr2->Offset+ptr2->Length)) break;
			_DATASEG_Swap(ptr1);
			return true;
		} while (0);

		ptr1 = ptr2;
		ptr2 = ptr2->next;
	}
	return false;
}

static void _DATASEG_Swap(datasegment_t* B) {
	// Swaps B with next
	datasegment_t* A = B->prev;
	datasegment_t* C = B->next;
	datasegment_t* D = C->next;
	if (*B->ptr_root == B) *B->ptr_root = C;

	if (A) A->next = C;
	C->next = B;
	B->next = D;

	if (D) D->prev = B;
	B->prev = C;
	C->prev = A;
}

static void _DATASEG_Splice(datasegment_t* root) {
	datasegment_t* real_root = DATASEG_GetRoot(root);

	datasegment_t* walker;
	datasegment_t* probe = real_root;
	while (probe) {
		uint32_t offsetA = probe->Offset;
		uint32_t offsetB = probe->Offset + probe->Length;
		walker = real_root;
		while (walker) {
			DATASEG_SplitRecord(walker, offsetB);
			DATASEG_SplitRecord(walker, offsetA);
			walker = walker->next;
		}


		probe = probe->next;
	}
}

static void _DATASEG_Shadow(datasegment_t* root) {
	datasegment_t* real_root = DATASEG_GetRoot(root);

	datasegment_t* probe = real_root;
	while (probe) {
		if (probe->prev) {
			if ((probe->Offset == probe->prev->Offset) && \
				(probe->Length == probe->prev->Length)) {
				DATASEG_Remove(probe->prev);
				continue;
			}
		}
		probe = probe->next;
	}

}

void DATASEG_SplitRecord(datasegment_t* record, uint32_t Offset) {
	if (!record) return;
	if (!record->Length) return;
	if (record->Offset >= Offset) return;
	if ((record->Offset + record->Length) <= Offset) return;

	uint32_t LenA = Offset - record->Offset;
	uint32_t LenB = record->Length - LenA;

	datasegment_t* recordB = DATASEG_InsertAfter(record);
	if (!recordB) return;
	DATASEG_Alloc(recordB, Offset, LenB);
	if (!recordB->Payload) return;
	memcpy(recordB->Payload, record->Payload + LenA, LenB);
	DATASEG_Extend(record, LenA);
}

