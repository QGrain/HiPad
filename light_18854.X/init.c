/*
 * File:   init.c
 * Author: cppzh
 *
 * Created on 2020年1月19日, 上午10:21
 */


#include "init.h"
#include "i2c_center.h"

void initHardware(bool isCenterBoard, unsigned char iicAddr) {
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;

    LATA = 0;
    LATB = 0;
    LATC = 0;

    ANSELA = 0;
    ANSELB = 0;

    TRISA = 0;
    TRISB = 0;
    TRISC = 0;

    //Set system clock
    //System clock will be decided by IRCF<3:0>
//    OSCCON1bits.SCS = 0b11;
//
//    //4MHz
//    OSCCON1bits.IRCF = 0b1101;
//    OSCCON1bits.SPLLEN = 0;

    //Enable interrupt service
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    if (isCenterBoard) {
        initIICCenterMode();
    }
}

void initIICCenterMode() {
     // Set RB4(SCL) RB5(SDA) as input mode for iic center mode
     //RB4 input
    TRISBbits.TRISB4 = 0;
    ANSELBbits.ANSB4 = 0;
    //RB5 input
    TRISBbits.TRISB5 = 0;
    ANSELBbits.ANSB5 = 0;

     //Initialize IIC
     SSP1CON1 = 0;

     //Set as IIC center mode
     SSP1CON1bits.SSPM0 = 0;
     SSP1CON1bits.SSPM1 = 0;
     SSP1CON1bits.SSPM2 = 0;
     SSP1CON1bits.SSPM3 = 1;

     //Set IIC baud speed as 100kHz (System clock is 4MHz)
     SSP1ADD = 0x09;

     //Enable SDA and SCL ports
     SSP1CON1bits.SSPEN = 1;

     SSP1CON2 = 0;
     SSP1STAT = 0x80;

     PIE3bits.SSP1IE = 1;
     PIR3bits.SSP1IF = 0;
}