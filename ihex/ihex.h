#ifndef _IHEX_H
#define _IHEX_H

#include "dataseg.h"

// Returns true if load done
bool IHEX_AppendHex(datasegment_t** ptr_root, char* filename);

#endif /* _IHEX_H */
