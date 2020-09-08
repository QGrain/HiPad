#ifndef IIC_CENTER_H
#define	IIC_CENTER_H

#include <xc.h> 

// For center mode
unsigned char* centerReadFromPeripheral(unsigned char peripheralAddr, unsigned char dataSize);
void centerWriteToPeripheral(unsigned char peripheralAddr, unsigned char data);

unsigned char IIC_Read_Byte(unsigned char ack);
void IIC_Write_Byte(unsigned char d);
void IIC_ACK(unsigned char ack);
void IIC_Start();
void IIC_Stop();
#endif