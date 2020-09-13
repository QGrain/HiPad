/*
 * File:   main.c
 * Author: Eldridge Bear
 *
 * Created on 2020?��8??14??, ????11:58
 */
// PIC16F18854 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = ECH    // External Oscillator mode selection bits (EC above 8MHz; PFM set to high power)
#pragma config RSTOSC = HFINT1  // Power-up default value for COSC bits (HFINTOSC (1MHz))
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = SWDTEN    // WDT operating mode (WDT enabled/disabled by SWDTEN bit in WDTCON0)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config WRT = WRT_upper  // UserNVM self-write protection bits (0x0000 to 0x01FF write protected)
#pragma config SCANE = not_available// Scanner Enable bit (Scanner module is not available for use)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (Program Memory code protection disabled)
#pragma config CPD = OFF        // DataNVM code protection bit (Data EEPROM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#define TMP75_BITS 12

#define _8ms_H 0xe0
#define _8ms_L 0xc0
#define _2ms_H 0xf0
#define _2ms_L 0xc8
#define _25ms_H 0x9e
#define _25ms_L 0x58

#define TMR0H_DATA _2ms_H
#define TMR0L_DATA _2ms_L
#define TMR1H_DATA _8ms_H
#define TMR1L_DATA _8ms_L
#define TMR3H_DATA _25ms_H
#define TMR3L_DATA _25ms_L

struct _LED_SHOW{
    char Number;       //��ʾ����
    char Flash;        //�Ƿ���˸,1��0����
    char Node;         //�Ƿ��С����,1��0����
    char FlashStatus;  //��˸״̬,1��0��
};
typedef struct _LED_SHOW LEDShow; 
typedef unsigned char u8;    //�޷���8λ��
typedef unsigned short u16;  //�޷���16λ��

void INT_init(void);
void OSC_init(void);
void LED_init(void);
void GPIO_init(void);
void TMR_init(void);
void MSSP_init(void);
void TMP75_init(u8 Bits);
void TMR_Clear(u8 TMR, u8 High, u8 Low);
void Show(u8 n);
void TMR0_INT(void);
void TMR1_INT(void);
void P_Plus(void);
void R_Plus(void);
void E_Plus(void);
void scan_select(void);
void scan(void);
void ShortClick(void);
void DoubleClick(void);
void LongClick(void);
void IIC2_Start(void);
void IIC2_Stop(void);
u8 IIC2_SendByte(u8 Byte);
u8 IIC2_RecvByte(void);
float ReadTMP75(u8 Bits);
void TMR3_INT(void);

u8 num_table[32]={0xFC,0x60,0xDA,0xF2,0x66,0xB6,0xBE,0xE0,0xFE,0xF6,0xEE,0x3E,0x9C,0x7A,0x9E,0x8E,
                            0xFD,0x61,0xDB,0xF3,0x67,0xB7,0xBF,0xE1,0xFF,0xF7,0xEF,0x3F,0x9D,0x7B,0x9F,0x8F};  //���ұ�
LEDShow LED[4] = {0};  //�����ĸ�����ܵ�״̬��Ϣ
u8 Buffer_Now;   //���¼�⵽�İ����Ļ�����������ͼ�еĻ�����1
u8 Buffer_Last;  //��һ�μ�⵽�İ����Ļ�����������ͼ�еĻ�����2
u8 E_Num;
u8 R_Num;
u8 P_Num;
//u8 Digit[10];  //����ʮ������ֱܷ𱻰��µĴ���

/**
 * @brief �ж����
 * @note  pic18854��Ƭ��ֻ��һ���ж���ڣ������ж϶�����������ڴ˺��������жϲ�ͬ�ж�Դ���벻ͬ���жϷ�����
 */
void __interrupt() isr_routine(void) {
    if (PIR0bits.TMR0IF)  //�ж��ж�Դ
        TMR0_INT();
    else if (PIR4bits.TMR1IF)
        TMR1_INT();
    else if (PIR4bits.TMR3IF)
        TMR3_INT();
    else
        return;
}

