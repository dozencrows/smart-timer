/* LCD module header */

#if !defined(__LCD_H__)
#define __LCD_H__

extern void lcdInit();
extern void lcdClear();
extern void lcdSetBacklight(int value);
extern bool lcdIsBacklightOn();
extern void lcdMoveTo(int x, int y);
extern void lcdPuts(const char* s);

#endif

