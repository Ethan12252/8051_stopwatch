#ifndef __LCD1602_H__
#define __LCD1602_H__

#include <reg52.h>

#include "int.h"

sbit lcd_RS = P3^6;		// Reg select pin
sbit lcd_RW = P3^5;		// Read(1) / Write(0) select pin
sbit lcd_EN = P3^4;		// Enable pin
#define lcd_data P2		// Data Port


void lcd_init();
void lcd_mode_set(uint8 display_on ,uint8 show_cursor , uint8 blink_cusor);

//void lcd_cursor_shift(uint8 direction);  //Unfinish
void lcd_cursor_return();
void lcd_clear_screen();

void lcd_set_char(uint8 x, uint8 y, char character);
void lcd_set_string(uint8 x, uint8 y, const char* string);
void lcd_set_number(uint8 x, uint8 y, uint8 length, uint16 num);

#endif  //__LCD1602_H__