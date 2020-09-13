/*
 * File:   main.c
 * Author: Eldridge Bear
 *
 * Created on 2020?ê8??14??, ????11:58
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
    char Number;       //显示的数
    char Flash;        //是否闪烁,1闪0不闪
    char Node;         //是否带小数点,1带0不带
    char FlashStatus;  //闪烁状态,1亮0灭
};
typedef struct _LED_SHOW LEDShow; 
typedef unsigned char u8;    //无符号8位数
typedef unsigned short u16;  //无符号16位数

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
                            0xFD,0x61,0xDB,0xF3,0x67,0xB7,0xBF,0xE1,0xFF,0xF7,0xEF,0x3F,0x9D,0x7B,0x9F,0x8F};  //查找表
LEDShow LED[4] = {0};  //保存四个数码管的状态信息
u8 Buffer_Now;   //最新检测到的按键的缓存区，流程图中的缓存区1
u8 Buffer_Last;  //上一次检测到的按键的缓存区，流程图中的缓存区2
u8 E_Num;
u8 R_Num;
u8 P_Num;
//u8 Digit[10];  //保存十个数码管分别被按下的次数

/**
 * @brief 中断入口
 * @note  pic18854单片机只有一个中断入口，所有中断都从这进，再在此函数里面判断不同中断源进入不同的中断服务函数
 */
void __interrupt() isr_routine(void) {
    if (PIR0bits.TMR0IF)  //判断中断源
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
    OSC_init();   //芯片主频初始化函数
    INT_init();   //中断初始化
    GPIO_init();  //GPIO初始化
    TMR_init();   //定时器初始化
    MSSP_init();  //MSSP初始化
    TMP75_init(TMP75_BITS);  //温度传感器TMP75初+始化
    
//    for (; i < 10; i++)
//        Digit[i] = 0;
    E_Num = 0;
    P_Num = 0;
    R_Num = 200;
    Buffer_Last = 10;
    T0CON0bits.T0EN = 1;  //开定时器0
    T1CONbits.ON = 1;  //开启定时器1
    T3CONbits.ON = 1;  //开启定时器3
    LED_init();
    while (1) {
    
    }
    return;
}

/**
 * @brief  芯片主频初始化函数，初始化为最高32M
 * @note   如果主频过低的话会出现按键检测流程过长导致按键不灵敏的现象，将主频开高会使代码执行速度更快
 * @param  None
 * @retval None
 */
void OSC_init(void) {
    OSCCON1 = 0x60;  //使用内部高速振荡器
    OSCENbits.HFOEN = 1;
    OSCFRQ = 0x06;  //32M
}

/**
 * @brief  中断初始化函数，包括开全局中断和开定时器中断
 */
void INT_init(void) {
    INTCON = 0xc0;  //开全局中断
    PIE0 = 0x20;  //开定时器0中断
    PIE4 = 0x05;  //开定时器1中断
    return;
}

/**
 * @brief  数码管显示初始化函数
 * @note   目前直接显示初值SEED，如有需要可在此处添加开机动画
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
 * @brief  GPIO初始化函数
 * @note   PortA是数码管位选信号；PortC第四位是数码管段选信号，高四位用于按键检测
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
 * @brief  定时器初始化函数
 */
void TMR_init(void) {
    T0CON0 = 0x10;  //16位模式
    T0CON1 = 0x43;  //定时器0 Fosc/4时钟源, Psc 1:8
    T1CLK = 0x01;  //定时器1 Fosc/4时钟源
    T1CON = 0x30;  //Psc 1:8
    T3CLK = 0x01;  //定时器3 Fosc/4时钟源
    T3CON = 0x30;  //Psc 1:8
    return;
}

/**
 * @brief 温度传感器芯片TMP75初始化函数
 * @note  芯片地址低三位接地，即地址为 1001 000x(0x9x)，最后一位x值控制读写，1读0写
 * @param Bits: 希望芯片初始化为多少位的精度，有9-12位可选，位数越低速度越快同时精度也越低
 */
