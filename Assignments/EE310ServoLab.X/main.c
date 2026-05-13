/*
;---------------------------------------------------------------
; Title: Assignment_Servo_Oscillation
;---------------------------------------------------------------
; Program Details:
;   This program controls a servo using two buttons. When button1 is pressed, the servo
;   moves to the left until the button is released. When button2 is pressed, the servo moves
;   to the right until the button is released. When no buttons are pressed, the servo
;   holds its position. The servo stops when it reaches it limit in either direction.
;
; Inputs:
;   - RD0 : Push Button to move servo to the left
;   - RD1 : Push Button to move servo to the right
;
; Outputs:
;   - RB3 : MG90S Micro Servo
;
; Setup:
;   - PIC18F47K42 mounted on breadboard
;   - Two pushbuttons used as servo directional input
;   - One servo connected as an output
;
; Date:
;   May 12, 2026
;
; File Dependencies / Libraries:
;   - xc.h
;   - config.h
;   - header.h
;
; Compiler:
;   XC8 v3.10
;
; Author:
;   Emanuel Barajas
;
; Versions:
;   V1.1
;---------------------------------------------------------------
*/




#include <xc.h> // must have this
#include "header.h" // must have this
#include "config.h" // must have this -  XC8_ConfigFile.h
//#include "C:\Program Files\Microchip\xc8\v2.40\pic\include\proc\pic18f46k42"


#define _XTAL_FREQ 4000000      // Fosc  frequency for _delay()  library
#define FCY    _XTAL_FREQ/4     // system clock is 1 usec
#define BUTTONleft PORTDbits.RD0
#define BUTTONright PORTDbits.RD1

#define myLED  PORTBbits.RB0
unsigned long PWM2_INITIALIZE_DUTY_VALUE = 47;
int duty_value = 47;

uint16_t checkdutyCycle;
char preScale;
_Bool pwmStatus;

void main (void) {
    OSCSTATbits.HFOR =1; // enable  HFINTOSC Oscillator (see clock schematic))
    OSCFRQ=0x02; // 00=1 MHZ, 02=4MHZ internal - see page 106 of data sheet
    
    ANSELB = 0b00000000;    
    TRISB= 0b00000000;//sets PORTB as all outputs 
    PORTB= 0b00000000;//turns off PORTB outputs so that the LED is initially off
    TMR2_Initialize();
    TMR2_StartTimer();
    ANSELD = 0x00;
    TRISD = 0b00000011;
    
    PWM_Output_D8_Enable();
    PWM2_Initialize();
    PWM2_LoadDutyValue(PWM2_INITIALIZE_DUTY_VALUE ); // initialize CCPR2H/L
   // PWM_Output_D8_Disable();
   // TMR2_StopTimer();  

    // Duty Cycle in percentage 
    checkdutyCycle =(uint16_t)((100UL*PWM2_INITIALIZE_DUTY_VALUE)/(4*(T2PR+1)));
    // binary value of Register T2CON.PRESCALE
    preScale = ((T2CON >> 4) & (0x0F)); 
    
    while (1) {
        pwmStatus = PWM2_OutputStatusGet();
        PORTBbits.RB2 = pwmStatus;
        if (BUTTONright){
            if (duty_value > 10){
                duty_value--;
            }
            PWM2_LoadDutyValue(duty_value);
            __delay_ms(70);
        }
        if (BUTTONleft){
            if (duty_value < 78){
                duty_value++;
            }
            PWM2_LoadDutyValue(duty_value);
            __delay_ms(70);
        }
    }
}