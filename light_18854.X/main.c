/*
 * File:   main.c
 * Author: zhiyu
 *
 * Created on 2020/8/12
 */


#include <xc.h>
//#include <pic16f18854.h>
#include "i2c_center.h"
#include "init.h"

#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
//#pragma config LVP = ON

#define TURN_ON(_r, _g, _b) do{ \
    while(PIR3bits.TXIF != 1); \
    TX1REG = _g; \
    while(PIR3bits.TXIF != 1); \
    TX1REG = _r; \
    while(PIR3bits.TXIF != 1); \
    TX1REG = _b; \
} while(0);

#define LIGHT_NUM 16

void init_oc();
void set_pps();
void set_cwg();
void set_clc();
void set_interrupt();
void init_port();
void set_eusart();
void enable_out();
void disable_out();
void out_reset();
void turn_off_all(int num);
void delay(int delay_time);
void lightup();

unsigned char led_arr[LIGHT_NUM] = {
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF};

//
//void interrupt irs_routine() {
//   if (PIR0bits.INTF == 1 || PIR3bits.RCIF == 1) {
//       PIE0bits.INTE = 0;
//       disable_out();
//       enable_out();
//       out_reset();
//       PIR0bits.INTF = 0;
//       PIE0bits.INTE = 1;
//   }
//}



void main(void) {
    init_oc();
    init_port();
    set_interrupt();

    set_pps();
    set_eusart();
    set_cwg();
    set_clc();
    
    unsigned char peripheralAddr = 50;
    unsigned char data = 1;
    initHardware(1, peripheralAddr);
    centerWriteToPeripheral(peripheralAddr, data);
    
    while(1){
        lightup();
    }
    return;
}

void lightup() {
    int led_addr = -1;
    unsigned char led_val;
    int i = 0;
    // int status = 0;

    enable_out();
    out_reset();

    while (1) {
        if (++led_addr == LIGHT_NUM) {
            led_addr = 0;
        }
        led_val = led_arr[led_addr];
        int R = ((led_val >> 2) & 0x03) << 4;
        int G = ((led_val >> 4) & 0x03) << 4;
        int B = ((led_val >> 6) & 0x03) << 4;

        TURN_ON(R, G, B);
        // if(++status >= 30)
        //     status = 0;
        // if(status <= 10){
        //     TURN_ON(R, 0x00, 0x00);
        // }
        // else if(status <= 20){
        //     TURN_ON(0x00, G, 0x00);
        // }
        // else if(status <= 30){
        //     TURN_ON(0x00, 0x00, B);
        // }
        // else {
        //     TURN_ON(0x00, 0x00, 0x00);
        // }
        
        if (++i == LIGHT_NUM) {
            i = 0;
            disable_out();
            enable_out();
            out_reset();

            if (PORTAbits.RA0 == 1) {
                goto Exit;
            }
        }
    }

Exit:
    turn_off_all(LIGHT_NUM);
    disable_out();
    return;
}

void init_oc() {
    OSCCON1bits.NOSC = 0b000;
    OSCCON1bits.NDIV = 0b0000;
    OSCFRQbits.HFFRQ = 0b0111;
    // OSCFRQbits.HFFRQ = 0b110;
}

void set_interrupt() {
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE0bits.INTE = 1;

    INTCONbits.INTEDG = 1;
    INTPPS = 0x00;

    // T0CON0bits.T0EN = 1;
    // T0CON0bits.T016BIT = 0;
    // T0CON0bits.T0OUTPS = 0b0000;
    // T0CON1bits.T0CS = 0b010;
    // T0CON1bits.T0ASYNC = 0;
    // T0CON1bits.T0CKPS = 0b1100;
    // TMR0H = 0x7C;
}

void init_port() {
    // //RA0
    // TRISAbits.TRISA0 = 1;
    // ANSELAbits.ANSA0 = 0;
    // WPUAbits.WPUA0 = 1;
//    //RC1
//    TRISCbits.TRISC1 = 0;
//    ANSELCbits.ANSC1 = 0;
//    //RC2
//    TRISCbits.TRISC2 = 0;
//    ANSELCbits.ANSC2 = 0;
//    //RC3
//    TRISCbits.TRISC3 = 0;
//    ANSELCbits.ANSC3 = 0;
//    //RC6
//    TRISCbits.TRISC6 = 0;
//    ANSELCbits.ANSC6 = 0;
//    //RC7
//    TRISCbits.TRISC7 = 0;
//    ANSELCbits.ANSC7 = 0;
    //RB4 input
    TRISBbits.TRISB4 = 0;
    ANSELBbits.ANSB4 = 0;
    //RB5 input
    TRISBbits.TRISB5 = 0;
    ANSELBbits.ANSB5 = 0;
    
    TRISC = 0;
    ANSELC = 0;
}

