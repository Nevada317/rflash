#ifndef _RFP_COMMON_H
#define _RFP_COMMON_H

#include <stdint.h>
#include <assert.h>

#define OFFSET_FLASH 0x00000000UL
#define OFFSET_EEP   0x01000000UL
#define OFFSET_FUSE  0x02000000UL


typedef enum __attribute__((__packed__)) {
	// ### TASK_STATUS field enum ###
	// This enum will be stored as uint8_t.
	// Upper nibble signifies major states:
	// 0 - Empty
	// 1 - Task creation
	// 2 - Low-level operation in progress
	// 4 - Write in progress
	// 5 - Read in progress
	// 8 - Result available

	// Buffer ready or filling
	RFP_Task_None      = 0x00U, // Buffer is empty and can be filled
	RFP_Task_Claimed   = 0x11U, // Buffer is partially filled
	RFP_Task_Filled    = 0x12U, // Buffer was filled. Waiting for execution

	// Low-Level operations (stub)
	RFP_Task_LL_SPI    = 0x21U, // Low-Level operation in progress - SPI
	RFP_Task_LL_I2C    = 0x22U, // Low-Level operation in progress - I2C
	RFP_Task_LL_BB     = 0x23U, // Low-Level operation in progress - BitBang

	// FLASH writing
	RFP_Task_PrepWrite = 0x41U, // Preparing chip for write
	RFP_Task_Filling   = 0x42U, // Filling page buffer
	RFP_Task_Writing   = 0x43U, // Performing page write
	RFP_Task_TimeWait  = 0x44U, // Waiting for page write execution
	RFP_Task_AckWait   = 0x45U, // Polling chip until write finished
	RFP_Task_Verifiyng = 0x46U, // Reading back all page

	// FLASH reading
	RFP_Task_PrepRead  = 0x51U, // Preparing chip for read
	RFP_Task_Reading   = 0x52U, // Reading requested page

	// Final states
	// Good results - 80-87
	RFP_Task_Finished  = 0x80U, // Write completed
	RFP_Task_DataReady = 0x81U, // Read completed
	RFP_Task_SPI_Res   = 0x82U, // Low-Level result - SPI
	RFP_Task_I2C_Res   = 0x83U, // Low-Level result - I2C
	RFP_Task_BB_Res    = 0x84U, // Low-Level result - BitBang
	// Bad results - 88-8F
	RFP_Task_BadTask   = 0x88U, // Task verification failed
	RFP_Task_ChipNACK  = 0x89U, // Chip does not respond
	RFP_Task_SigErr    = 0x8AU, // Signature verification error
	RFP_Task_Mismatch  = 0x8BU, // Data verification failed
	RFP_Task_Unsupp    = 0x8EU, // Unsupported value given
	RFP_Task_CRC_Err   = 0x8FU, // CRC mismatch
} rfp_task_status_t;

typedef enum __attribute__((__packed__)) {
	// ### COMMAND field enum ###
	// This enum will be stored as uint8_t.
	// Request (PC to programmer)
	RFP_Cmd_Poll       = 0x00U,
	RFP_Cmd_AddData    = 0x01U,
	RFP_Cmd_Ack        = 0x7FU,

	// Response (Programmer to PC)
	RFP_Cmd_Status     = 0x80U,
	RFP_Cmd_DataReport = 0x81U,
} rfp_command_t;

typedef enum __attribute__((__packed__)) {
	// ### OPERATION field enum ###
	// This enum will be stored as uint8_t.
	// This enum should be handled as bitfield

	RFP_OPER_SigCheck = 0x01U, // Verify chip signature (AVR only)
	RFP_OPER_Erase    = 0x02U, // Perform chip erase (AVR only?)
	RFP_OPER_RMW      = 0x04U, // Read-Modify-Write, assuming FF is "unchanged"
	RFP_OPER_Write    = 0x08U, // Write given page
	RFP_OPER_Verify   = 0x10U, // Verify written page
	RFP_OPER_Read     = 0x20U, // Read whole page
	// Bit 6 is reserved
	RFP_OPER_LL       = 0x80U, // Perform low-level operation
} rfp_operation_t;

typedef enum __attribute__((__packed__)) {
	// ### PROTOCOL field enum ###
	// This enum will be stored as uint8_t.
	RFP_PROTOCOL_AVR     = 0x01U,
	RFP_PROTOCOL_SPI     = 0x02U,
	RFP_PROTOCOL_I2C     = 0x03U,
	RFP_PROTOCOL_BitBang = 0x04U,
} rfp_protocol_t;

// Safety assertions for protocol uniformity
static_assert(sizeof(rfp_task_status_t) == 1, "Short enums expected");
static_assert(sizeof(rfp_command_t)     == 1, "Short enums expected");
static_assert(sizeof(rfp_operation_t)   == 1, "Short enums expected");
static_assert(sizeof(rfp_protocol_t)    == 1, "Short enums expected");

typedef struct  __attribute__((packed)) rfp_buffer_t {
	uint8_t Payload[256];
	uint8_t PayloadSize; // 0 here means 256 due to overflow
	uint32_t Address;
	rfp_operation_t Operation;
	rfp_protocol_t Protocol;
	uint8_t Signature[4];

	uint16_t CRC;
} rfp_buffer_t;


#endif /* _RFP_COMMON_H */