void main(void) {
    u8 i = 0;
    OSC_init();   //оƬ��Ƶ��ʼ������
    INT_init();   //�жϳ�ʼ��
    GPIO_init();  //GPIO��ʼ��
    TMR_init();   //��ʱ����ʼ��
    MSSP_init();  //MSSP��ʼ��
    TMP75_init(TMP75_BITS);  //�¶ȴ�����TMP75��+ʼ��
    
//    for (; i < 10; i++)
//        Digit[i] = 0;
    E_Num = 0;
    P_Num = 0;
    R_Num = 200;
    Buffer_Last = 10;
    T0CON0bits.T0EN = 1;  //����ʱ��0
    T1CONbits.ON = 1;  //������ʱ��1
    T3CONbits.ON = 1;  //������ʱ��3
    LED_init();
    while (1) {
    
    }
    return;
}

/**
 * @brief  оƬ��Ƶ��ʼ����������ʼ��Ϊ���32M
 * @note   �����Ƶ���͵Ļ�����ְ���������̹������°��������������󣬽���Ƶ���߻�ʹ����ִ���ٶȸ���
 * @param  None
 * @retval None
 */
void OSC_init(void) {
    OSCCON1 = 0x60;  //ʹ���ڲ���������
    OSCENbits.HFOEN = 1;
    OSCFRQ = 0x06;  //32M
}

/**
 * @brief  �жϳ�ʼ��������������ȫ���жϺͿ���ʱ���ж�
 */
void INT_init(void) {
    INTCON = 0xc0;  //��ȫ���ж�
    PIE0 = 0x20;  //����ʱ��0�ж�
    PIE4 = 0x05;  //����ʱ��1�ж�
    return;
}

/**
 * @brief  �������ʾ��ʼ������
 * @note   Ŀǰֱ����ʾ��ֵSEED��������Ҫ���ڴ˴���ӿ�������
 */
void LED_init(void) {
    LED[0].Number = 0x0d;
    LED[0].Node = 1;
    LED[1].Number = 0x0e;
    LED[2].Number = 0x0e;
    LED[3].Number = 0x05;
    LED[3].Flash = 1;
    return;
}

/**
 * @brief  GPIO��ʼ������
 * @note   PortA�������λѡ�źţ�PortC����λ������ܶ�ѡ�źţ�����λ���ڰ������
 */
void GPIO_init(void) {
    TRISA = 0x0;
    ANSELA = 0x0;
    LATA = 0x0;
    TRISB = 0x06;
    ANSELB = 0x0;
    LATB = 0x0;
    TRISC = 0x0;
    ANSELC = 0x0;
    LATC = 0x0;
    WPUC = 0xff;
    return;
}

/**
 * @brief  ��ʱ����ʼ������
 */
void TMR_init(void) {
    T0CON0 = 0x10;  //16λģʽ
    T0CON1 = 0x43;  //��ʱ��0 Fosc/4ʱ��Դ, Psc 1:8
    T1CLK = 0x01;  //��ʱ��1 Fosc/4ʱ��Դ
    T1CON = 0x30;  //Psc 1:8
    T3CLK = 0x01;  //��ʱ��3 Fosc/4ʱ��Դ
    T3CON = 0x30;  //Psc 1:8
    return;
}

/**
 * @brief �¶ȴ�����оƬTMP75��ʼ������
 * @note  оƬ��ַ����λ�ӵأ�����ַΪ 1001 000x(0x9x)�����һλxֵ���ƶ�д��1��0д
 * @param Bits: ϣ��оƬ��ʼ��Ϊ����λ�ľ��ȣ���9-12λ��ѡ��λ��Խ���ٶ�Խ��ͬʱ����ҲԽ��
 */
