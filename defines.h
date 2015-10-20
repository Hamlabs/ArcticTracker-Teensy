
 
#define AFSK_RX_QUEUE_SIZE 128 
#define AFSK_TX_QUEUE_SIZE 128

/* Hardware timers */
#define AFSK_RX_GPT      GPTD1
#define AFSK_TX_GPT      GPTD2
#define AFSK_TONEGEN_GPT GPTD3


/* Serial ports */
#define SHELL_SERIAL   SDU1
#define WIFI_SERIAL    SD1
#define GPS_SERIAL     SD2
#define TRX_SERIAL     SD3

/* 0 = 12.5 KHz, 1 = 25 KHz */
#define TRX_BANDWIDTH  0
#define TRX_PTT_PORT   IOPORT4
#define TRX_PTT_PIN    PORTD_TEENSY_PIN6
#define TRX_PD_PORT    IOPORT4  
#define TRX_PD_PIN     PORTD_TEENSY_PIN5
#define TRX_SQ_PORT
#define TRX_SQ_PIN

#define LED_R_PIN      PORTD_TEENSY_PIN2
#define LED_G_PIN      PORTA_TEENSY_PIN3
#define LED_B_PIN      PORTA_TEENSY_PIN4
#define LED_R_PORT     IOPORT4
#define LED_G_PORT     IOPORT1
#define LED_B_PORT     IOPORT1


#define THREAD_STACK(n, st)  static THD_WORKING_AREA(wa_##n, st)
#define THREAD_START(n, prio, arg) chThdCreateStatic(wa_##n, sizeof(wa_##n), (prio), n, arg)
#define THREAD_DSTART(n, size, prio, arg) \
   chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE((size)), (prio), n, arg)


#define sleep(n)  chThdSleepMilliseconds(n)
#define t_yield   chThdYield

#define Stream BaseSequentialStream

#define putch(s, ch) streamPut(s, ch)
#define getch(s) streamGet(s)


