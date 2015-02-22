//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * MRT Timer Interrupt Handling
 */
 
#if !defined(__MRT_INTERRUPT_H__)
#define __MRT_INTERRUPT_H__

extern void mrt_interrupt_control(bool enable);
extern void mrt_interrupt_set_timer_callback(int channel, void (*callback)());

#endif // #if !defined(__MRT_INTERRUPT_H__)
