;---------------------
; Title: Seven Segment
;---------------------
; Program Details:
; The purpose of this program is to increment the value displayed on a seven
; segment display by pressing a button and decrement that value by pressing
; another button.
    
; Inputs: RB0, RB1
; Outputs: RD0-7
; Setup: The Curiosity Board

; Date: Mar 24, 2026
; File Dependencies / Libraries: It is required to include the
;   AssemblyConfig.inc in the Header Folder
; Compiler: xc8, 2.4
; Operating System: macOS

; Build / Compile:
;   Build using MPLAB X "Build Project"
;   Program via Curiosity Nano drag-and-drop (.hex file)

; Author: Emanuel Barajas
; Versions:
;       V1.0: Original

;----------------
; PROGRAM INPUTS
;----------------
;The DEFINE directive is used to create macros or symbolic names for values.
;It is more flexible and can be used to define complex expressions or sequences of instructions.
;It is processed by the preprocessor before the assembly begins.

#include "AssemblyConfig.inc"
#include <xc.inc>
    
innerLoop EQU 1
outerLoop EQU 1
thirdLoop EQU 1


;---------------------
; Definitions
;---------------------

#define BUTTONA	    PORTB, 0
#define BUTTONB	    PORTB, 1
#define DISPLAY     LATD

;---------------------
; Program Constants
;---------------------

REG10 EQU 0x0A ;Starting address for the LFSR
REG25 EQU 0x19
REG31 EQU 0x31
REG32 EQU 0x32
REG33 EQU 0x33

    
    PSECT absdata,abs,ovrld
    
    ORG 0x30    ;Start PC at 0x30
  

    BANKSEL PORTB   ; Initialize PortB for our buttons
    CLRF PORTB
    BANKSEL LATB
    CLRF LATB
    BANKSEL ANSELB
    CLRF ANSELB
    BANKSEL TRISB
    MOVLW 0x03
    MOVWF TRISB
    
    BANKSEL PORTD   ; Initialize PortD for our 7 segement display
    CLRF PORTD
    BANKSEL LATD
    CLRF LATD
    BANKSEL ANSELD
    CLRF ANSELD
    BANKSEL TRISD
    MOVLW 0x00
    MOVWF TRISD
    
    LFSR 0, REG10

	       ; Set registers 0x0A - 0x19 to 7 segment hex values
    MOVLW 0x3F ; 0
    MOVWF REG10
    MOVLW 0x06 ; 1
    MOVWF 0x0B
    MOVLW 0x5B ; 2
    MOVWF 0x0C
    MOVLW 0x4F ; 3
    MOVWF 0x0D
    MOVLW 0x66 ; 4
    MOVWF 0x0E
    MOVLW 0x6D ; 5
    MOVWF 0x0F
    MOVLW 0x7D ; 6
    MOVWF 0x10
    MOVLW 0x07 ; 7
    MOVWF 0x11
    MOVLW 0x7F ; 8
    MOVWF 0x12
    MOVLW 0x6F ; 9
    MOVWF 0x13
    MOVLW 0x77 ; A
    MOVWF 0x14
    MOVLW 0x7C ; b
    MOVWF 0x15
    MOVLW 0x39 ; C
    MOVWF 0x16
    MOVLW 0x5E ; d
    MOVWF 0x17
    MOVLW 0x79 ; E
    MOVWF 0x18
    MOVLW 0x71 ; F
    MOVWF REG25
    
    MOVF INDF0, W	; Move 0 to our display
    MOVWF DISPLAY
    
checkSame:		; Check if both buttons are pressed
    BTFSS BUTTONA
    GOTO checkButton
    
    BTFSS BUTTONB
    GOTO checkButton
    
    GOTO zero
    
    
checkButton:
    BTFSC BUTTONA	; Check if button A is clear
    GOTO increase	; If it is not clear begin incrementing
    
    BTFSC BUTTONB	; Check if button B is clear
    GOTO decrease	; If it is not clear begin decrementing
    
    GOTO checkSame	; If both are low loop back
    
increase:		; Begins incrementation process
    MOVF INDF0, W	; Check if our counted value is F
    CPFSEQ REG25	
    GOTO increment	; Increment if it is not F
    GOTO zero		; start reset process
    
decrease:		; Begins decrementation process
    MOVF INDF0, W	; Check if our counted value is 0
    CPFSEQ REG10
    GOTO decrement	; Increment if it is not 0
    GOTO max		; Start the maxing out process
   
increment:		; Increment the counted value and display it
    INCF FSR0L, F
    MOVF INDF0, W
    MOVWF DISPLAY
    CALL loopDelay	; Start the delay process
    GOTO checkSame	; Go back to checking the inputs

decrement:		; Decrement the counted value and display it
    DECF FSR0L, F
    MOVF INDF0, W
    MOVWF DISPLAY
    CALL loopDelay	; Start the delay process
    GOTO checkSame	; Go back to checking the inputs
    
max:			; Set our counted value to F
    LFSR 0, REG25	
    MOVF INDF0, W
    MOVWF DISPLAY
    CALL loopDelay	; Start the delay process
    GOTO checkSame	; Go back to checking the inputs
 
zero:			; Set out counted value to 0
    LFSR 0, REG10
    MOVF INDF0, W
    MOVWF DISPLAY
    CALL loopDelay	; Start the delay process
    GOTO checkSame	; Go back to checking the inputs

loopDelay:		; Initialize our delay
    MOVLW innerLoop	; Set correct values for a 600 ms delay
    MOVWF REG31
    MOVLW outerLoop
    MOVWF REG32
    MOVLW thirdLoop
    MOVWF REG33
    
loop1:			; Loop and decrement values for about 600ms
    DECF REG31, F
    BNZ loop1
    
    MOVLW innerLoop
    MOVWF REG31
    
    DECF REG32, F
    BNZ loop1
    
    MOVLW outerLoop
    MOVWF REG32
    
    DECF REG33, F
    BNZ loop1
    RETURN		; Go back to where this was called