void TMP75_init(u8 Bits) {
    if (Bits == 9)
        return;  //芯片上电默认9位模式，直接退出即可
    IIC2_Start();  //启动IIC2
    IIC2_SendByte(0x90);  //选中温度传感器芯片，写模式
    IIC2_SendByte(0x01);  //选择配置寄存器
    switch (Bits) {
        case 10:
            IIC2_SendByte(0x20);  //给配置寄存器送值选择10位模式
            IIC2_SendByte(0x20);  //给配置寄存器送值选择10位模式
            break;
        case 11:
            IIC2_SendByte(0x40);  //给配置寄存器送值选择11位模式
            IIC2_SendByte(0x40);  //给配置寄存器送值选择11位模式
            break;
        case 12:
            IIC2_SendByte(0x60);  //给配置寄存器送值选择12位模式
            IIC2_SendByte(0x60);  //给配置寄存器送值选择12位模式
            break;
    }
    IIC2_Stop();  //结束IIC2
    return;
}

/**
 * @brief  MSSP初始化函数
 * @note   不光要配置MSSP，还要配置PPS输入和输出，这里因为使用默认的PPS输入引脚RB1和RB2所以只配置了PPS输出
 */
void MSSP_init(void) {
    RB1PPS = 0x16;  // SCL2 to RB1
    RB2PPS = 0x17;  // SDA2 to RB2
    SSP2CON1 = 0x28;  //I2C 主模式，时钟 = FOSC/(4 * (SSPxADD + 1)), 主频32M
    SSP2ADD = 0x4f;  //100kHz
}

/**
 * @brief 读取TMP75模块的数据
 * @note  严格按照TMP75芯片上的时序图来做
 * @param Bits: 选择TMP75模块的分辨率
 * @return  返回读到的温度，以浮点数形式返回
 */
float ReadTMP75(u8 Bits) {
    u16 tmp;
    IIC2_Start();
    if (IIC2_SendByte(0x90))  //选中温度传感器芯片，写模式
        return 0;
    if (IIC2_SendByte(0x00))  //选择温度寄存器
        return 0;
    IIC2_Start();
    if (IIC2_SendByte(0x91))  //读模式
        return 0;
    tmp = (u16)IIC2_RecvByte();  //读取高八位
    switch (Bits) {
        case 9:
            tmp = (tmp << 1) + (IIC2_RecvByte() >> 7);  //读取低八位
            IIC2_Stop();
            return tmp * 128.0f / 256;
        case 10:
            tmp = (tmp << 2) + (IIC2_RecvByte() >> 6);  //读取低八位
            IIC2_Stop();
            return tmp * 128.0f / 512;
        case 11:
            tmp = (tmp << 3) + (IIC2_RecvByte() >> 5);  //读取低八位
            IIC2_Stop();
            return tmp * 128.0f / 1024;
        case 12:
            tmp = (tmp << 4) + (IIC2_RecvByte() >> 4);  //读取低八位
            IIC2_Stop();
            return tmp * 128.0f / 2048;
    }
    return 0;
}

/**
 * @brief IIC2启动函数
 */
void IIC2_Start(void) {
    PIR3bits.SSP2IF = 0;
    SSP2CON2bits.SEN = 1;  //产生启动条件
    while (!PIR3bits.SSP2IF) {}  //等待启动完成，启动完成后标志位置一
    PIR3bits.SSP2IF = 0;  //清零标志位
}

/**
 * @brief IIC2停止函数
 */
void IIC2_Stop(void) {
    SSP2CON2bits.PEN = 1;
    while (!PIR3bits.SSP2IF) {}  //等待启动完成，启动完成后标志位置一
    PIR3bits.SSP2IF = 0;  //清零标志位
    return;
}

