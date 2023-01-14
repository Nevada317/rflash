#ifndef _AVR_DEVICES_H
#define _AVR_DEVICES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct avr_signature_t {
	uint8_t Length;
	uint8_t Value[8];
} avr_signature_t;

typedef struct avr_specs_t {
	bool has_chiperase;
	bool has_lfuse;
	bool has_hfuse;
	bool has_efuse;
	bool has_lock;
	uint32_t EEPROM_Size;
	uint32_t EEPROM_PageSize;
	uint32_t FLASH_Size;
	uint32_t FLASH_PageSize;
} avr_specs_t;

typedef struct avr_device_t {
	char* id;
	char* name;
	avr_signature_t signature;
	avr_specs_t specs;
} avr_device_t;

// Returns pointer to static RAM
const avr_device_t* AVR_DEVICE_Search(char* id);

#endif /* _AVR_DEVICES_H */
