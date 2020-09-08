#ifndef IIC_PERIPHERAL_H
#define	IIC_PERIPHERAL_H

#include <xc.h> 

#define BROADCAST_ADDR 127

// ACK = 0 means center device wants to continue recieve data; ACK = 1 to stop recieve data.
void iicPeripheralInterruptTx(unsigned char data[], unsigned char size);
extern unsigned char IICAddr;
unsigned char iicPeripheralInterruptRx();

#endif