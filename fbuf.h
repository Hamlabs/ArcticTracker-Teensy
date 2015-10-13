#if !defined __FBUF_H__
#define __FBUF_H__

#include "ch.h"
#include <inttypes.h>

#define NILPTR        255


#define fbuf_t FBUF
#define fbq_t FBQ

typedef uint16_t fbindex_t;

/*********************************
   Packet buffer chain
 *********************************/
typedef struct _fb
{
   fbindex_t head, wslot, rslot; 
   uint8_t   rpos; 
   uint8_t   length;
}
FBUF; 


/****************************************
   Operations for packet buffer chain
 ****************************************/

void  fbuf_new      (FBUF* b);
FBUF  fbuf_newRef   (FBUF* b);
void  fbuf_release  (FBUF* b);
void  fbuf_reset    (FBUF* b);
void  fbuf_rseek    (FBUF* b, const uint8_t pos);
void  fbuf_putChar  (FBUF* b, const char c);
void  fbuf_write    (FBUF* b, const char* data, const uint8_t size);
void  fbuf_putstr   (FBUF* b, const char *data);
char  fbuf_getChar  (FBUF* b);
char* fbuf_read     (FBUF* b, uint8_t size, char *buf);
void  fbuf_insert   (FBUF* b, FBUF* x, uint8_t pos);
void  fbuf_connect  (FBUF* b, FBUF* x, uint8_t pos);

fbindex_t fbuf_freeSlots(void);
uint16_t fbuf_freeMem(void);

#define fbuf_eof(b) ((b)->rslot == NILPTR)
#define fbuf_length(b) ((b)->length)
#define fbuf_empty(b) ((b)->length == 0)



/*********************************
   Queue of packet buffer chains
 *********************************/

typedef mailbox_t FBQ;

#define FBQ_DECL(name, buffer, size) FBQ name = _MAILBOX_DATA(name, buffer, size)


/************************************************
   Operations for queue of packet buffer chains
 ************************************************/

FBUF* fbq_get(FBQ* fbq);

#define fbq_length(q) chMBGetUsedCountI(q)
#define fbq_eof(q)    (chMBGetUsedCountI(q) <= 0)
#define fbq_clear(q)  chMBReset(q)
#define fbq_put(q, b) chMBPost(q, (msg_t) b, TIME_INFINITE)

#define FBQ_INIT(name,size)  static msg_t name##_fbqbuf[(size)*sizeof(msg_t*)];    \
    chMBObjectInit(&(name), (name##_fbqbuf), (size))

#endif /* __FBUF_H__ */
