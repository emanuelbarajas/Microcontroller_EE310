/*
;---------------------------------------------------------------
; Title: Assignment_7_Interfacing_With_Sensors
;---------------------------------------------------------------
; Program Details:
;   This program interfaces two photoresistors, an emergency
;   interrupt switch, a 7-segment display, a buzzer, and a
;   motor/relay with the PIC18F47K42 microcontroller.
;
;   The program measures transitions from PR1 and PR2. PR1 is used
;   to input the first digit of the code, while PR2 is used to input
;   the second digit. Each digit is determined by counting the
;   number of signal changes detected on the respective sensor.
;
;   When activity on PR1 stops for a short duration, the first digit
;   is saved and the system moves to PR2 input mode. When activity
;   on PR2 stops, the second digit is saved and the full 2-digit
;   code is evaluated against the predefined target code.
;
;   If the entered code matches the target code, the motor/relay
;   is enabled. If the code does not match, the buzzer is activated
;   briefly to indicate an error.
;
;   The system also includes an emergency interrupt switch. When
;   triggered, the normal operation is halted and the buzzer plays
;   a distinct emergency tone within the ISR.
;
;   The 7-segment display is connected to PORTD and shows the
;   current count so the user can verify the input from the sensors.
;
; Inputs:
;   - RB1 : Photoresistor 1 (PR1), first digit input
;   - RB2 : Photoresistor 2 (PR2), second digit input
;   - RB0 : Emergency switch interrupt input
;
; Outputs:
;   - PORTD : 7-segment display output
;   - RC0   : System status LED
;   - RC3   : Motor / relay control output
;   - RC2   : Buzzer output
;
; Setup:
;   - PIC18F47K42
;   - Two photoresistor circuits configured as digital inputs
;   - One emergency switch connected to external interrupt
;   - One 7-segment display connected to PORTD
;   - One relay or motor driver circuit
;   - One buzzer output circuit
;
; Date:
;   April 9, 2026
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

const unsigned char targetDigit1 = 3;   // first digit from PR1
const unsigned char targetDigit2 = 2;   // second digit from PR2

const unsigned char sevenSegMap[10] = {
    0x3F,   // 0
    0x06,   // 1
    0x5B,   // 2
    0x4F,   // 3
    0x66,   // 4
    0x6D,   // 5
    0x7D,   // 6
    0x07,   // 7
    0x7F,   // 8
    0x6F    // 9
};

void __interrupt(irq(IRQ_INT0), base(8)) INT0_ISR(void)
{
    emergencyFlag = 1;
    melodyISR();
    PIR1bits.INT0IF = 0;
}

void initPIC(void)
{
    OSCCON1 = 0x60;
    OSCFRQ = 0x02;   // 4 MHz

    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;

    // inputs
    TRISBbits.TRISB0 = 1;   // emergency
    TRISBbits.TRISB1 = 1;   // PR1
    TRISBbits.TRISB2 = 1;   // PR2

    // outputs
    TRISCbits.TRISC0 = 0;   // SYS LED
    TRISCbits.TRISC3 = 0;   // motor / relay
    TRISCbits.TRISC2 = 0;   // buzzer
    TRISD = 0x00;           // 7-segment

    LATCbits.LATC0 = 0;
    LATCbits.LATC3 = 0;
    LATCbits.LATC2 = 0;
    LATD = 0x00;

    // weak pull-ups on PORTB
    WPUBbits.WPUB0 = 1;
    WPUBbits.WPUB1 = 1;
    WPUBbits.WPUB2 = 1;

    // emergency interrupt on RB0 / INT0
    INTCON0bits.INT0EDG = 1;
    PIR1bits.INT0IF = 0;
    PIE1bits.INT0IE = 1;
    INTCON0bits.GIEL = 1;
    INTCON0bits.GIEH = 1;
}

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

        // count a swipe when RB1 goes from HIGH to LOW
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

        // no activity for about 1.5 sec means first digit finished
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

        // count a swipe when RB2 goes from HIGH to LOW
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

        // no activity for about 1.5 sec means second digit finished
        __delay_ms(50);
        idleCounter++;

        if ((swipeCount > 0) && (idleCounter >= 30))
            break;
    }

    return swipeCount;
}

void goodCode(void)
{
    LATCbits.LATC3 = 1;
    __delay_ms(5000);
    LATCbits.LATC3 = 0;
}

void badCode(void)
{
    unsigned char index;
    for (index = 0; index < 6; index++)
    {
        LATCbits.LATC2 = 1;
        __delay_ms(200);
        LATCbits.LATC2 = 0;
        __delay_ms(200);
    }
}

void melodyISR(void)
{
    unsigned char index;

    LATCbits.LATC3 = 0;

    for (index = 0; index < 8; index++)
    {
        LATCbits.LATC2 = 1;
        __delay_ms(100);
        LATCbits.LATC2 = 0;
        __delay_ms(100);
    }

    for (index = 0; index < 4; index++)
    {
        LATCbits.LATC2 = 1;
        __delay_ms(250);
        LATCbits.LATC2 = 0;
        __delay_ms(250);
    }

    showDigit(0);
}

void main(void)
{
    unsigned char enteredDigit1;
    unsigned char enteredDigit2;

    initPIC();

    while (1)
    {
        emergencyFlag = 0;
        LATCbits.LATC0 = 1;   // system LED on
        showDigit(0);

        // read first digit from PR1
        enteredDigit1 = readPR1Digit();

        if (emergencyFlag == 1)
            continue;

        __delay_ms(500);

        // read second digit from PR2
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