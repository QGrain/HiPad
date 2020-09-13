/*
 * File:   init.c
 * Author: cppzh
 *
 * Created on 2020年1月19日, 上午10:21
 */


#include "init.h"
#include "i2c_peripheral.h"

void initHardware(unsigned int isCenterBoard, unsigned char iicAddr) {
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
    OSCCONbits.SCS = 0b11;

    //4MHz
    OSCCONbits.IRCF = 0b1101;
    OSCCONbits.SPLLEN = 0;

    //Enable interrupt service
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    if (isCenterBoard == 0) {
        initIICPeripheralMode(iicAddr);
    }
}

void initIICPeripheralMode(unsigned char iicAddr) {
    // Set RC3(SCL) RC4(SDA) as input mode for iic peripheral mode
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;

    SSPSTAT = 0x80;
    // Set iic peripheral address
    SSPADD = (iicAddr << 1);
    IICAddr = iicAddr;
    
    //Initialize IIC
    SSPCON1 = 0;

    //Set as IIC pheripheral mode (7bits address)
    SSPCON1bits.SSPM0 = 0;
    SSPCON1bits.SSPM1 = 1;
    SSPCON1bits.SSPM2 = 1;
    SSPCON1bits.SSPM3 = 0;

    //Enable clock
    SSPCON1bits.CKP = 1;
    //Enable IIC
    SSPCON1bits.SSPEN = 1;

    SSPCON2bits.SEN = 1;

    //Enabel IIC interupt
    PIE1bits.SSP1IE = 1; //Enable MSSP interupt
    PIR1bits.SSP1IF = 0;
}