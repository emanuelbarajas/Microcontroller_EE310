/*
 * The purpose of this program is demonstrate INT0 as an external interrupt. 
 * In this case when the signal changes state on RB0 (INT0) then D0 starts 
 * blinking for 4 seconds and then stops. The results can be simulated and
 * verified.  
 * 
 * Author: Farid Farahmand 
 */

// PIC18F46K42 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1L
#pragma config FEXTOSC = LP     
#pragma config RSTOSC = EXTOSC  

// CONFIG1H
#pragma config CLKOUTEN = OFF   
#pragma config PR1WAY = ON      
#pragma config CSWEN = ON       
#pragma config FCMEN = ON       

// CONFIG2L
#pragma config MCLRE = EXTMCLR  
#pragma config PWRTS = PWRT_OFF 
#pragma config MVECEN = ON      
#pragma config IVT1WAY = ON     
#pragma config LPBOREN = OFF    
#pragma config BOREN = SBORDIS  

// CONFIG2H
#pragma config BORV = VBOR_2P45 
#pragma config ZCD = OFF        
#pragma config PPS1WAY = ON     
#pragma config STVREN = ON      
#pragma config DEBUG = OFF      
#pragma config XINST = OFF      

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF       

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC      

// CONFIG4L
#pragma config BBSIZE = BBSIZE_512
#pragma config BBEN = OFF       
#pragma config SAFEN = OFF      
#pragma config WRTAPP = OFF     

// CONFIG4H
#pragma config WRTB = OFF       
#pragma config WRTC = OFF       
#pragma config WRTD = OFF       
#pragma config WRTSAF = OFF     
#pragma config LVP = ON         

// CONFIG5L
#pragma config CP = OFF         

#include <xc.h>

#define _XTAL_FREQ 4000000                 
#define FCY    _XTAL_FREQ/4

// Defining Interrupt ISR 
void __interrupt(irq(IRQ_INT0),base(0x4008)) INT0_ISR(void)
{
    if (PIR1bits.INT0IF){   // Check if interrupt flag for INT0 is set to 1 - (note INT0 is your input)
            // if so, do something
                // e.g,blink an LED connected to  PORTDbits.RD0 for 10 times with a delay of __delay_ms(250)

        for (int i = 0; i < 10; i++){
            LATDbits.LATD0 = 1;
            //__delay_ms(300);
            LATDbits.LATD0 = 0;
            //__delay_ms(300);
        }

        PIR1bits.INT0IF = 0;   // always clear the interrupt flag for INT0 when done
        LATDbits.LATD0 = 0;    // turn off the led on PORTDbits.RD0 
    }    
}

void INTERRUPT_Initialize (void)
{
    // Enable interrupt priority bit in INTCON0 (check INTCON0 register and find the bit)
    INTCON0bits.IPEN = 1;

    // Enable high priority interrupts using bits in INTCON0
    INTCON0bits.GIEH = 1;

    // Enable low priority interrupts using bits in INTCON0
    INTCON0bits.GIEL = 1;

    // Interrupt on rising edge of INT0 pin using bits in INTCON0
    INTCON0bits.INT0EDG = 1;

    // Set the interrup high priority (IP) for INT0 - INT0IP
    IPR1bits.INT0IP = 1;

    // Enable the interrup (IE) for INT0
    PIE1bits.INT0IE = 1;

    //Clear interrupt flag for INT0
    PIR1bits.INT0IF = 0;
  
    // Change IVTBASE by doign the following
    // Set IVTBASEU to 0x00 
    IVTBASEU = 0x00;

    // Set IVTBASEH to  0x40; 
    IVTBASEH = 0x40;

    // Set IVTBASEL to 0x08; 
    IVTBASEL = 0x08;
}

void main(void) {

    // Initialization  
    // set port B and D as outputs 
    TRISB = 0x01;   // RB0 input, others output
    TRISD = 0x00;   // all PORTD output

    ANSELB = 0x00;  // digital
    ANSELD = 0x00;

    // enable the weak pull-ups are enabled for port B
    WPUBbits.WPUB0 = 1;

    // initialize the interrupt_initialization by calling the proper function
    INTERRUPT_Initialize();

    // main code here 
        // blink an LED connected to RD1 every 2 seconds
    while (1){
        LATDbits.LATD1 = 1;
        //__delay_ms(2000);
        LATDbits.LATD1 = 0;
        //__delay_ms(2000);
    }
}