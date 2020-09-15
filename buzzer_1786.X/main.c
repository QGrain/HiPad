/*
 * File:   main.c
 * Author: Zhiyu
 *
 * Created on 2020年8月23日, 上午10:22
 */

//// CONFIG1
//#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
//#pragma config WDTE = OFF        // Watchdog Timer Enable (WDT enabled)
//#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
//#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
//#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
//#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
//#pragma config BOREN = OFF       // Brown-out Reset Enable (Brown-out Reset enabled)
//#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
//#pragma config IESO = OFF        // Internal/External Switchover (Internal/External Switchover mode is enabled)
//#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)
//
//// CONFIG2
//#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
//#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable bit (Vcap functionality is disabled on RA6.)
//#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
//#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
//#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
//#pragma config LPBOR = OFF      // Low Power Brown-Out Reset Enable Bit (Low power brown-out is disabled)
//#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// CONFIG1
#pragma config FOSC = ECH       // Oscillator Selection (ECH, External Clock, High Power Mode (4-32 MHz): device clock supplied to CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable bit (Vcap functionality is disabled on RA6.)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low Power Brown-Out Reset Enable Bit (Low power brown-out is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "i2c_peripheral.h"
#include "init.h"

unsigned int cnt_0 = 0;
unsigned int cnt_1 = 0;
unsigned int cnt_2 = 0;
unsigned int cnt_i = 0;
unsigned int cnt_j = 0;
// unsigned int voice[10] = {43, 38, 34, 32, 29, 25, 22, 21, 19, 17};
//unsigned int voice[18] = {0, 86, 76, 68, 64, 57, 51, 46, 43, 38, 34, 32, 29, 25, 22, 21, 19, 17};
                        //OO,C3, D3, E3, F3, G3, A3, B3, C4, D4, E4, F4, G4, A4, B4, C5, D5, E5
unsigned int voice[15] = {0, 64, 57, 51, 46, 43, 38, 34, 32, 29, 25, 22, 21, 19, 17};
                        //OO,F3, G3, A3, B3, C4, D4, E4, F4, G4, A4, B4, C5, D5, E5
unsigned int book[24] = {6, 10, 12, 10, 6, 9, 6, 8, 6, 8, 10, 10, 9, 10, 8, 6, 10, 12, 13, 13, 13, 10, 13, 0};
//unsigned int book[17] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
unsigned int time[24] = {4, 2, 1, 2, 2, 4, 1, 1, 1, 1, 4, 1, 1, 1, 1, 4, 2, 1 ,2, 2, 2, 2, 4, 4};
unsigned int i = 0, j = 0;
unsigned char count = 60;
unsigned char recvData;
unsigned int play = 0;

void Init();
void Enable_INT();
void Pull_Up();
void delay_time(int cnt);
void base_sound(int gate0);
void sound1(int gate1);
void sound2(int gate2);
void i2c_isr();

void interrupt irs_routine(void)
{
    if(PIR1bits.SSP1IF == 1){
        i2c_isr();
    }
    return;
}

void base_sound(int gate0)
{
    int time = 2000;
    while(--time) {
        if(gate0) {
            if(++cnt_0 >= gate0>>1) {
                LATBbits.LATB0 = !LATBbits.LATB0;
                cnt_0 = 0;
            }
        }
    }
    recvData = 0;
}

void sound1(int gate1)
{
    if(++cnt_i >= time[i]*1200) {
        cnt_i = 0;
        i = i + 1;
        if(i >= 24) i = 0;
    }
    
    if(gate1) {
        if(++cnt_1 >= gate1>>1) {
            LATBbits.LATB0 = !LATBbits.LATB0;
            cnt_1 = 0;
        }
    }
}

void sound2(int gate2)
{
    if(++cnt_j >= time[j]*4000) {
        cnt_j = 0;
        j = j + 1;
        if(j >= 24) j = 0;
    }
    
    if(gate2) {
        if(++cnt_2 >= gate2) {
            LATBbits.LATB1 = !LATBbits.LATB1;
            cnt_2 = 0;
        }
    }
}

void i2c_isr() {
    // IIC received request;
    if (SSPSTATbits.D_nA == 0 && SSPSTATbits.R_nW == 0) {
        //Center write
        recvData = iicPeripheralInterruptRx();
        if(recvData == 0b00000001) {
            play = 1;
        }
        else if(recvData == 0b00000010) {
            play = 0;
        }
    }
    PIR1bits.SSP1IF = 0;
}

void main(void) {
    OSCCON = 0b11111011; //设置中断源为内部振荡器，切频率为16MHz
    Init();
    Enable_INT();
    Pull_Up();
    WPUCbits.WPUC3 = 1;
    WPUCbits.WPUC4 = 1;

    unsigned char peripheralAddr = 50;
    initHardware(0, peripheralAddr);
    
    // GPIO
    TRISB = 0;
    LATBbits.LATB0 = 0;
    
    while(1) {
        if(play)
            sound1(voice[book[i]]);
        else {
            switch(recvData) {
                case 3: base_sound(voice[recvData+3]);break;
                case 4: base_sound(voice[recvData+3]);break;
                case 5: base_sound(voice[recvData+3]);break;
                case 6: base_sound(voice[recvData+3]);break;
                case 8: base_sound(voice[recvData+2]);break;
                case 9: base_sound(voice[recvData+2]);break;
                default:break;
            }
        }
    }   
}


void delay_time(int cnt)
{
    while(--cnt > 0);
}

void Init()
{
    
    TRISA = 0x07;
    ANSELA = 0;
    
    TRISB = 0;
    ANSELB = 0;
    
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;    
    return;
}

void Enable_INT()
{
    GIE = 1;
    PEIE = 1;
    
    PIR1 = 0;
    PIE1 = 0;
    return;
}

void Pull_Up()
{
    OPTION_REGbits.nWPUEN = 0;
    return;
}