#include "ch.h"
#include "hal.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "config.h"
#include "igate.h"
#include "digipeater.h"
#include "ui/ui.h"
#include "ui/gui.h"


typedef void (*menucmd_t)(void*);

typedef struct {
  const char     *mc_name;  
  menucmd_t       mc_func;
  void           *mc_arg;
} MenuCommand;


/* Handler functions (defined below) */
static void* mhandle_send(void*);
static void* mhandle_igate(void*); 
static void* mhandle_digipeater(void*); 
static void* mhandle_wifi(void*); 



MenuCommand items[] = 
{
    { "Send report", mhandle_send, NULL },
    { "WIFI +|-", mhandle_wifi, NULL },
    { "Igate +|-", mhandle_igate, NULL },
    { "Digipeater +|-", mhandle_digipeater, NULL },
    { "Blow up all", NULL, NULL },
};
int nitems = 5; 



/*********************************************************
 * Show a menu with one item highlighted
 * items is an array of strings of max length 4. 
 *********************************************************/

#define MIN(x,y) (x<y ? x : y)
#define MAX_VISIBLE 4

static int start = 0, selection = 0;
static bool menu_active;

 
static void menu_show(int start, int sel) 
{ 
   gui_clear();
   gui_hLine(1,0,82);
   gui_hLine(1,44,83);
   gui_hLine(3,45,82);
   gui_vLine(0,0,45);
   gui_vLine(82,0,45);
   gui_vLine(83,3,41);

   gui_box(0, sel*11, 83, 12, true);
   int i;
   for (i=0; i < MIN(nitems,MAX_VISIBLE); i++) 
     gui_writeText(4, 2+i*11, items[start+i].mc_name); 
   gui_flush();
}



bool menu_is_active()
   { return menu_active; }


void menu_activate()
{
    selection = 0;
    start = 0;
    menu_active = true;
    menu_show(start, selection); 
}


void menu_increment()
{
    if (start+selection >= nitems-1) {
        menu_end();
        return; // How do we end the menu?
    }
    if (selection >= MAX_VISIBLE-1)
       start++;
    else
       selection++;
    menu_show(start, selection);
}



void menu_replaceText(char* txt)
{
    strcpy(items[start+selection].mc_name, txt);
}



void menu_select()
{
    menu_end();
    if (items[start+selection].mc_func != NULL)
        items[start+selection].mc_func(items[start+selection].mc_arg);
}


void menu_end()
{
    menu_active = false; 
    gui_clear();
    status_show();
}



static void click_handler(void* p) {
    lcd_backlight();
    if (menu_active)
        menu_increment();
    else
        status_next();
}


static void push_handler(void* p) {
   lcd_backlight();
   if (menu_active)
       menu_select();
   else
       menu_activate();
}



/****************************************************
 * Thread that periodically update display and turn
 * off/on igate or digipeater.
 ****************************************************/
 THREAD_STACK(gui_thread, STACK_GUI);
 
  __attribute__((noreturn))
 static THD_FUNCTION(gui_thread, arg) {
     while (true) {
         sleep(5000);
         
         if (!menu_is_active())
             status_show();
         
         igate_on(GET_BYTE_PARAM(IGATE_ON));
         digipeater_on(GET_BYTE_PARAM(DIGIPEATER_ON));
     }
 }
 
void menu_init()
{
    register_button_handlers(click_handler, push_handler);
    THREAD_START(gui_thread, NORMALPRIO, NULL);
}


/**********************************************************
 * Menu commands
 **********************************************************/

static void* mhandle_send(void* x) {
    tracker_posReport(); 
}

static void* mhandle_igate(void* x) {
    bool isOn = GET_BYTE_PARAM(IGATE_ON); 
    igate_on( !isOn ); 
}

static void* mhandle_digipeater(void* x) {
    bool isOn = GET_BYTE_PARAM(DIGIPEATER_ON); 
    digipeater_on( !isOn ); 
}

static void* mhandle_wifi(void* x) {
    bool isOn = GET_BYTE_PARAM(WIFI_ON); 
    wifi_on( !isOn ); 
}


