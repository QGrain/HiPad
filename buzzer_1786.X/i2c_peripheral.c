#include "i2c_peripheral.h"

unsigned char IICAddr;

void iicPeripheralInterruptTx(unsigned char data[], unsigned char size) {
    SSPCONbits.CKP = 0;
    unsigned char i = 0;
    unsigned char tmp;
    //Read operation
    if(SSPSTATbits.R_nW == 1) {
        //Clear interupt flag
        PIR1bits.SSP1IF = 0;
        tmp = SSP1BUF;
        SSPSTATbits.BF = 0;
        SSP1BUF = data[i];
        SSPCONbits.CKP = 1;
        while(PIR1bits.SSP1IF == 0);
        PIR1bits.SSP1IF = 0;
        i++;
        
        while(SSPCON2bits.ACKSTAT == 0) {
            SSP1BUF = data[i];
            SSPCONbits.CKP = 1;
            while(PIR1bits.SSP1IF == 0);
            if (i + 1 < size) {
                i++;
            }
            PIR1bits.SSP1IF = 0;
        }
    }
    PIR1bits.SSP1IF = 0;
}


unsigned char iicPeripheralInterruptRx() {
    PIR1bits.SSP1IF = 0;
    unsigned char tmp;
    tmp = SSP1BUF;
    SSPSTATbits.BF = 0;
    SSPCONbits.CKP = 1;
    unsigned char data = 0;
    while (SSPSTATbits.BF == 0);
    SSPCON2bits.ACKDT = 1; //ACK 0 or 1, 0 is active
    SSPCON2bits.ACKEN = 1; //在SDA和SCL引脚上启动应答顺序，并发送ACKDT数据位
    while (!PIR1bits.SSP1IF); //等待应答发送结束
    PIR1bits.SSP1IF = 0; //SSPIF标志清0
    data = SSP1BUF;
    SSPCONbits.CKP = 1;
    PIR1bits.SSP1IF = 0;
    return data;
}