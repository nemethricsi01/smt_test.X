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
#pragma config IESO = OFF        // Internal/External Switch Over (Internal External Switch Over mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config ZCD = OFF        // Zero Cross Detect Disable Bit (ZCD disable.  ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PLLEN = ON       // PLL Enable Bit (4x PLL is always enabled)
#pragma config STVREN = OFF      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
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
#include<stdint.h>
void smt_init(void)
{
    SMT1CON0bits.EN = 1;
    SMT1CON1bits.MODE = 0b0010;//capture mode
    SMT1CON1bits.REPEAT =1;
    SMT1CON0bits.SMT1PS = 3;
    SMT1PR = 0xffffff;
    SMT1CON1bits.SMT1GO = 1;
    
    SMT2CON1bits.MODE = 0;//timer mode
    SMT2CON1bits.REPEAT =1;
    SMT2CON0bits.SMT2PS = 1;
    SMT2CON0bits.EN = 1;
    SMT2CON1bits.SMT2GO = 1;
    
}
void uart_init(void)
{
    TX1STAbits.TXEN = 1;
    TX1STAbits.BRGH = 1;
    RC1STAbits.SPEN = 1;
    BAUD1CONbits.BRG16 = 1;
    SPBRGH = 0;
    SPBRGL =  70;//115200bps
    RC7PPS = 0b10010;//Uart to RC7
    
    
}

volatile char dataAvailable;
volatile long smt2Value;

void main(void) {
    TRISBbits.TRISB7 = 0;//debug
    TRISAbits.TRISA4 = 1;//smt1 signal in(default)
    ANSELAbits.ANSA4 =0;//analog enabled by default.....
    TRISBbits.TRISB6 = 0;//CLC out
    TRISCbits.TRISC7 = 1;
    OSCCONbits.SCS = 0;
    OSCCONbits.IRCF = 0b1110;//32Mhz clock
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE4bits.SMT1PRAIE = 1;
    PIE4bits.SMT2IE = 1;
    OSCTUNEbits.TUN = 0b011111;//probably dont need it
    
    smt_init();
    uart_init();
    CLC1CON = 0x86;
    CLC1GLS0 = 0x02;
    CLC1GLS1 = 0;
    CLC1GLS2 = 0;
    CLC1GLS3 = 0;
    CLC1POL = 0xA;
    CLC1SEL0 = 0x14;
    CLC1SEL1 = 0;
    CLC1SEL2 = 0;
    CLC1SEL3 = 0;
    RB6PPS = 0b00100;//CLC out to RC6
    CLC1CONbits.EN = 1;
    while(1)
    {
        if(dataAvailable)
        {
            smt2Value = ((SMT1CPR&0xffffff)/25);
            printf("%ld\n",SMT1CPR&0xffffff);
            dataAvailable = 0;
        }
    }
    return;
}
void __interrupt () myIsr (void)
{
    if(PIR4bits.SMT1PRAIF)
    {      
        dataAvailable = 1;
        LATBbits.LATB7 = ~LATBbits.LATB7;//just for debug
        PIR4bits.SMT1PRAIF = 0;
    }
    if(PIR4bits.SMT2IF)
    {
        PIR4bits.SMT2IF = 0;
        SMT2TMR = 16777216-smt2Value;
    }
}
void putch(char c)//printf redirect
{
       TX1REG = c;
       while(TX1STAbits.TRMT != 1);
}