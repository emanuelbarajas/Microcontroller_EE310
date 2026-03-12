//-----------------------------
// Title: EE 310 Project 1
//-----------------------------
// Purpose: This program is designed for temperature regulation
//	    system that measures the current temperature of a room
//	    so it can heat or cool it to get closer to the set
//	    desired temperature.
// Dependencies: MyConfigFile.inc
// Compiler: MPLAB X IDE v6.25
// Author: Emanuel Barajas 
// OUTPUTS: The signal to turn on the heater is connected to an LED PORTD0.
//	    The signal to turn on the cooler is connected to an LED PORTD1.
// INPUTS: measuredTempInput and refTempInput
// Versions:
//  	V1.0: 3/6/2026 - First version 
//-----------------------------

;----------------
; PROGRAM INPUTS
;----------------
;The DEFINE directive is used to create macros or symbolic names for values.
;It is more flexible and can be used to define complex expressions or sequences of instructions.
;It is processed by the preprocessor before the assembly begins.

#include "MyConfigFile.inc"
#include <xc.inc>

#define  measuredTempInput 	5 ; this is the input value
#define  refTempInput 		15 ; this is the input value

;---------------------
; Definitions
;---------------------
#define SWITCH    LATD,2  
#define LED0      PORTD,0
#define LED1	  PORTD,1
    
 
;---------------------
; Program Constants
;---------------------
refTemp		EQU	0x20	;Register with reference temperature
measuredTemp	EQU	0x21	;Register with measured temperature
contReg		EQU	0x22	;Register that stores final output for PORTD

refCopy EQU 0x41    ;register of copy of reference temperature
refVal0 EQU 0x60    ;register of first digit of reference temperature
refVal1 EQU 0x61    ;register of second digit of reference temperature
refVal2 EQU 0x62    ;register of third digit of reference temperature

measuredCopy EQU 0x42	;register of copy of measured temperature
measuredVal0 EQU 0x70	;register of first digit of measured temperature
measuredVal1 EQU 0x71	;register of second digit of measured temperature
measuredVal2 EQU 0x72	;register of third digit of measured temperature

; The EQU (Equals) directive is used to assign a constant value to a symbolic name or label.
; It is simpler and is typically used for straightforward assignments.
;It directly substitutes the defined value into the code during the assembly process.
    
REG10   equ     10h   // in HEX
REG11   equ     11h
REG01   equ     1h

   
   PSECT absdata,abs,ovrld
   
   ORG 0x20			;Start PC at 0x20
   
   MOVLW 0x0C			;Define inputs/outputs for PORTD
   MOVWF TRISD, 0
   
   CLRF REG10, 0		;Clear registers 10 and 11 for incrementing
   CLRF REG11, 0
   
   MOVLW measuredTempInput	;Move the measured temp into REG21
   MOVWF measuredTemp, 0	
   MOVWF measuredCopy, 0	;Make a copy of measuredTemp for manipulation
   
   MOVLW refTempInput		;Move the reference temp into REG20
   MOVWF refTemp, 0
   MOVWF refCopy, 0		;Make a copy of refTemp for manipulation
   
   BTFSC measuredTemp, 7, 0	;Check highest bit of measured in case of negative
   GOTO NEGHEAT			;Go to a label that deals with the negative
   
   CPFSGT measuredTemp, 0	;Check if the the measured is greater than the ref
   GOTO HEAT			;If not go to a heating label for further analysis
   GOTO COOL			;If so then go to a cooling label to output cool LED
   
   ORG 0x100
   NEGHEAT:			
    NEGF measuredTemp, 0	;Take the 2s compliment of the measured temperature
    NEGF measuredCopy, 0	;Take the 2s compliment of the copy as well
    MOVLW 0x01			;Light up the heating LED
    MOVWF contReg, 0
    MOVFF contReg, PORTD
    GOTO CONVERTREF		;Continue to the hex to decimal conversion
   
   ORG 0x150
   HEAT:
    CPFSLT measuredTemp, 0	;Check if measured temp is less than reference
    GOTO OFF			;If not go to a label turns both LEDs off
    
    MOVLW 0x01			;If less than light up the heating LED
    MOVWF contReg, 0
    MOVFF contReg, PORTD
    GOTO CONVERTREF
   
   ORG 0x200
   COOL:
    MOVLW 0x02			;Light up the cooling LED
    MOVWF contReg, 0
    MOVFF contReg, PORTD
    GOTO CONVERTREF
   
   ORG 0x250
   OFF:
    MOVLW 0x00			;Keep both LEDs off for no heat or cool
    MOVWF contReg, 0
    MOVFF contReg, PORTD
    GOTO CONVERTREF
    
    ORG 0x300
   CONVERTREF:
    MOVLW 0x0A			;Converts reference temperature from HEX to DEC,
    CPFSEQ refCopy, 0		;by building the tens place digit by subtracting
    CPFSLT refCopy, 0		;10 from the copy of reference temperature and
    GOTO ADJUSTREF		;incrementing register 0x61 as many times as it
    GOTO SENDREF		;can subtract before going negative
    
   ADJUSTREF:
    INCF REG10, 1, 0
    SUBWF refCopy, 1, 0
    GOTO CONVERTREF
   
   SENDREF:			;Sends the decimal digits of the reference
    MOVFF REG10, refVal1	;temperature to registers 60-62
    MOVFF refCopy, refVal0
    
   CONVERTMEAS:
    CPFSEQ measuredCopy, 0	;Converts reference temperature from HEX to DEC
    CPFSLT measuredCopy, 0	;by building the tens place digit by subtracting
    GOTO ADJUSTMEAS		;10 from the copy of measured temperature and
    GOTO SENDMEAS		;incrementing register 0x71 as many times as it
				;can subtract before going negative
   ADJUSTMEAS:
    INCF REG11, 1, 0
    SUBWF measuredCopy, 1, 0
    GOTO CONVERTMEAS
    
   SENDMEAS:			;Sends the decimal digits of the reference
    MOVFF REG11, measuredVal1	;temperature to registers 60-62
    MOVFF measuredCopy, measuredVal0
    
   TERMINATE:
    END				;Ends the program
   