/**
 * @brief IIC2发送一个字节的数据
 * @param Byte: 待发送饿数据
 * @return 发送结果，0正常1未收到ack
 */
u8 IIC2_SendByte(u8 Byte) {
    SSP2BUF = Byte;  //将需要发送的数据送入缓存区启动发送
    while (SSP2STATbits.BF) {}  //等待发送完成
    while (!PIR3bits.SSP2IF) {}  //等待发送完成，发送完成后标志位置一
    PIR3bits.SSP2IF = 0;  //清零标志位
    if (SSP2CON2bits.ACKSTAT) {
        LED[3].Number = 0;	
        return 1;
    }  //确认收到应答信号
    return 0;
}

/**
 * @brief IIC2接收一个字节的数据
 * @return 返回接收到的数据
 */
u8 IIC2_RecvByte(void) {
    u8 temp;
    SSP2CON2bits.RCEN = 1;  //使能接收模式
    while (!PIR3bits.SSP2IF) {}  //等待接收完成，发送完成后标志位置一
    PIR3bits.SSP2IF = 0;  //清零标志位
    temp  = SSP2BUF;  //返回读取到的数据，cpu读取buf时BF标志自动清零
    SSP2CON2bits.ACKDT = 0;  //应答
    SSP2CON2bits.ACKEN = 1;  //发送应答
    while (!PIR3bits.SSP2IF) {}  //等待发送应答完成，发送完成后标志位置一
    PIR3bits.SSP2IF = 0;  //清零标志位
    return temp;  
}

/**
 * @brief  定时器0中断服务函数，在此函数内将LED[4]数组中显示信息读取出来显示
 * @param  None
 * @retval None
 */
void TMR0_INT(void) {
    PIR0bits.TMR0IF = 0;
    TMR_Clear(0, TMR0H_DATA, TMR0L_DATA);
    static u8 Mode = 0;
    switch (++Mode) {  //判断是轮到第几个数码管显示，从0-4循环
        case 1: Show(0); break;
        case 2: Show(1); break;
        case 3: Show(2); break;
        case 4: Show(3); Mode = 0; break;
    }
    return;
}

/**
 * @brief  定时器1中断服务函数，在此函数内进行按键全扫面检测
 * @param  None
 * @retval None
 */
void TMR1_INT(void) {
    PIR4bits.TMR1IF = 0;
    TMR_Clear(1, TMR1H_DATA, TMR1L_DATA);
    scan();  //全扫描
    scan_select();  //根据全扫描结果进行行为判断
    return;
}

/**
 * @brief 定时器3中断函数，定时25ms，软件计数cnt10次，相当于250ms
 */
void TMR3_INT(void) {
    static u8 cnt = 10;
    float tmp = 0;
    PIR4bits.TMR3IF = 0;
    TMR_Clear(3, TMR3H_DATA, TMR3L_DATA);
    if (!--cnt) {
        tmp = ReadTMP75(TMP75_BITS);  //读取温度
        cnt = 10;
        LED[2].Number = ((u8)tmp)/10;  //显示十位
        LED[1].Number = ((u8)tmp)%10;  //显示个位
    }
    return;
}

/**
 * @brief 定时器送初值函数
 * @param TMR: 需要送初值的定时器
 * @param High: 送初值高八位
 * @param Low: 送初值低八位
 * @retval None
 */
