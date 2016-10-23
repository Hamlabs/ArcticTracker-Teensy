 
#if !defined __DEF_TRACKER_H__
#define __DEF_TRACKER_H__


void tracker_setGate(FBQ* gt);
void tracker_on(void);
void tracker_off(void);
void tracker_posReport(void);
void tracker_init(void);
void tracker_addObject(void);
void tracker_clearObjects(void);


#endif