void TMP75_init(u8 Bits) {
    if (Bits == 9)
        return;  //оƬ�ϵ�Ĭ��9λģʽ��ֱ���˳�����
    IIC2_Start();  //����IIC2
    IIC2_SendByte(0x90);  //ѡ���¶ȴ�����оƬ��дģʽ
    IIC2_SendByte(0x01);  //ѡ�����üĴ���
    switch (Bits) {
        case 10:
            IIC2_SendByte(0x20);  //�����üĴ�����ֵѡ��10λģʽ
            IIC2_SendByte(0x20);  //�����üĴ�����ֵѡ��10λģʽ
            break;
        case 11:
            IIC2_SendByte(0x40);  //�����üĴ�����ֵѡ��11λģʽ
            IIC2_SendByte(0x40);  //�����üĴ�����ֵѡ��11λģʽ
            break;
        case 12:
            IIC2_SendByte(0x60);  //�����üĴ�����ֵѡ��12λģʽ
            IIC2_SendByte(0x60);  //�����üĴ�����ֵѡ��12λģʽ
            break;
    }
    IIC2_Stop();  //����IIC2
    return;
}

/**
 * @brief  MSSP��ʼ������
 * @note   ����Ҫ����MSSP����Ҫ����PPS����������������Ϊʹ��Ĭ�ϵ�PPS��������RB1��RB2����ֻ������PPS���
 */
void MSSP_init(void) {
    RB1PPS = 0x16;  // SCL2 to RB1
    RB2PPS = 0x17;  // SDA2 to RB2
    SSP2CON1 = 0x28;  //I2C ��ģʽ��ʱ�� = FOSC/(4 * (SSPxADD + 1)), ��Ƶ32M
    SSP2ADD = 0x4f;  //100kHz
}

/**
 * @brief ��ȡTMP75ģ�������
 * @note  �ϸ���TMP75оƬ�ϵ�ʱ��ͼ����
 * @param Bits: ѡ��TMP75ģ��ķֱ���
 * @return  ���ض������¶ȣ��Ը�������ʽ����
 */
float ReadTMP75(u8 Bits) {
    u16 tmp;
    IIC2_Start();
    if (IIC2_SendByte(0x90))  //ѡ���¶ȴ�����оƬ��дģʽ
        return 0;
    if (IIC2_SendByte(0x00))  //ѡ���¶ȼĴ���
        return 0;
    IIC2_Start();
    if (IIC2_SendByte(0x91))  //��ģʽ
        return 0;
    tmp = (u16)IIC2_RecvByte();  //��ȡ�߰�λ
    switch (Bits) {
        case 9:
            tmp = (tmp << 1) + (IIC2_RecvByte() >> 7);  //��ȡ�Ͱ�λ
            IIC2_Stop();
            return tmp * 128.0f / 256;
        case 10:
            tmp = (tmp << 2) + (IIC2_RecvByte() >> 6);  //��ȡ�Ͱ�λ
            IIC2_Stop();
            return tmp * 128.0f / 512;
        case 11:
            tmp = (tmp << 3) + (IIC2_RecvByte() >> 5);  //��ȡ�Ͱ�λ
            IIC2_Stop();
            return tmp * 128.0f / 1024;
        case 12:
            tmp = (tmp << 4) + (IIC2_RecvByte() >> 4);  //��ȡ�Ͱ�λ
            IIC2_Stop();
            return tmp * 128.0f / 2048;
    }
    return 0;
}

/**
 * @brief IIC2��������
 */
void IIC2_Start(void) {
    PIR3bits.SSP2IF = 0;
    SSP2CON2bits.SEN = 1;  //������������
    while (!PIR3bits.SSP2IF) {}  //�ȴ�������ɣ�������ɺ��־λ��һ
    PIR3bits.SSP2IF = 0;  //�����־λ
}

/**
 * @brief IIC2ֹͣ����
 */
