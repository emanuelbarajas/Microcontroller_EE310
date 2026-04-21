/*
;---------------------------------------------------------------
; Title: Assignment_7_Interfacing_With_Sensors
;---------------------------------------------------------------
; Program Details:
;   This program interfaces two photoresistors, a pushbutton,
;   an emergency interrupt switch, a 7-segment display, a buzzer,
;   and a motor/relay with the PIC18F47K42 microcontroller.
;
;   The program reads input counts from PR1 and PR2. PR1 is used
;   to enter the first digit of the code, and PR2 is used to enter
;   the second digit of the code. A pushbutton is used to store
;   each digit after it has been entered.
;
;   After the first button press, the PR1 count is stored and the
;   system switches to PR2 input mode. After the second button
;   press, the PR2 count is stored and the full 2-digit code is
;   compared to the preset secret code.
;
;   If the entered code matches the secret code, the motor/relay
;   is activated. If the code is incorrect, the buzzer is turned on
;   for a short period of time.
;
;   The system also includes an emergency interrupt switch. When
;   the emergency switch is triggered, the normal program flow is
;   interrupted and the buzzer plays a distinct emergency melody
;   inside the ISR.
;
;   The 7-segment display is connected to PORTD and is used to
;   display the currently entered digit so the user can verify
;   the sensor input count.
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
;   - PIC18F47K42 mounted on the Curiosity board
;   - Two photoresistor circuits configured as digital inputs
;   - One pushbutton for digit storage
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

const unsigned char secret1 = 3;   // first digit from PR1
const unsigned char secret2 = 2;   // second digit from PR2

const unsigned char segCode[10] = {
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

void showDigit(unsigned char digit)
{
    if (digit <= 9)
        LATD = segCode[digit];
    else
        LATD = 0x00;
}

unsigned char readPR1Digit(void)
{
    unsigned char count = 0;
    unsigned char prev;
    unsigned char idleLoops = 0;

    prev = PORTBbits.RB1;

    while (1)
    {
        if (emergencyFlag == 1)
            return 0;

        // count a swipe when RB1 goes from HIGH to LOW
        if ((PORTBbits.RB1 == 0) && (prev == 1))
        {
            count++;
            if (count > 9)
                count = 9;

            showDigit(count);
            idleLoops = 0;
            __delay_ms(120);
        }

        prev = PORTBbits.RB1;

        // no activity for about 1.5 sec means first digit finished
        __delay_ms(50);
        idleLoops++;

        if ((count > 0) && (idleLoops >= 30))
            break;
    }

    return count;
}

unsigned char readPR2Digit(void)
{
    unsigned char count = 0;
    unsigned char prev;
    unsigned char idleLoops = 0;

    prev = PORTBbits.RB2;

    while (1)
    {
        if (emergencyFlag == 1)
            return 0;

        // count a swipe when RB2 goes from HIGH to LOW
        if ((PORTBbits.RB2 == 0) && (prev == 1))
        {
            count++;
            if (count > 9)
                count = 9;

            showDigit(count);
            idleLoops = 0;
            __delay_ms(120);
        }

        prev = PORTBbits.RB2;

        // no activity for about 1.5 sec means second digit finished
        __delay_ms(50);
        idleLoops++;

        if ((count > 0) && (idleLoops >= 30))
            break;
    }

    return count;
}

void goodCode(void)
{
    LATCbits.LATC3 = 1;
    __delay_ms(5000);
    LATCbits.LATC3 = 0;
}

void badCode(void)
{
    unsigned char i;
    for (i = 0; i < 6; i++)
    {
        LATCbits.LATC2 = 1;
        __delay_ms(200);
        LATCbits.LATC2 = 0;
        __delay_ms(200);
    }
}

void melodyISR(void)
{
    unsigned char i;

    LATCbits.LATC3 = 0;

    for (i = 0; i < 8; i++)
    {
        LATCbits.LATC2 = 1;
        __delay_ms(100);
        LATCbits.LATC2 = 0;
        __delay_ms(100);
    }

    for (i = 0; i < 4; i++)
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
    unsigned char firstDigit;
    unsigned char secondDigit;

    initPIC();

    while (1)
    {
        emergencyFlag = 0;
        LATCbits.LATC0 = 1;   // system LED on
        showDigit(0);

        // read first digit from PR1
        firstDigit = readPR1Digit();

        if (emergencyFlag == 1)
            continue;

        __delay_ms(500);

        // read second digit from PR2
        secondDigit = readPR2Digit();

        if (emergencyFlag == 1)
            continue;

        __delay_ms(500);

        if ((firstDigit == secret1) && (secondDigit == secret2))
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