void TMR_Clear(u8 TMR, u8 High, u8 Low) {
    switch(TMR) {  //选择定时器
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
 * @brief 数码管显示函数，从LED[4]数组中依次取出数据进行相应判断并显示出来
 * @param n: 显示的是第几个数码管，从右到左依次0-3
 * @retval None
 */
void Show(u8 n) {
    static u8 Twinkle[4] = {0x0f, 0x0f, 0x0f, 0x0f};
    u8 Num;
    Num = LED[n].Number;
    if (LED[n].Flash) {  //判断该位数码管是否需要闪烁
        if (--Twinkle[n] == 0) {
            LED[n].FlashStatus ^= 1;  //将亮灭标记取反
            Twinkle[n] = 0x0f;
        }
        LATA = 0x00;  //在闪烁状态是LATA先清零，防止影响旁边的数码管
        if (!LED[n].FlashStatus) //如果是灭的标记则直接返回
            return;
    }
    if(LED[n].Node)  //判断该位数码管是否需要加上小数点
        Num += 16;   //需要加上小数点则往表后移16位
//    if (Num > 31)  //判断数字是否超出表的范围
//        Num %= 32;
    Num = num_table[Num];  //从表中取出解码数据
    switch (n) {  //给相应数码管送段选和位选数据，为确保段选和位选数据尽量贴近，故将位选数据也放在case里
        case 0: LATA = Num; LATC = 0x0e; break;
        case 1: LATA = Num; LATC = 0x0d; break;
        case 2: LATA = Num; LATC = 0x0b; break;
        case 3: LATA = Num; LATC = 0x07; break;
    }
    return;
}

/**
 * @brief  全扫描函数
 * @note   具有优先级，7 8 9 10优先级最高，1 2 5第二，3 4其次，6最低
 *          具体判断过程详见流程图
 */
void scan(void) {
    PORTC |= 0xf0;
    TRISC = 0xf0;
    //状态一
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
    //状态二
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
    //状态三
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
    //状态四
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
 * @brief  全扫描结果存入Buffer_Now，根据Buffer_Now进行下一步判断
 */
void scan_select(void){
    if(Buffer_Now == 99){  //没有按键按下
        R_Plus();
        return;
    }
    if(Buffer_Now == 100){  //多个按键被按下
        E_Plus();
        return;
    }
    P_Plus();  //有一个按键被按下
    return;        
}

/**
 * @brief  按键被按下后的动作难检测处理流程，详见流程图
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
        DoubleClick();  //双击
        return;
    }
    if (P_Num == 100)
        LongClick();  //长按
    return;
}

/**
 * @breif  检测到没有按键被按下时的处理流程，详见流程图
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
    ShortClick();  //单击
    return;
}

/**
 * @brief  多个按键被按下的处理流程，详见流程图
 */
void E_Plus(void) {
    if (++E_Num < 3)
        return;
    P_Num = 0;
    R_Num = 200;
    Buffer_Last = 10;
    LED[0].Number = 0x0d;  //初始化数码管
    LED[0].Node = 1;
    LED[1].Number = 0x0e;
    LED[2].Number = 0x0e;
    LED[3].Number = 0x05;
    return;
}

/**
 * @brief  单击处理函数
 */
void ShortClick(void) {
    LED[0].Number = 1;  //数码管0显示1，表示检测到单击
    LED[1].Number = Buffer_Last;  //数码管1将按下的按键显示出来
//    Digit[Buffer_Last]++;  //按键对应的按下次数加一
//    LED[2].Number = Digit[Buffer_Last];  //数码管2显示对应按键的按下次数
    return;
}

/**
 * @brief  双击处理函数
 */
void DoubleClick(void) {
    LED[0].Number = 2;  //数码管0显示2，表示检测到双击
    LED[1].Number = Buffer_Last;  //数码管1将按下的按键显示出来
//    Digit[Buffer_Last]++;  //按键对应的按下次数加一
//    LED[2].Number = Digit[Buffer_Last];  //数码管2显示对应按键的按下次数
    return;
}

/**
 * @brief  长按处理函数
 */
void LongClick(void) {
    LED[0].Number = 3;  //数码管0显示3，表示检测到长按
    LED[1].Number = Buffer_Last;  //数码管1将按下的按键显示出来
//    Digit[Buffer_Last]++;  //按键对应的按下次数加一
//    LED[2].Number = Digit[Buffer_Last];  //数码管2显示对应按键的按下次数
    return;
}
