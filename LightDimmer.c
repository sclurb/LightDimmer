/*
 * File:   LightDimmer.c
 * Author: Bob
 *
 * Created on June 16, 2016, 8:37 AM
 * Revised on November 1, 2017
 * (Added debouncing feature and put two functions to implement debounce
 * void Bright ()
 * void Dim()
 */

// PIC16F688 Configuration Bit Settings

// 'C' source line config statements



// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

// #defines
#define _XTAL_FREQ 4000000                  // Fosc  frequency for _delay()  library
#define LedGreen PORTCbits.RC1
#define LedRed PORTCbits.RC2
#define Zero PORTAbits.RA2
#define UP PORTAbits.RA4
#define DOWN PORTAbits.RA5
#define OUT PORTCbits.RC3
#define BLAM PORTCbits.RC5
//definitions
unsigned char regdata;
unsigned char data;
unsigned char kip;
unsigned char count;
unsigned char xcount;
unsigned char xdiff;
bit upflag;
bit dwnflag;
unsigned char memory;
unsigned char Tcount;
bit memflag;

// function prototypes
unsigned char Getdata(unsigned char regdata);
void interrupt blink_r(void);
void run (void);

// functions

void Bright (void)
{
    while (UP == 0)
    {
        if (upflag == 0)        //upflag stays high while button pushed
        {
            xdiff = xcount;
            upflag = 1;
        }
        __delay_ms(50);
        xcount++;
        if (UP && (xcount - xdiff) < 5)
        {
            xcount = memory;    // if I try and add a nested if statement in this
        }                       // statement, I get problems.   Stack overflow maybe?
        if (xcount >= 51)
        {
            xcount = 51;
            LedGreen = 0;       // Light Green when at full brightness
            LedRed = 1;            
        }        
        else
        {
            LedRed = 1;         // else turn both red and gree off
            LedGreen = 1;
        }
        if (UP)
        {
            upflag = 0;
        }
        memflag = 1;
        run();   
    }
}
void Dim (void)
{
    while (DOWN == 0)
    {
        if (dwnflag == 0)
        {
            xdiff = xcount;
            dwnflag = 1;
        }
        __delay_ms(75);
        if (DOWN && (xdiff - xcount) < 7)
        {
            xcount = 0;
            memflag = 0;
        }
        else
        {
            xcount--;
        }

        if (xcount <= 1)
        {
            xcount = 1;
            LedRed = 0;             // light red when at full dim
            LedGreen = 1;
        }
        else
        {
            LedGreen = 1;           // otherwise turn both red and green off
            LedRed = 1;
        }
        if (DOWN)
        {
            dwnflag = 0;
        }
        run();  
    }
}
unsigned char Getdata(unsigned char regdata)
{
    ADCON0 = regdata;
    while (ADCON0bits.GO);
    return ADRESH;
}

void run (void)
{
    data = (unsigned char) xcount  * 0x05; 
}

//  high priority interrupt routine
void interrupt blink_r(void)
{
    if (INTF == 1)                  // interrupts with either Zero crossing or
    {                               // TMR0, 
        TMR0 = data;                // with data from the A/D
        count = 1;                  // 
        INTF = 0;                   // clears interrupt flag 
    }
    if (T0IF == 1 && count == 1)
    {
        count = 2;                  // sets the count to '2' so that when TMR0
                                    // trips the interrupt flag, it will ignore this if statement
        if (data <= 40)              // (for anlg operation only)  if data is less than 40, just turn the SCR gate pulse off       
        {                           // this has been changed to 40 to test whether LED bulbs are tripping on low A/C levels
            OUT = 1;                // turns off the SCR gate pulse
        }
        else
        {
            OUT = 0;                // turns the SCR gate pulse on
        }
        T0IF = 0;                   // resets the TMR0 interrupt flag
        TMR0 = 0xfa;                // sets the width of the SCR gate pulse
    }
    if ( T0IF == 1 && count == 2)   // if T0 interrupts with Count = 2,
    {                               // then turn off the SCR gate pulse
        OUT = 1;                    // turns off the SCR gate pulse
        count = 0;                  // resets the count
        T0IF = 0;                   // resets the TMR0 interrupt flag
    }
}  // end high priority interrupt

void main(void)
{
    OSCCON = 0x67;
    ANSEL = 0b00010000;
    WPUA = 0x00;
    CMCON0 = 0xff;
    TRISA = 0b00111111;
    TRISC = 0b00100001;
    OUT = 1;
    __delay_ms(100);
    //Configure Timer 0
    OPTION_REG = 0b11000100;
    //Configure Timer 1
    T1CON = 0b00000001;
    //Configure A/D Module
    regdata = 0b00010011;
    ADCON1 = 0x10;             //Conversion Clock = Fosc/8
    GIE = 1 ;
    T0IE = 1;
    INTE = 1;
    //PEIE = 1;
    //TMR1IE = 1;
      TMR1IF = 0;
  //  TMR1H = 0x00;
    count = 0;
    LedGreen = 1;
    LedRed = 1;
    OUT = 1;
    xcount = 1;
    xdiff = 0;
    upflag = 0;
    dwnflag = 0;
    Tcount = 0;
    memory = 0;
    memflag = 0;
    data = 0;
    while (1)
    {
        if (BLAM)                       // if RC5 is let to float high, then
        {                               // this routine is called to turn thin input from AN4
            data = Getdata(regdata);    //calls function to get A/D value
             if (data < 10)               // if data is low, turn off triac altogether.
            {
                data = 5;
            }                           // 
            else
            {
                if (data >= 255)             // if data is high, then give triac max value.
                {
                    data = 255;
                }
            }
        }
//----------------------------------------------------------------------------------------------        
        else
        {
            if (UP == 0)             // start of up switch loop
            {
                __delay_ms(50);
                if (UP == 0)
                {
                    Bright();
                }
            }
//---------------------------------------------------------------------------------            
            if (!DOWN)                       // start of down switch loop
            {
                __delay_ms(50);
                if (DOWN == 0)
                {
                    Dim();
                }
            }
            if (UP == 1 && DOWN == 1)
            {
                if ( memflag == 1)
                {            
                    memory = xcount;
                }
            }
        }
    }
}
