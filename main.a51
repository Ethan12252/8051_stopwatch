P4 		    EQU  0E8H
; timer values for 25ms @ 24MHz/12
TIMER0_H    EQU  03CH     ; Higher byte of (65536 - 50000) 
TIMER0_L    EQU  0B0H    ; Lower byte of (65536 - 50000)

; buttons
run_btn		EQU  P2.0
rst_btn     EQU  P2.1
lap_btn     EQU  P2.2
	
; ---------------------Variables---------------------------
timer_count  EQU  30H	 
disp_time    EQU  31H    ; ~34H : 4 bytes(hundreds, tens, ones, tenths) 
actual_time  EQU  35H    ; ~38H : 4 bytes

running      EQU  39H	 ; running flag
lap_mode     EQU  3AH	 ; lap mode flag

; ----------------------Program Start-------------------------
    ORG  0000H
    JMP  START

;---------------------Timer0 ISR---------------------------
    ORG  000BH
    PUSH  ACC
    
    ; Reload timer 
    MOV  TH0, #TIMER0_H
    MOV  TL0, #TIMER0_L
    
    ; Check if 0.1s passed (4 counts of 25ms)
    INC  timer_count
    MOV  A, timer_count
    CJNE  A, #4, isr_exit
    
    ; 0.1s passed, reset counter
    MOV  timer_count, #0
    
    ; update only if running
    MOV  A, running
    JZ  isr_exit
    
    ; Update the digits (if reaches 9, carry foward)
    MOV  A, actual_time+3	; tenths
    INC  A	; Increment tenths
    MOV  actual_time+3, A
    CJNE  A, #10, check_display
    MOV  actual_time+3, #0
    
    INC  actual_time+2    ; ones
    MOV  A, actual_time+2
    CJNE  A, #10, check_display
    MOV  actual_time+2, #0
    
    INC  actual_time+1    ; tens
    MOV  A, actual_time+1
    CJNE  A, #10, check_display
    MOV  actual_time+1, #0
    
    INC  actual_time      ; hundreds
    MOV  A, actual_time
    CJNE  A, #10, check_display
    
    ; Stop at 999.9
    MOV  actual_time, #9
    MOV  actual_time+1, #9
    MOV  actual_time+2, #9
    MOV  actual_time+3, #9
    CLR  TR0	; stop timer
    MOV  running, #0
    
	check_display:
    ; Update the actual_time to disp_time if not in lap mode
    MOV  A, lap_mode
    JNZ  isr_exit
    ; Update
    MOV  A, actual_time
    MOV  disp_time, A
    MOV  A, actual_time+1
    MOV  disp_time+1, A
    MOV  A, actual_time+2
    MOV  disp_time+2, A
    MOV  A, actual_time+3
    MOV  disp_time+3, A
    
	isr_exit:
    POP     ACC
    RETI

; -----------------------Debounce Delay---------------------------
; Debounce delay for buttons
DELAY_DEBOUNCE:
	MOV R6, #50
	INNER_LOOP:
	MOV R7, #0FFH
	DEBOUNCE_LOOP:
    DJNZ R7, DEBOUNCE_LOOP
	DJNZ r6, INNER_LOOP
    RET

; ---------------------------Main------------------------------
START:
    ; Init Timer0
    MOV  TMOD, #01H    ; Timer0 Mode1
    MOV  TH0, #TIMER0_H
    MOV  TL0, #TIMER0_L
    SETB  EA		; Enable interrupts
    SETB  ET0        ; Enable Timer0
    
    ; Init all the variables
    MOV  timer_count, #0
    MOV  running, #0
    MOV  lap_mode, #0
    MOV  disp_time, #0
    MOV  disp_time+1, #0
    MOV  disp_time+2, #0
    MOV  disp_time+3, #0
    MOV  actual_time, #0
    MOV  actual_time+1, #0
    MOV  actual_time+2, #0
    MOV  actual_time+3, #0

MAIN_LOOP:
    
CHECK_RUNSTOP:
    ; Check run/stop button (P2.0)
    JB  run_btn, CHECK_RESET
    ACall DELAY_DEBOUNCE
    
    ; Toggle running state
    MOV  A, running
    CPL  A
    MOV  running, A
    JNZ  START_TIMER
    CLR  TR0		; Stop timer
    JMP  WAIT_RUN
	START_TIMER:
    SETB  TR0       ; Start timer
    
	WAIT_RUN:
    JNB  run_btn, WAIT_RUN  ; Wait for the button release

CHECK_RESET:
    ; Check reset button (P2.1), activate only if not running
    JB  rst_btn, CHECK_LAP
    MOV  A, running
    JNZ  CHECK_LAP
    ACall  DELAY_DEBOUNCE
    
    ; Clear all times
    MOV  actual_time, #0
    MOV  actual_time+1, #0
    MOV  actual_time+2, #0
    MOV  actual_time+3, #0
    MOV  disp_time, #0
    MOV  disp_time+1, #0
    MOV  disp_time+2, #0
    MOV  disp_time+3, #0
    
    WAIT_RESET:
    JNB  rst_btn, WAIT_RESET    ; Wait for button release

CHECK_LAP:
    ; Check lap (P2.2), activate only if running
    JB  lap_btn, DISPLAY
    MOV  A, running
    JZ  DISPLAY
    ACall  DELAY_DEBOUNCE
    
    ; Toggle lap mode
    MOV  A, lap_mode
    CPL  A
    MOV  lap_mode, A
    JNZ  WAIT_LAP
   
    ; If leaving lap mode, update display
    MOV  A, actual_time
    MOV  disp_time, A
    MOV  A, actual_time+1
    MOV  disp_time+1, A
    MOV  A, actual_time+2
    MOV  disp_time+2, A
    MOV  A, actual_time+3
    MOV  disp_time+3, A
    
    WAIT_LAP:
    JNB  lap_btn, WAIT_LAP

DISPLAY:
    ; Display current digit
    MOV  R1, #0      ; Digit count
    MOV  R0, #disp_time
    
	DISP_LOOP:
    ; Display
	MOV  A, @R0      ; Get digit
    MOV  DPTR, #LUT_CODES
    MOVC  A, @A+DPTR
	CJNE R1, #2, NO_DP 	; Add decimal point for D3
	ANL  A, #7FH    ; Turn on decimal point
	NO_DP:
    MOV  P0, A       ; Output to display
    
	; Drive
    MOV  A, R1
    MOV  DPTR, #LUT_DRIVE
    MOVC  A, @A+DPTR
    MOV  P4, A       ; Select digit
    
    ; Small delay for display
    MOV  R7, #0FFH
	DISP_DELAY:
    DJNZ  R7, DISP_DELAY
    
    INC  R0
    INC  R1
    CJNE  R1, #4, DISP_LOOP
    
    JMP  MAIN_LOOP

;----------------------------Tables---------------------------------
; LUT for Seven segment codes (0~9)
LUT_CODES:  
	DB  0C0H, 0F9H, 0A4H, 0B0H, 099H, 092H, 082H, 0F8H, 080H, 090H

; LUT for Display drive codes (D1~D3)
LUT_DRIVE: 
	DB  0FEH, 0FDH, 0FBH, 0F7H

END