void IIC2_Stop(void) {
    SSP2CON2bits.PEN = 1;
    while (!PIR3bits.SSP2IF) {}  //�ȴ�������ɣ�������ɺ��־λ��һ
    PIR3bits.SSP2IF = 0;  //�����־λ
    return;
}

/**
 * @brief IIC2����һ���ֽڵ�����
 * @param Byte: �����Ͷ�����
 * @return ���ͽ����0����1δ�յ�ack
 */
u8 IIC2_SendByte(u8 Byte) {
    SSP2BUF = Byte;  //����Ҫ���͵��������뻺������������
    while (SSP2STATbits.BF) {}  //�ȴ��������
    while (!PIR3bits.SSP2IF) {}  //�ȴ�������ɣ�������ɺ��־λ��һ
    PIR3bits.SSP2IF = 0;  //�����־λ
    if (SSP2CON2bits.ACKSTAT) {
        LED[3].Number = 0;	
        return 1;
    }  //ȷ���յ�Ӧ���ź�
    return 0;
}

/**
 * @brief IIC2����һ���ֽڵ�����
 * @return ���ؽ��յ�������
 */
u8 IIC2_RecvByte(void) {
    u8 temp;
    SSP2CON2bits.RCEN = 1;  //ʹ�ܽ���ģʽ
    while (!PIR3bits.SSP2IF) {}  //�ȴ�������ɣ�������ɺ��־λ��һ
    PIR3bits.SSP2IF = 0;  //�����־λ
    temp  = SSP2BUF;  //���ض�ȡ�������ݣ�cpu��ȡbufʱBF��־�Զ�����
    SSP2CON2bits.ACKDT = 0;  //Ӧ��
    SSP2CON2bits.ACKEN = 1;  //����Ӧ��
    while (!PIR3bits.SSP2IF) {}  //�ȴ�����Ӧ����ɣ�������ɺ��־λ��һ
    PIR3bits.SSP2IF = 0;  //�����־λ
    return temp;  
}

/**
 * @brief  ��ʱ��0�жϷ��������ڴ˺����ڽ�LED[4]��������ʾ��Ϣ��ȡ������ʾ
 * @param  None
 * @retval None
 */
void TMR0_INT(void) {
    PIR0bits.TMR0IF = 0;
    TMR_Clear(0, TMR0H_DATA, TMR0L_DATA);
    static u8 Mode = 0;
    switch (++Mode) {  //�ж����ֵ��ڼ����������ʾ����0-4ѭ��
        case 1: Show(0); break;
        case 2: Show(1); break;
        case 3: Show(2); break;
        case 4: Show(3); Mode = 0; break;
    }
    return;
}

/**
 * @brief  ��ʱ��1�жϷ��������ڴ˺����ڽ��а���ȫɨ����
 * @param  None
 * @retval None
 */
void TMR1_INT(void) {
    PIR4bits.TMR1IF = 0;
    TMR_Clear(1, TMR1H_DATA, TMR1L_DATA);
    scan();  //ȫɨ��
    scan_select();  //����ȫɨ����������Ϊ�ж�
    return;
}

/**
 * @brief ��ʱ��3�жϺ�������ʱ25ms���������cnt10�Σ��൱��250ms
 */
void TMR3_INT(void) {
    static u8 cnt = 10;
    float tmp = 0;
    PIR4bits.TMR3IF = 0;
    TMR_Clear(3, TMR3H_DATA, TMR3L_DATA);
    if (!--cnt) {
        tmp = ReadTMP75(TMP75_BITS);  //��ȡ�¶�
        cnt = 10;
        LED[2].Number = ((u8)tmp)/10;  //��ʾʮλ
        LED[1].Number = ((u8)tmp)%10;  //��ʾ��λ
    }
    return;
}

/**
 * @brief ��ʱ���ͳ�ֵ����
 * @param TMR: ��Ҫ�ͳ�ֵ�Ķ�ʱ��
 * @param High: �ͳ�ֵ�߰�λ
 * @param Low: �ͳ�ֵ�Ͱ�λ
 * @retval None
 */
