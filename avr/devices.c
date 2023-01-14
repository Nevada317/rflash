#include "avr/devices.h"

static const avr_device_t AvailableParts[] = {
	{
		.id = "m162",
		.name = "ATmega162",
		.signature = {
			.Length = 3,
			.Value = {0x12, 0x34, 0x56},
		},
		.specs = {
			.FLASH_Size = 16 * 1024,
			.FLASH_PageSize = 32, // For debug only
		}
	},
	{0} // End of array
};

const avr_device_t* AVR_DEVICE_Search(char* id) {
	(void)id;
	return &AvailableParts[0];
}
