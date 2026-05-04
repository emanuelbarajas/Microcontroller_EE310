/*
;---------------------------------------------------------------
; Title: Assignment: Analog to Digital Converters
;---------------------------------------------------------------
; Program Details:
;   This program continuously reads values from an accelerometer using the PIC?s ADC.
;   It uses those readings to figure out the orientation and displays either
;   "FLAT", "TILTING RIGHT", "TILTING LEFT", or "SHAKING" on the LCD.
;   When the interrupt button is pressed, the program pauses for about 10 seconds
;   and flashes an LED to show that the interrupt was triggered.
;
; Inputs:
;   - RA1 : Accelerometer x-output
;   - RA2 : Accelerometer y-output
;   - RA3 : Accelerometer z-output
;   - RC3 : Interrupt Button
;
; Outputs:
;   - PORTB : LCD
;   - RD2   : Status LED
;
; Setup:
;   - PIC18F47K42 mounted on breadboard
;   - Accelerometer mounted on separate breadboard connected to ADC pins
;   - LCD to display accelerometer reading connected to PORTB
;   - One pushbutton connected to external interrupt
;   - One potentiometer to adjust LCD contrast
;   - One status LED to indicate interrupt has been triggered
;
;
; Date:
;   May 1, 2026
;
; File Dependencies / Libraries:
;   - xc.h
;   - stdio.h
;   - string.h
;   - stdlib.h
;
; Compiler:
;   XC8 v3.10
;
; Author:
;   Emanuel Barajas
;
; Versions:
;   V1.0 : Original version
;---------------------------------------------------------------
*/

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// PIC18F47K42 Configuration Bits
#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_1MHZ

#pragma config CLKOUTEN = OFF
#pragma config PR1WAY = ON
#pragma config CSWEN = ON
#pragma config FCMEN = OFF

#pragma config MCLRE = EXTMCLR
#pragma config PWRTS = PWRT_OFF
#pragma config MVECEN = OFF
#pragma config IVT1WAY = ON
#pragma config LPBOREN = OFF
#pragma config BOREN = SBORDIS

#pragma config BORV = VBOR_2P45
#pragma config ZCD = OFF
#pragma config PPS1WAY = ON
#pragma config STVREN = ON
#pragma config DEBUG = OFF
#pragma config XINST = OFF

#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF

#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC

#pragma config BBSIZE = BBSIZE_512
#pragma config BBEN = OFF
#pragma config SAFEN = OFF
#pragma config WRTAPP = OFF

#pragma config WRTB = OFF
#pragma config WRTC = OFF
#pragma config WRTD = OFF
#pragma config WRTSAF = OFF
#pragma config LVP = ON

#pragma config CP = OFF

#define _XTAL_FREQ 1000000UL

#define LCD_RS LATDbits.LATD0
#define LCD_EN LATDbits.LATD1
#define LCD_PORT LATB

#define REF_VOLT 5.0

int adc_value;
float adc_voltage;
char lcd_buffer[16];

void ADC_Init(void);
void LCD_Init(void);
void float_to_string(float val, char *buf);
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char dat);
void LCD_String(const char *msg);
void LCD_String_xy(unsigned char row, unsigned char pos, const char *msg);
void LCD_Clear(void);
void LCD_Print_Status(unsigned int x_val, unsigned int y_val, unsigned int z_val);
unsigned int ADC_Read(unsigned char channel);

void main(void)
{
    ANSELB = 0x00;
    ANSELD = 0x00;

    TRISB = 0x00;
    TRISD = 0x00;

    LATB = 0x00;
    LATD = 0x00;

    ANSELDbits.ANSELD2 = 0;
    TRISDbits.TRISD2 = 0;
    LATDbits.LATD2 = 0;

    ANSELCbits.ANSELC3 = 0;
    TRISCbits.TRISC3 = 1;
   
    PORTC;

    IOCCPbits.IOCCP3 = 1;
    IOCCNbits.IOCCN3 = 1;

    IOCCFbits.IOCCF3 = 0;
    PIR0bits.IOCIF = 0;

    PIE0bits.IOCIE = 1;

    INTCON0bits.IPEN = 0;
    INTCON0bits.GIE = 1;

    ADC_Init();
    __delay_ms(500);
    LCD_Init();

    unsigned int x_val, y_val, z_val;

    while(1)
    {
        x_val = ADC_Read(0x01);
        y_val = ADC_Read(0x02);
        z_val = ADC_Read(0x03);

        LCD_Print_Status(x_val, y_val, z_val);

        __delay_ms(150);
    }
}

