//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/* Timers module header */

#if !defined(__TIMERS_H__)
#define __TIMERS_H__

extern void timersInit();

// Delay via stall - will block interrupts
extern void delayUs(int microseconds);

// Delay via loop - will allow interrupts
extern void delayMs(int milliseconds);

#endif
