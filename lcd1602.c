#include "lcd1602.h"

static void lcd_delay(uint8 time) 
{
	while(time--);
}

static void lcd_write_cmd(uint8 cmd)
{
	lcd_delay(50);
	lcd_EN = 0;
	lcd_RW = 0;		// Write Mode
	lcd_RS = 0;		// commend reg
	
	lcd_data = cmd;
	lcd_delay(10);
	lcd_EN = 1;
	lcd_delay(50);
	lcd_EN = 0;
}

static void lcd_write_data(uint8 cmd)
{
	lcd_delay(50);
	lcd_EN = 0;
	lcd_RW = 0;		// Write Mode
	lcd_RS = 1;		// Data reg
	
	lcd_data = cmd;
	lcd_delay(10);
	lcd_EN = 1;
	lcd_delay(50);
	lcd_EN = 0;
}

void lcd_init()
{
	// LCD Function set: 8bit Data Mode, 2 display line, font type 5x8
	lcd_write_cmd(0x38);
	
	// Entry mode set: set cursor direction right, disable SH
	lcd_write_cmd(0x06);
}

void lcd_mode_set(uint8 display_on ,uint8 show_cursor , uint8 blink_cusor)
{
	// Display on/off controll
	uint8 cmd = 0x08;
	
	if(display_on == 1)
	{
		cmd |= 0x04;  //set Display controll Bit D on
	}
	
	if(show_cursor == 1)
	{
		cmd |= 0x02;  //set Corsor controll Bit C on
	}
	
	if(blink_cusor == 1)
	{
		cmd |= 0x01;  //set blink controll Bit B on
	}
	
	lcd_write_cmd(cmd);
}

void lcd_cursor_return()
{
    lcd_write_cmd(0x02);
}



void lcd_clear_screen()
{
	lcd_write_cmd(0x01);
}

void lcd_set_char(uint8 x, uint8 y, char character)
{
	uint8 cmd = 0x80;	// Set DDRAM address
	
	// Clear the out of bound value (16x2)
	x &= 0x0f;
	y &= 0x01;
	
	// 1st line address: 00H ~ 27H   2nd line: 40H ~ 67H
	if(y == 0)
	{
		cmd |= x;
		
	}else if(y == 1)
	{
		cmd |= 0x40;	//Address offset
		cmd |= x;
	}
	
	lcd_write_cmd(cmd);
	lcd_write_data(character);
}


void lcd_set_string(uint8 x, uint8 y, const char* string)
{
    // Clear the out of bound values (16x2)
    x &= 0x0F; 
    y &= 0x01; 
	
    for (; y < 2; y++)
    {
        while (x < 16 && *string != '\0')
        {
            lcd_set_char(x, y, *(string++));
            x++; 
        }

        // reset x for next line
        x = 0;
    }
}

void lcd_set_number(uint8 x, uint8 y, uint8 length, uint16 num)  // Display in decimal
{
	uint8 i;
    uint8 digits[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; // 8 digits max
    uint8 digit_count = 0;
    
    //special case num = 0
    if (num == 0) 
	{
        digits[length - 1] = '0';
    }else 
	{
        // Extract digits from right to left and place them directly at the right position
        while (num > 0) 
		{
            digits[length - 1 - digit_count] = (num % 10) + '0';
            digit_count++;
            num /= 10;
            if (digit_count >= length) break;  // Prevent overflow
        }
    }
    
    // Display all characters
    for (i = 0; i < length; i++) 
	{
        lcd_set_char(x + i, y, digits[i]);
    }
}


