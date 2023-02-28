/*
 * File:   smt_main.c
 * Author: nemet
 *
 * Created on 2023. február 27., 19:03
 */
// PIC16F1619 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switch Over (Internal External Switch Over mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config ZCD = OFF        // Zero Cross Detect Disable Bit (ZCD disable.  ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PLLEN = ON       // PLL Enable Bit (4x PLL is always enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// CONFIG3
#pragma config WDTCPS = WDTCPS1F// WDT Period Select (Software Control (WDTPS))
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config WDTCWS = WDTCWSSW// WDT Window Select (Software WDT window size control (WDTWS bits))
#pragma config WDTCCS = SWC     // WDT Input Clock Selector (Software control, controlled by WDTCS bits)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#define _XTAL_FREQ 32000000
#include <xc.h>
#include <stdio.h>
void smt_init(void)
{
    
    SMT1CON1bits.MODE = 0b0111;//capture mode
    SMT1CON1bits.SMT1GO = 1;
    SMT1CON1bits.REPEAT =1;
    SMT1CLKbits.CSEL = 1;
    //smtwin = ra5
    SMT1CON0bits.EN = 1;
    
}
void uart_init(void)
{
    TX1STAbits.TXEN = 1;
    TX1STAbits.BRGH = 1;
    RC1STAbits.SPEN = 1;
    BAUD1CONbits.BRG16 = 1;
    SPBRGH = 0;
    SPBRGL =  70;
    RC7PPS = 0b10010;
    
    
}

static char dataAvailable;
static long lastValue,actValue;

void main(void) {
    TRISBbits.TRISB7 = 0;
    TRISAbits.TRISA5 = 1;

    TRISCbits.TRISC7 = 1;
    OSCCONbits.SCS = 0;
    OSCCONbits.IRCF = 0b1111;
    NOP();
    NOP();
    NOP();
    NOP();
    
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE4bits.SMT1PRAIE = 1;
    smt_init();
    uart_init();
    while(1)
    {
        
        if(dataAvailable)
        {
        printf("SMT1CPR: %ld\n",actValue);
        __delay_ms(50);
        dataAvailable = 0;
        }
        
    }
    return;
}
void __interrupt () myIsr (void)
{
    if(PIR4bits.SMT1PRAIF)
    {
        actValue = SMT1CPR-lastValue;
    lastValue = SMT1CPR;
    LATBbits.LATB7 ^= 1;
    dataAvailable = 1;
    PIR4bits.SMT1PRAIF = 0;
    }
}
void putch(char c)
{
       TX1REG = c;
       while(TX1STAbits.TRMT != 1);
}