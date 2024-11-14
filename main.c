#include <reg52.h>
#include "int.h"

#define TIMER0_VAL 50000    // Timer value for 25ms interval with 24MHz/12 timer clock

// Ports
#define DISPLAY P0
#define DISPLAY_DRIVE P4
// Buttons
sbit run_stop = P2^0;      // Start/Stop button
sbit reset    = P2^1;      // reset button
sbit lap      = P2^2;      // lap button

typedef struct { 
	uint8 hundreds, tens, ones, tenths; 
} Time_Digits;

Time_Digits display_time;   // Time to shown on display
Time_Digits actual_time;    // Actual counting time
uint8* display_time_arr = (uint8*)&display_time; // Array form of display_time

// Status flags
bit running = 0;
bit lap_mode = 0; 

// Look-up table for the display 
const uint8 seven_segment_codes[] = {
    0xC0,  // 0
    0xF9,  // 1
    0xA4,  // 2
    0xB0,  // 3
    0x99,  // 4
    0x92,  // 5
    0x82,  // 6
    0xF8,  // 7
    0x80,  // 8
    0x90   // 9
};

// Drive table for the diaplay
const uint8 seven_segment_drive[] = {
	0xFE,	// D1 (The leftmost one on the dev board)
	0xFD,	// D2
	0xFB,	// D3
	0xF7	// D4
};

// Prototypes
void init_timer();
void update_digits(Time_Digits *time);
void display_update();
void reset_time(Time_Digits *time);
void delay();

/* Timer0 ISR - Called every 25ms when timer is running */
void timer0_isr() interrupt 1 {
	static uint8 timer_count = 0;
	// Reload Timer
    TH0 = (65536 - TIMER0_VAL) / 256U;
    TL0 = (65536 - TIMER0_VAL) % 256U;
    
    if (timer_count == 4) { // 25ms * 4 = 0.1s
        if (running) {
            actual_time.tenths++;  // +0.1s
            update_digits(&actual_time);    
            if (!lap_mode) { display_time = actual_time; } 
        }
        timer_count = 0;
    }
    timer_count++;
}

/* Update digits when counting since we store it by digits */
void update_digits(Time_Digits *time) {
	// Handle The Overflow
    if (time->tenths > 9)   { time->tenths = 0; time->ones++;     }  
    if (time->ones > 9)     { time->ones = 0;   time->tens++;     } 
    if (time->tens > 9)     { time->tens = 0;   time->hundreds++; }
    if (time->hundreds > 9) {
    	//If reaches 999.9s, Stop the timer
        time->hundreds = time->tens = time->ones = time->tenths = 9;
        running = 0;  
        TR0 = 0;
    }
}

void reset_time(Time_Digits *time) { 
	time->hundreds = time->tens = time->ones = time->tenths = 0;
}

void init_timer() {
    TMOD |= 0x01; // Timer0 Mode1(16bit)
    TH0 = (65536 - TIMER0_VAL) / 256U;	// Set the timer value
    TL0 = (65536 - TIMER0_VAL) % 256U;
    TF0 = 0;    // Clear timer flag
    EA = 1;     // Enable All Interrupt
    ET0 = 1;    //Enable Interrupt for timer0
}

void delay(uint8 num) {
    uint8 i;
    for(i = 0; i < num; i++);
}

/* Display Drive Code */
void display_update() {
	uint8 i;
	for(i = 0; i < 4; i++) {
		DISPLAY = seven_segment_codes[ display_time_arr[i] ];
		DISPLAY_DRIVE = seven_segment_drive[i];
		delay(500);
	}
}

int main() {
    init_timer();
    while(1) {
        if (!run_stop) { // Active low button
            delay(1500);        // Wait for some times for the button input to be stable
            running = !running; // Toggle the running flag
            TR0 = running;      // Start timer
            while(!run_stop);   // Wait for the button to be released
        }
        
        if (!reset && !running) {
            delay(1500);
            reset_time(&actual_time);  			
            reset_time(&display_time);
            while(!reset);
        }
        
        if (!lap && running) {
            delay(1500);
            lap_mode = !lap_mode;
            if (!lap_mode) { display_time = actual_time; }
            while(!lap);
        }
        display_update();  // Update the display_time to the display
    }
}