void __interrupt() ISR(void)
{
    if(PIR0bits.IOCIF)
    {
        if(IOCCFbits.IOCCF3)
        {
            IOCCFbits.IOCCF3 = 0;

            for(int i = 0; i < 10; i++)
            {
                LATDbits.LATD2 = 1;
                __delay_ms(250);
                LATDbits.LATD2 = 0;
                __delay_ms(250);
            }
        }

        PIR0bits.IOCIF = 0;
    }
}

void LCD_Print_Status(unsigned int x_val, unsigned int y_val, unsigned int z_val)
{
    static unsigned int prev_x = 0;
    static unsigned int prev_y = 0;
    static unsigned int prev_z = 0;

    int delta_x = abs((int)x_val - (int)prev_x);
    int delta_y = abs((int)y_val - (int)prev_y);
    int delta_z = abs((int)z_val - (int)prev_z);

    prev_x = x_val;
    prev_y = y_val;
    prev_z = z_val;

    LCD_Clear();

    if(delta_x > 100 || delta_y > 100 || delta_z > 100)
    {
        LCD_String_xy(1, 0, "SHAKING");
    }
    else if(x_val > 2150)
    {
        LCD_String_xy(1, 0, "TILTING RIGHT");
    }
    else if(x_val < 1950)
    {
        LCD_String_xy(1, 0, "TILTING LEFT");
    }
    else
    {
        LCD_String_xy(1, 0, "FLAT");
    }
}

unsigned int ADC_Read(unsigned char channel)
{
    ADPCH = channel;
    __delay_ms(2);

    ADCON0bits.GO = 1;
    while(ADCON0bits.GO);

    return ((unsigned int)ADRESH << 8) | ADRESL;
}

void LCD_Init(void)
{
    __delay_ms(500);

    LCD_RS = 0;
    LCD_EN = 0;
    LCD_PORT = 0x00;

    __delay_ms(50);

    LCD_Command(0x30);
    __delay_ms(10);

    LCD_Command(0x30);
    __delay_ms(10);

    LCD_Command(0x30);
    __delay_ms(10);

    LCD_Command(0x38);
    __delay_ms(10);

    LCD_Command(0x08);
    __delay_ms(5);

    LCD_Command(0x01);
    __delay_ms(10);

    LCD_Command(0x06);
    __delay_ms(5);

    LCD_Command(0x0C);
    __delay_ms(5);
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    __delay_ms(5);
}

void LCD_Command(unsigned char cmd)
{
    LCD_PORT = cmd;

    LCD_RS = 0;
    __delay_ms(1);

    LCD_EN = 1;
    __delay_ms(1);
    LCD_EN = 0;

    __delay_ms(3);
}

void LCD_Char(unsigned char dat)
{
    LCD_PORT = dat;

    LCD_RS = 1;
    __delay_ms(1);

    LCD_EN = 1;
    __delay_ms(1);
    LCD_EN = 0;

    __delay_ms(2);
}

void LCD_String(const char *msg)
{
    while(*msg != '\0')
    {
        LCD_Char(*msg);
        msg++;
    }
}

void LCD_String_xy(unsigned char row, unsigned char pos, const char *msg)
{
    unsigned char location;

    if(row == 1)
    {
        location = 0x80 + pos;
    }
    else
    {
        location = 0xC0 + pos;
    }

    LCD_Command(location);
    LCD_String(msg);
}

void ADC_Init(void)
{
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;

    ANSELAbits.ANSELA1 = 1;
    ANSELAbits.ANSELA2 = 1;
    ANSELAbits.ANSELA3 = 1;

    ADCON0bits.FM = 1;
    ADCON0bits.CS = 1;

    ADCLK = 0x00;
    ADPREL = 0x00;
    ADPREH = 0x00;
    ADACQL = 0x00;
    ADACQH = 0x00;

    ADCON0bits.ON = 1;
}