void set_eusart() {
    //set BRC default
    BAUD1CONbits.BRG16 = 1;
    SP1BRGL = 0xce;
    SP1BRGH = 0x00;

    //set sync master clock from BRC
    TX1STAbits.CSRC = 1;
    TX1STAbits.SYNC = 1;

    TX1STAbits.TX9 = 0;
    RC1STAbits.RX9 = 0;

    RC1STAbits.SPEN = 1;
    RC1STAbits.CREN = 0;
}

void set_pps() {
    //EURSART OUT: DT->RC7, TC/CK->RC6
    RC7PPS = 0x11;
    RC6PPS = 0x10;

    //CWG IN: IN1<-CWG clock, IN2<-RC6(CK)
    CWG1PPS = 0x16;
    //CWG OUT: CWGA->RC1, CWGB->RC2
    RC1PPS = 0x05;
    RC2PPS = 0x06;
    
    //SCK SDL IN: RB4 <- RC3(SCL), RB5 <- RC4(SDA)
    SSP1CLKPPS = 0x0c;
    SSP1DATPPS = 0x0d;

    //CLC IN: CLCIN0<-RC7(DT), CLCIN1<-RC6(CK), CLCIN2<-RC1(CWGA), CLCIN3<-RC2(CWGB)
    // CLCIN0PPS = 0x17;
    // CLCIN1PPS = 0x16;
    CLCIN2PPS = 0x11;
    CLCIN3PPS = 0x12;
    //CLC OUT: CLC1->RC3
    RC3PPS = 0x01;
    ANSELCbits.ANSC3 = 0;
    TRISCbits.TRISC3 = 0;
}

void set_cwg() {
    //port set input
    TRISCbits.TRISC1 = 1;
    TRISCbits.TRISC2 = 1;

    CWG1CON0bits.EN = 0;
    CWG1CON0bits.CWG1MODE = 0b100;
    //set dead-band
    CWG1DBR = 0x0C;
    CWG1DBF = 0x07;

    //set auto-shutdown
    CWG1AS0bits.REN = 0;
    CWG1AS0bits.LSAC = 0b10;
    CWG1AS0bits.LSBD = 0b10;
    CWG1AS1bits.AS0E = 0;

    //FOSC->input
    CWG1CLKCONbits.CS = 0;
    CWG1ISMbits.IS = 0b0000;

    //port set output
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
}

void set_clc() {
    CLC1CONbits.LC1EN = 0;

    //sel0<-CWGB(CLCIN3), sel1<-DT, sel2<-CWGA(CLCIN2), sel3<-CK
    CLC1SEL0bits.LC1D1S = 0b000011;
    CLC1SEL1bits.LC1D2S = 0b100100;
    CLC1SEL2bits.LC1D3S = 0b000010;
    CLC1SEL3bits.LC1D4S = 0b100101;

    CLC1GLS0 = 0x02; //~(CWGB) -> lc1gl
    CLC1POLbits.LC1G1POL = 1;
    CLC1GLS1 = 0x08; //DT -> lc1g2
    CLC1POLbits.LC1G2POL = 0;
    CLC1GLS2 = 0x22; //~(CWGA+CWGB) -> lc1g3
    CLC1POLbits.LC1G3POL = 1;
    CLC1GLS3 = 0x80; //CK -> lc1g4
    CLC1POLbits.LC1G4POL = 0;

    CLC1CONbits.LC1MODE = 0b000;
}

void enable_out() {
    //ensble eusart
    TX1STAbits.TXEN = 1;
    TX1STAbits.SYNC = 1;
    RC1STAbits.CREN = 0;

    //enable cwg
    CWG1CON0bits.EN = 1;

    //enable clc
    CLC1CONbits.LC1EN = 1;
}

void delay(int delay_time) {
    int i = delay_time;
    while (i > 0) {
        --i;
    }
}

void disable_out() {
    delay(100);
    //ensble eusart
    TX1STAbits.TXEN = 0;
    TX1STAbits.SYNC = 0;
    RC1STAbits.CREN = 1;

    //enable cwg
    CWG1CON0bits.EN = 0;

    //enable clc
    CLC1CONbits.LC1EN = 0;
}

void out_reset() {
    LATBbits.LATB5 = 0;
    delay(88); // i = 88 time = 50.3125us; i = 100 time = 57.0625us
}

void turn_off_all(int num) {
    int i = num * 3;
    while (i > 0) {
        while (PIR3bits.TXIF != 1);
        TX1REG = 0x00;
        --i;
    }
}