void TMR_Clear(u8 TMR, u8 High, u8 Low) {
    switch(TMR) {  //ѡ��ʱ��
        case 0:
            TMR0H = High;
            TMR0L = Low;
            return;
        case 1:
            TMR1H = High;
            TMR1L = Low;
            return;
        case 3:
            TMR3H = High;
            TMR3L = Low;
            return;
    }
    return;
}

/**
 * @brief �������ʾ��������LED[4]����������ȡ�����ݽ�����Ӧ�жϲ���ʾ����
 * @param n: ��ʾ���ǵڼ�������ܣ����ҵ�������0-3
 * @retval None
 */
void Show(u8 n) {
    static u8 Twinkle[4] = {0x0f, 0x0f, 0x0f, 0x0f};
    u8 Num;
    Num = LED[n].Number;
    if (LED[n].Flash) {  //�жϸ�λ������Ƿ���Ҫ��˸
        if (--Twinkle[n] == 0) {
            LED[n].FlashStatus ^= 1;  //��������ȡ��
            Twinkle[n] = 0x0f;
        }
        LATA = 0x00;  //����˸״̬��LATA�����㣬��ֹӰ���Աߵ������
        if (!LED[n].FlashStatus) //�������ı����ֱ�ӷ���
            return;
    }
    if(LED[n].Node)  //�жϸ�λ������Ƿ���Ҫ����С����
        Num += 16;   //��Ҫ����С�������������16λ
//    if (Num > 31)  //�ж������Ƿ񳬳���ķ�Χ
//        Num %= 32;
    Num = num_table[Num];  //�ӱ���ȡ����������
    switch (n) {  //����Ӧ������Ͷ�ѡ��λѡ���ݣ�Ϊȷ����ѡ��λѡ���ݾ����������ʽ�λѡ����Ҳ����case��
        case 0: LATA = Num; LATC = 0x0e; break;
        case 1: LATA = Num; LATC = 0x0d; break;
        case 2: LATA = Num; LATC = 0x0b; break;
        case 3: LATA = Num; LATC = 0x07; break;
    }
    return;
}

/**
 * @brief  ȫɨ�躯��
 * @note   �������ȼ���7 8 9 10���ȼ���ߣ�1 2 5�ڶ���3 4��Σ�6���
 *          �����жϹ����������ͼ
 */
void scan(void) {
    PORTC |= 0xf0;
    TRISC = 0xf0;
    //״̬һ
    if(!PORTCbits.RC4){  
        if(PORTCbits.RC5 && PORTCbits.RC6 && PORTCbits.RC7)
            Buffer_Now = 7;
        else
            Buffer_Now = 100;
        return;
    }
    else if(!PORTCbits.RC5){
        if(PORTCbits.RC6 && PORTCbits.RC7)
            Buffer_Now = 8;
        else
            Buffer_Now = 100;
        return;
    }
    else if(!PORTCbits.RC6){
        if( PORTCbits.RC7)
            Buffer_Now = 9;
        else
            Buffer_Now = 100;
        return;
    }
    else if(!PORTCbits.RC7){
        Buffer_Now = 0;
        return;
    }
    //״̬��
    TRISC = 0xE0;
    PORTCbits.RC4 = 0;
    _delay(1);
    if(!PORTCbits.RC5){
        if(PORTCbits.RC6 && PORTCbits.RC7)
            Buffer_Now = 5;
        else
            Buffer_Now = 100;
        return;
    }
    else if(!PORTCbits.RC6){
        if(PORTCbits.RC7)
            Buffer_Now = 1;
        else
            Buffer_Now = 100;
        return;
    }
    else if(!PORTCbits.RC7){
        Buffer_Now = 2;
        return;
    }
    //״̬��
    TRISC = 0xC0;
    PORTCbits.RC5 = 0;
    _delay(1);
    if(!PORTCbits.RC6){
        if(PORTCbits.RC7)
            Buffer_Now = 3;
        else
            Buffer_Now = 100;
        return;
    }
    else if(!PORTCbits.RC7){
        Buffer_Now = 4;
        return;
    }
    //״̬��
    TRISC = 0x80;
    PORTCbits.RC6 = 0;
    _delay(1);
    if(!PORTCbits.RC7)
        Buffer_Now = 6;
    else
        Buffer_Now = 99;
    return;
}

