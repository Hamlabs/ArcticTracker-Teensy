
 
#define AFSK_RX_QUEUE_SIZE 128 
#define AFSK_TX_QUEUE_SIZE 128

/* Timers */

#define AFSK_RX_GPT      GPTD1
#define AFSK_TX_GPT      GPTD2
#define AFSK_TONEGEN_GPT GPTD3


/* Serial ports */
#define SHELL_SERIAL SDU1
#define WIFI_SERIAL  SD1
#define GPS_SERIAL   SD2
#define RADIO_SERIAL SD3



#define THREAD_STACK(n, st)  static THD_WORKING_AREA(wa_##n, st)
#define THREAD_START(n, prio, arg) chThdCreateStatic(wa_##n, sizeof(wa_##n), (prio), n, arg)
#define THREAD_DSTART(n, size, prio, arg) \
   chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE((size)), (prio), n, arg)


#define sleep(n)  chThdSleepMilliseconds(n)
#define t_yield   chThdYield

#define Stream BaseSequentialStream

#define putch(s, ch) streamPut(s, ch)
#define getch(s) streamGet(s)