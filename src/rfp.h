#ifndef _RFP_COMMON_H
#define _RFP_COMMON_H

#include <stdint.h>
#include <assert.h>

#define OFFSET_FLASH 0x00000000UL
#define OFFSET_EEP   0x01000000UL
#define OFFSET_FUSE  0x02000000UL

/* ###  PROTOCOL DEFINITION  ###
 * Server is a programming hardware (executor)
 * Client is programming software running in PC.
 *
 * Each side has to send similar packets using transport-level protocol
 *
 * Transport packets carry up to 128 bytes of payload.
 * Byte stuffing is necessary.
 * Special bytes has to be stuffed.
 * STX and ESC symbols should be replaced by
 * ESC ESTX and ESC EESC byte pairs.
 *
 * NOTE: Bytes, that need to be stuffed appear only in Payload and CRC, as
 * other fields may not become equal to those special values.
 *
 * Transport parcel looks like:
 * | STX | CMD | IDX | LEN |  PAYLOAD  | CRC |
 *       |                 \_LEN bytes_/     |
 *       \__________Byte stuffing____________/
 * Where:
 *   STX - Fixed constant
 *   CMD - Command (see rfp_command_t)
 *     (FROM PC to executor)
 *       Reset
 *       Poll
 *       Add0
 *       Add1
 *       Add2
 *       Get0
 *       Get1
 *       Ack (finishes task)
 *     (FROM executor to PC)
 *       Report
 *       Data0
 *       Data1
 *   IDX - Block number
 *       Should be 255 for reset/poll/report command,
 *       and in 0-127 range in others cases
 *   LEN - length of payload (0-128)
 *   PAYLOAD - [LEN] bytes
 *   CRC - CRC-16 code for CMD..PAYLOAD
 *     (TODO: Describe poly and initial state)
 *
 * Add0 command allows one to fill Buffer[0..127]
 * Add1 command allows one to fill Buffer[128..255]
 * Add2 command allows one to fill task parameters (Address, oper etc)
 *
 * Executor should check, that ALL of those commands are executed IN ORDER.
 * If payload's second part is not needed (due to low page size), one shall call
 * empty Add1 operation with payload size 0, so Add2 command becomes is available.
 * Second call to the same function will be ignored by executor.
 *
 * Best case scenario:
 * [FF] Reset >
 * [01] Add0 >
 * [01] Add1 >
 * [01] Add2 >
 * [FF] Poll >
 * < Report (task 01 is in state GotAdd1)
 * [01] Add2 >
 * [FF] Poll >
 * < Report (task 01 is in state Filled)
 * [02] Add0 >
 * [02] Add1 >
 * [02] Add2 >
 * < Report (01 is in execution, 02 is filled)
 * */

#define RFP_STX 0xFC
#define RFP_ESC 0xFE
#define RFP_STUFFING_MASK 0x80
#define RFP_ESTX (RFP_STX ^ RFP_STUFFING_MASK)
#define RFP_EESC (RFP_ESC ^ RFP_STUFFING_MASK)



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
	RFP_Task_GotAdd0   = 0x11U, // Buffer is partially filled
	RFP_Task_GotAdd1   = 0x12U, // Buffer is partially filled
	RFP_Task_Filled    = 0x13U, // Buffer was filled. Waiting for execution

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

	RFP_Task_Process   = 0x7EU, // Processing (internal use)
	RFP_Task_Unknown   = 0x7FU, // State is unknown (internal use)
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
	RFP_CMD_POLL        = 0x00,
	RFP_CMD_Add0        = 0x11,
	RFP_CMD_Add1        = 0x12,
	RFP_CMD_Add2        = 0x13,
	RFP_CMD_Get0        = 0x21,
	RFP_CMD_Get1        = 0x22,
	RFP_CMD_Ack         = 0x55,
	RFP_CMD_RESET       = 0xFF,

	// Response (Programmer to PC)
	RFP_CMD_REPORT      = 0xF0,
	RFP_CMD_Data0       = 0x81,
	RFP_CMD_Data1       = 0x82,
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
	RFP_OPER_Release  = 0x40U, // Release target/bus
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

/*
typedef enum __attribute__((__packed__)) {
	// ### Flags_Common field enum ###
	// This enum will be stored as uint8_t.
	RFP_FLAGS_COMMON_Unused0    = 0x01U,
	RFP_FLAGS_COMMON_Unused1    = 0x02U,
	RFP_FLAGS_COMMON_Unused2    = 0x04U,
	RFP_FLAGS_COMMON_Unused3    = 0x08U,
	RFP_FLAGS_COMMON_Unused4    = 0x10U,
	RFP_FLAGS_COMMON_Unused5    = 0x20U,
	RFP_FLAGS_COMMON_Unused6    = 0x40U,
	RFP_FLAGS_COMMON_Unused7    = 0x80U,
} rfp_flags_com_t;
*/

#define Flag_AVR_ExtendedAddress 0x01

// Safety assertions for protocol uniformity
static_assert(sizeof(rfp_task_status_t) == 1, "Short enums expected");
static_assert(sizeof(rfp_command_t)     == 1, "Short enums expected");
static_assert(sizeof(rfp_operation_t)   == 1, "Short enums expected");
static_assert(sizeof(rfp_protocol_t)    == 1, "Short enums expected");
// static_assert(sizeof(rfp_flags_com_t)   == 1, "Short enums expected");

typedef struct  __attribute__((packed)) rfp_buffer_t {
	uint8_t Payload[256];
	uint8_t PayloadSize; // 0 here means 256 due to overflow
	uint32_t Address;
	rfp_operation_t Operation;
	rfp_protocol_t Protocol;
	// rfp_flags_com_t Flags_Common;
	uint8_t Flags;
	uint16_t CRC;
} rfp_buffer_t;

typedef struct rfp_list_t rfp_list_t;
struct rfp_list_t {
	rfp_list_t* next;
	rfp_list_t* root;
	uint32_t index;
	uint32_t count; // Valid only for root record

	rfp_buffer_t Buffer;
};

typedef struct __attribute__((packed)) rfp_flexbuffer_t {
	uint16_t Length;
	uint8_t Data[];
} rfp_flexbuffer_t;

rfp_list_t* RFP_LIST_NewRecord(rfp_list_t** root_ptr);
void RFP_AppendCRC(rfp_buffer_t* Buffer);

// Returns block, allocated by malloc. Should be free'd
rfp_flexbuffer_t * RFP_CreateParcel(rfp_command_t command, uint8_t index, rfp_buffer_t* buffer);

#endif /* _RFP_COMMON_H */
