/* Timers module header */

#if !defined(__TIMERS_H__)
#define __TIMERS_H__

extern void timersInit();

// Delay via stall - will block interrupts
extern void delayUs(int microseconds);

// Delay via loop - will allow interrupts
extern void delayMs(int milliseconds);

#endif
