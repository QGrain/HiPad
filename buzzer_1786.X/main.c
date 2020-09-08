/*
 * File:   main.c
 * Author: Zhiyu
 *
 * Created on 2020年8月23日, 上午10:22
 */


#include <xc.h>
#include "i2c_peripheral.h"
#include "init.h"

unsigned int cnt_1 = 0;
unsigned int cnt_2 = 0;
unsigned int cnt_3 = 0;
 unsigned int voice[10] = {85, 80, 76, 72, 68, 64, 60, 57, 54, 51};
//unsigned int voice[10] = {43, 40, 38, 36, 34, 32, 30, 29, 27, 25};
                        //C4, D4, E4, F4, G4, A4, B4, C5, D5, E5
unsigned int i = 0;
unsigned char count = 60;
unsigned char recvData;

void Init();
void Enable_INT();
void Pull_Up();
void delay_time(int cnt);
void sound(int gate1);
void i2c_isr();

void interrupt irs_routine(void)
{
    if(PIR1bits.TMR1IF == 1) {
        sound(voice[i]);
        // LATBbits.LATB0 = !LATBbits.LATB0;
        PIR1bits.TMR1IF = 0;
        TMR1H = 0xff;
        TMR1L = 0xfe;
    }
    if(PIR1bits.TMR2IF == 1) {
        sound(voice[i]);
//        LATBbits.LATB0 = !LATBbits.LATB0;
        PIR1bits.TMR2IF = 0;
        TMR2 = 0xfe;
    }
    if(PIR1bits.SSP1IF == 1){
        i2c_isr();
    }
    return;
}

void sound(int gate1)
{
   if(++cnt_2 >= 50) {
        cnt_2 = 0;
        if(++cnt_3 >= 50) {
            cnt_3 = 0;
            i = i + 1;
            if(i >= 10) i = 0;
        }
    }
    
    if(++cnt_1 == gate1) {
        LATBbits.LATB0 = !LATBbits.LATB0;
        cnt_1 = 0;
    }
}

void i2c_isr() {
    // IIC received request
    //PORTAbits.RA1 = ~PORTAbits.RA1;
    if (SSPSTATbits.D_nA == 0 && SSPSTATbits.R_nW == 0) {
        //Center write
        recvData = iicPeripheralInterruptRx();
        count++;
        //PORTAbits.RA1 = recvData % 2;
    } else if (SSPSTATbits.D_nA == 0 && SSPSTATbits.R_nW == 1) {
        //Center read
        unsigned char data = 2;
        iicPeripheralInterruptTx(data, 1);
        count++;
    }
    PIR1bits.SSP1IF = 0;
}

void main(void) {
    OSCCON = 0b01101011; //设置中断源为内部振荡器，切频率为4MHz
    Init();
    Enable_INT();
    // Pull_Up();

    // unsigned char peripheralAddr = 50;
    // initHardware(false, peripheralAddr);
    
    // GPIO
    TRISB = 0b11111100;
    LATBbits.LATB0 = 0;
    
    //Timer1循环次数设置
    TMR1H = 0xff;
    TMR1L = 0xfe;
    T1CONbits.TMR1ON = 1;
    
    //Timer2循环次数设置
    // TMR2 = 0xf5;
    // T2CONbits.TMR2ON = 1;
    // while(1) {
    //     if(recvData == 1)
    //         T2CONbits.TMR2ON = 1;
    //     else 
    //         T2CONbits.TMR2ON = 0;
    // }
    
}


void delay_time(int cnt)
{
    while(--cnt > 0);
}

void Init()
{
    
    TRISA = 0;          //8 seg_sel
    ANSELA = 0;
    
    TRISB = 0;
    ANSELB = 0;
    
    TRISCbits.TRISC3 = 0;
    TRISCbits.TRISC4 = 0;
    
    T1CON = 0b01000000;
    // T2CON = 0;
    
    return;
}

void Enable_INT()
{
    GIE = 1;
    PEIE = 1;
    
    PIR1 = 0;
    PIE1 = 0;
    PIE1bits.TMR1IE = 1;
    // PIE1bits.TMR2IE = 1; 
    return;
}

void Pull_Up()
{
    OPTION_REGbits.nWPUEN = 0;
    return;
}