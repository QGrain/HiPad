#include "i2c_center.h"

#define MAX_READ_BUFFER_SIZE 10

void delay_time(int delay_time);
unsigned char recievedData[MAX_READ_BUFFER_SIZE];

unsigned char* centerReadFromPeripheral(unsigned char peripheralAddr, unsigned char dataSize) {
    if (dataSize <= 0) {
        return 0;
    }
    IIC_Start();         //Start condition
    IIC_Write_Byte((peripheralAddr << 1) + 1);     //7 bit address + Read
    for (unsigned char i = 0; i < dataSize; i++) {
        if (i == dataSize - 1) {
            recievedData[i] = IIC_Read_Byte(1);
        } else {
            recievedData[i] = IIC_Read_Byte(0);
        }
    }
    IIC_Stop();          //Stop condition
    return recievedData;
}

void centerWriteToPeripheral(unsigned char peripheralAddr, unsigned char data) {
    IIC_Start();         //Start condition
    IIC_Write_Byte((peripheralAddr << 1) + 0);     //7 bit address + write
    IIC_Write_Byte(data);
    IIC_Stop();          //Stop condition
}

unsigned char IIC_Read_Byte(unsigned char ack) {
    //Read one byte
    unsigned char b;
    SSP2CON2bits.RCEN = 1; //使能IIC接收模式
    while (!PIR3bits.SSP2IF);
    PIR3bits.SSP2IF = 0;
    b = SSP2BUF;
    SSP2STATbits.BF = 0;
    IIC_ACK(ack);
    return b;
}

void IIC_Write_Byte(unsigned char d) {
    SSP2BUF = d; //写入SSPBUF
    while (!PIR3bits.SSP2IF); //等待发送结束
    PIR3bits.SSP2IF = 0; //SSPIF标志清0
}

void IIC_ACK(unsigned char ack) {
    //The master ACK 0 or 1
    SSP2CON2bits.ACKDT = (ack & 0x01); //ACK 0 or 1, 0 is active
    SSP2CON2bits.ACKEN = 1; //在SDA和SCL引脚上启动应答顺序，并发送ACKDT数据位
    while (!PIR3bits.SSP2IF); //等待应答发送结束
    PIR3bits.SSP2IF = 0; //SSPIF标志清0
}

void IIC_Start() {
    PIR3bits.SSP2IF = 0;
    SSP2CON2bits.SEN = 1; // Start signal
    while(!PIR3bits.SSP2IF);
//    delay_time(2000);
    PIR3bits.SSP2IF = 0;
}

void IIC_Stop() {
    SSP2CON2bits.PEN = 1; //产生IIC停止信号
    while (!PIR3bits.SSP2IF); //等待发送结束
    PIR3bits.SSP2IF = 0; //SSPIF标志清0
}

void delay_time(int delay_time) {
    int i = delay_time;
    while (i > 0) {
        --i;
    }
}