/**
 * @brief  ȫɨ��������Buffer_Now������Buffer_Now������һ���ж�
 */
void scan_select(void){
    if(Buffer_Now == 99){  //û�а�������
        R_Plus();
        return;
    }
    if(Buffer_Now == 100){  //�������������
        E_Plus();
        return;
    }
    P_Plus();  //��һ������������
    return;        
}

/**
 * @brief  ���������º�Ķ����Ѽ�⴦�����̣��������ͼ
 */
void P_Plus(void) { 
    E_Num = 0;
    if (++P_Num > 250)
        P_Num = 200;
    if (P_Num < 2)
        return;
    if (Buffer_Now != Buffer_Last) {
        if(P_Num < 4)
            Buffer_Last = Buffer_Now;
        return;
    }
    if (R_Num > 9) {
        R_Num = 0;
        return;
    }
    if (R_Num > 4) {
        P_Num = 101;
        R_Num = 0;
        DoubleClick();  //˫��
        return;
    }
    if (P_Num == 100)
        LongClick();  //����
    return;
}

/**
 * @breif  ��⵽û�а���������ʱ�Ĵ������̣��������ͼ
 */
void R_Plus(void) {
    E_Num = 0;
    if (++R_Num > 250)
        R_Num = 200;
    if (R_Num < 2)
        return;
    if (P_Num > 99)
        R_Num = 21;
    P_Num = 0;
    if (R_Num != 20)
        return;
    ShortClick();  //����
    return;
}

/**
 * @brief  ������������µĴ������̣��������ͼ
 */
void E_Plus(void) {
    if (++E_Num < 3)
        return;
    P_Num = 0;
    R_Num = 200;
    Buffer_Last = 10;
    LED[0].Number = 0x0d;  //��ʼ�������
    LED[0].Node = 1;
    LED[1].Number = 0x0e;
    LED[2].Number = 0x0e;
    LED[3].Number = 0x05;
    return;
}

/**
 * @brief  ����������
 */
void ShortClick(void) {
    LED[0].Number = 1;  //�����0��ʾ1����ʾ��⵽����
    LED[1].Number = Buffer_Last;  //�����1�����µİ�����ʾ����
//    Digit[Buffer_Last]++;  //������Ӧ�İ��´�����һ
//    LED[2].Number = Digit[Buffer_Last];  //�����2��ʾ��Ӧ�����İ��´���
    return;
}

/**
 * @brief  ˫��������
 */
void DoubleClick(void) {
    LED[0].Number = 2;  //�����0��ʾ2����ʾ��⵽˫��
    LED[1].Number = Buffer_Last;  //�����1�����µİ�����ʾ����
//    Digit[Buffer_Last]++;  //������Ӧ�İ��´�����һ
//    LED[2].Number = Digit[Buffer_Last];  //�����2��ʾ��Ӧ�����İ��´���
    return;
}

/**
 * @brief  ����������
 */
void LongClick(void) {
    LED[0].Number = 3;  //�����0��ʾ3����ʾ��⵽����
    LED[1].Number = Buffer_Last;  //�����1�����µİ�����ʾ����
//    Digit[Buffer_Last]++;  //������Ӧ�İ��´�����һ
//    LED[2].Number = Digit[Buffer_Last];  //�����2��ʾ��Ӧ�����İ��´���
    return;
}
