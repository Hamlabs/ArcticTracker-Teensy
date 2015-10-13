#include "fbuf.h"

#define HDLC_FLAG 0x7E
#define MAX_HDLC_FRAME_SIZE 289 // including FCS field
 

void hdlc_test_on(uint8_t b);
void hdlc_test_off(void);
FBQ* hdlc_init_encoder(output_queue_t *oq);
fbq_t* hdlc_get_encoder_queue(void);
bool hdlc_enc_packets_waiting(void);
uint8_t rand_u8(void); 