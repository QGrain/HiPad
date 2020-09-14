#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config CLKOUTEN = OFF    // Clock Out Enable bit (CLKOUT function is enabled; FOSC/4 clock appears at OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config WRT = OFF        // UserNVM self-write protection bits (Write protection off)
#pragma config SCANE = available// Scanner Enable bit (Scanner module is available for use)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (Program Memory code protection disabled)
#pragma config CPD = OFF        // DataNVM code protection bit (Data EEPROM code protection disabled)

#include <xc.h>
#include <pic16f18854.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"
#include "iic.h"
#include "bt.h"

void set_pps();
void set_interrupt();
void init_port();
void delay(uint32_t delay_time);
void motor1_run(void);

void __interrupt() irs_routine() {
    if (PIR3bits.RCIF) { // don't call any print function here
        recv_handler();
    }
}

void main(void) {
    // initialization
    recvd_char = 0;
    is_recvd = 0;

    // init_oc();
    init_port();
    set_interrupt();
    set_pps();
    init_eusart();
    init_iic();

    // main loop
//    char reg_buf[3];
    printf("Hello!\n");
    iic_start();
    printf("World!\n");
    while (1) {
        // iic read register
//        read_reg(MPU6050_DEFAULT_ADDRESS, , 1, reg_buf);
//        iic_write_byte(0x68, 1);
        
        // echo code
        if (is_recvd) { // check if reception
            char ch = recvd_char;
            printf("%c\n", ch);
            is_recvd = 0; // clear the flag
        }
    }

    return;
}

void init_port() {
    // motor1
    // RA5 out
    TRISAbits.TRISA5 = 0;
    // RA7 out
    TRISAbits.TRISA7 = 0;

    //motor2
    // RC1 out
    TRISAbits.TRISA6 = 0;
    // RC2 out
    TRISCbits.TRISC1 = 0;

    //motor1
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;
}

void init_accelerometer(void) {
    return;
}

void init_gyroscope(void) {
    return;
}

void set_interrupt() {
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;

    PIE3bits.RCIE = 1;
}

void set_pps() {
    // ref https://www.youtube.com/watch?v=tf2SfSm6fQg
    // eusart out: TX->RC6
    RC6PPS = 0x10;
    // eusart in:  RX->RC5
    RXPPS = 0x15;
}

void delay(uint32_t delay_time) {
    uint32_t i = delay_time;
    while (i > 0) {
        --i;
    }
}

void motor1_run(void) {
    LATAbits.LATA5 = 1;
}

//void out_reset() {
//    delay(88); // i = 88 time = 50.3125us; i = 100 time = 57.0625us
//}

//void init_oc() {
//    // HFINTOSC
//    OSCCON1bits.NOSC = 0b000;
//    // 32 MHz
//    OSCFRQbits.HFFRQ = 0b110;
//    // 1x divider
//    OSCCON1bits.NDIV = 0b0000;
//}