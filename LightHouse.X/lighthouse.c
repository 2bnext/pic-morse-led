/*
 * File:   lighthouse.c
 * Author: Wino
 *
 * Created on November 4, 2016, 5:09 PM
 */

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection 
                                // (INTOSC oscillator: I/O function on CLKIN pin)
                                
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select 
                                // (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection 
                                // (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection 
                                // (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable 
                                // (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. 
                                // I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover 
                                // (Internal/External Switchover mode is enabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable 
                                // (Fail-Safe Clock Monitor is disabled)
// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection 
                                // (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable 
                                // (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection 
                                // (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable 
                                // (Low-voltage programming enabled)

// -----------------------------------------------------------------------------

#define _XTAL_FREQ 31000

#include <xc.h>
#include <pic.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------

#define LED LATA0
#define PWR LATA1
#define DURATION 250
#define MORSE(c, n) ((c << 3) | n)

typedef unsigned char byte;

// -----------------------------------------------------------------------------

byte Enabled  = 1;

byte MorseAZ[] =
{
    MORSE(0b0001, 2),   // A
    MORSE(0b1000, 4),   // B
    MORSE(0b1010, 4),   // C
    MORSE(0b0100, 3),   // D
    MORSE(0b0000, 1),   // E
    MORSE(0b0010, 4),   // F
    MORSE(0b0110, 3),   // G
    MORSE(0b0000, 4),   // H
    MORSE(0b0000, 2),   // I
    MORSE(0b0111, 4),   // J
    MORSE(0b0101, 3),   // K
    MORSE(0b0100, 4),   // L
    MORSE(0b0011, 2),   // M
    MORSE(0b0010, 2),   // N
    MORSE(0b0111, 3),   // O
    MORSE(0b0110, 4),   // P
    MORSE(0b1101, 4),   // Q
    MORSE(0b0010, 3),   // R
    MORSE(0b0000, 3),   // S
    MORSE(0b0001, 1),   // T
    MORSE(0b0001, 3),   // U
    MORSE(0b0001, 4),   // V
    MORSE(0b0011, 3),   // W
    MORSE(0b1001, 4),   // X
    MORSE(0b1011, 4),   // Y
    MORSE(0b1100, 4)    // Z
};
/*
eeprom byte Msg[] = 
        "cq cq cq  "
        "koen de jongen die echt alles weet  "
        "maar practisch nog niet zoveel deed  "
        "toch komt er een dag  "
        "dat iedereen zag  "
        "hoe jij het heelal aan stukken reet  "
        "  "
        "koen je vader houdt van je "
        "voorzichtig met het heelal "
        ;
*/
__eeprom byte Msg[] = 
        "cq cq cq  "
        "joris is man maar eens was hij jongen  "
        "dat heeft zo goed als verdrongen  "
        "er komt een moment  "
        "dan ben je een vent  "
        "zo heeft de natuur dat bedongen  "
        "  "
        "joris je vader houdt van je "
        "jouw levenspad is recht vooruit "
        ;
        
// -----------------------------------------------------------------------------
// Send morse to LED

void Send(byte Code)
{
    byte m = MorseAZ[Code];
    byte n = m & 0b111;
    byte p = m >> 3;
    
    for (byte b = n; b > 0; b--)
    {        
        LED = 1;
        
        if (p & (1 << (b - 1)))
            __delay_ms(DURATION * 3);
        else
            __delay_ms(DURATION);
        
        LED = 0;
        
        __delay_ms(DURATION);
    }
    
    __delay_ms(DURATION * 2);
}

// -----------------------------------------------------------------------------
// Initialize MCU

void Init()
{
    // select 31khz clock
    
    IRCF1 = 0;  
    IRCF2 = 0;
    IRCF3 = 0;
    
    // set pins output, low
    
    TRISA = 0;
    LATA = 0;
    
    // set A5 output pull up
    
    TRISA5 = 1;
    WPUA5 = 1;
    
    // enable A5 change interrupt
    
    IOCAN5 = 1;
    IOCAF5 = 0;
    IOCIE = 1;
    GIE = 1;
    
    // show power
    
    PWR = 1;
}

// -----------------------------------------------------------------------------
// Interrupt routine 

void Interrupt()
{
    if (IOCAF5)
    {
        // pin change interrupt on pin A5
        
        if (RA5 == 0) // this fixes de-bounce issues
        {
            Enabled ^= 1;
            PWR = Enabled;
        }
            
        IOCAF5 = 0;
    }
}

// -----------------------------------------------------------------------------

void main(void) 
{
    Init();
    
    for (;;)
    {
        for (byte *p = Msg; *p; p++)
        {
            if (!Enabled)
            {
                if (RA5 == 0)   // reset message?
                {
                    Enabled = 1;
                    PWR = 1;
                    break;
                }
                    
                PWR = 0;
                SLEEP();
            }
            
            if (*p == ' ')
                __delay_ms(DURATION * 4);
            else
                Send(*p - 'a');
        }
    }
}
