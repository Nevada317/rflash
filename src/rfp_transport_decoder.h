#ifndef _RFP_TRANSPORT_DECODER_H
#define _RFP_TRANSPORT_DECODER_H

void RFP_Transport_Decode_Init();
// Calls user CB with payload of form rfp_transport_rx_t
void RFP_Transport_Decode_SetCallback(void (*cb)(void* data, int length));

void RFP_Transport_Decode_Byte(uint8_t data);
void RFP_Transport_Decode_Block(void* data, int length);

#endif /* _RFP_TRANSPORT_DECODER_H */
