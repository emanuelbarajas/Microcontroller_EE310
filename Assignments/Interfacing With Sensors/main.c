/*
;---------------------------------------------------------------
; Title: Assignment_7_Interfacing_With_Sensors
;---------------------------------------------------------------
; Program Details:
;   This program interfaces two photoresistors, a pushbutton,
;   an emergency interrupt switch, a 7-segment display, a buzzer,
;   and a motor/relay with the PIC18F47K42 microcontroller.
;
;   The program reads swipe counts from PR1 and PR2. PR1 is used
;   to enter the first digit (enteredDigit1), and PR2 is used to
;   enter the second digit (enteredDigit2). A pushbutton is used
;   to confirm each digit after it has been entered.
;
;   After the first confirmation, the PR1 swipe count is stored
;   and the system switches to PR2 input mode. After the second
;   confirmation, the PR2 swipe count is stored and the complete
;   2-digit code is compared to the preset target digits
;   (targetDigit1 and targetDigit2).
;
;   If the entered digits match the target digits, the motor/relay
;   is activated. If the code is incorrect, the buzzer produces
;   an error beep pattern.
;
;   The system also includes an emergency interrupt switch. When
;   triggered, the normal program flow is interrupted and the
;   buzzer plays a distinct emergency melody inside the ISR while
;   setting the emergencyFlag.
;
;   The 7-segment display is connected to PORTD and displays the
;   current swipe count using the sevenSegMap lookup table so the
;   user can verify the entered digit.
;
; Inputs:
;   - RB1 : Photoresistor 1 (PR1), first digit input
;   - RB2 : Photoresistor 2 (PR2), second digit input
;   - RB0 : Emergency switch interrupt input
;
; Outputs:
;   - PORTD : 7-segment display output (digit display)
;   - RC0   : System status LED (system active indicator)
;   - RC3   : Motor / relay control output (activated on correct code)
;   - RC2   : Buzzer output (error beeps and emergency melody)
;
; Setup:
;   - PIC18F47K42 mounted on the Curiosity board
;   - Two photoresistor circuits configured as digital inputs
;   - One pushbutton for digit confirmation
;   - One emergency switch connected to external interrupt
;   - One 7-segment display connected to PORTD
;   - One relay or motor driver circuit
;   - One buzzer output circuit
;
; Date:
;   April 8, 2028
;
; File Dependencies / Libraries:
;   - xc.h
;   - XC8_ConfigFile.h
;   - Project header/source files as required
;
; Compiler:
;   XC8 v2.40
;
; Author:
;   Emanuel Barajas
;
; Versions:
;   V1.0 : Original version
;---------------------------------------------------------------
*/

#include "config.h"
#include "init.h"
#include "functions.h"

volatile unsigned char emergencyFlag = 0;

const unsigned char targetDigit1 = 3;
const unsigned char targetDigit2 = 2;

const unsigned char sevenSegMap[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

void showDigit(unsigned char value)
{
    if (value <= 9)
        LATD = sevenSegMap[value];
    else
        LATD = 0x00;
}

unsigned char readPR1Digit(void)
{
    unsigned char swipeCount = 0;
    unsigned char previousState;
    unsigned char idleCounter = 0;

    previousState = PORTBbits.RB1;

    while (1)
    {
        if (emergencyFlag == 1)
            return 0;

        if ((PORTBbits.RB1 == 0) && (previousState == 1))
        {
            swipeCount++;
            if (swipeCount > 9)
                swipeCount = 9;

            showDigit(swipeCount);
            idleCounter = 0;
            __delay_ms(120);
        }

        previousState = PORTBbits.RB1;

        __delay_ms(50);
        idleCounter++;

        if ((swipeCount > 0) && (idleCounter >= 30))
            break;
    }

    return swipeCount;
}

unsigned char readPR2Digit(void)
{
    unsigned char swipeCount = 0;
    unsigned char previousState;
    unsigned char idleCounter = 0;

    previousState = PORTBbits.RB2;

    while (1)
    {
        if (emergencyFlag == 1)
            return 0;

        if ((PORTBbits.RB2 == 0) && (previousState == 1))
        {
            swipeCount++;
            if (swipeCount > 9)
                swipeCount = 9;

            showDigit(swipeCount);
            idleCounter = 0;
            __delay_ms(120);
        }

        previousState = PORTBbits.RB2;

        __delay_ms(50);
        idleCounter++;

        if ((swipeCount > 0) && (idleCounter >= 30))
            break;
    }

    return swipeCount;
}

void main(void)
{
    unsigned char enteredDigit1;
    unsigned char enteredDigit2;

    initPIC();

    while (1)
    {
        emergencyFlag = 0;
        LATCbits.LATC0 = 1;
        showDigit(0);

        enteredDigit1 = readPR1Digit();

        if (emergencyFlag == 1)
            continue;

        __delay_ms(500);

        enteredDigit2 = readPR2Digit();

        if (emergencyFlag == 1)
            continue;

        __delay_ms(500);

        if ((enteredDigit1 == targetDigit1) && (enteredDigit2 == targetDigit2))
        {
            goodCode();
        }
        else
        {
            badCode();
        }

        __delay_ms(1000);
        showDigit(0);
    }
}