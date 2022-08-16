#include "STM32F103.h"
#include "OLED_Library.h"

// OLED屏幕ISP接口初始化
void OLED_IO_Init(void)
{
    //初始化结构体
    GPIO_InitTypeDef GPIO_InitStructure; //初始化GPIO结构体
    SPI_InitTypeDef SPI_InitStructure;   //初始化SPI结构体

    //使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE); //使用复用时钟复用GPIOA指示引脚SPI1功能

    // GPIO结构体
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; //选择引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;        //复用输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //初始化PA5(SCL),PA7(SDA)

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6; //选择引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //初始化PA4(RST),PA6(DC)

    GPIO_SetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_7); // PA5 and PA7上拉

    // SPI结构体
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;           //设置SPI单线只发送
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       //主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                   // SPI 发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                          //串行同步时钟的空闲状态为低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                        //第1个跳变沿数据被采样
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                           // NSS主机片选信号(CS)由软件控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; //预分频 16

    // SPI 速度设置函数（调整传输速度快慢 只有4个分频可选）
    // SPI_BaudRatePrescaler_2 2 分频 (SPI 36M@sys 72M)
    // SPI_BaudRatePrescaler_8 8 分频 (SPI 9M@sys 72M)
    // SPI_BaudRatePrescaler_16 16 分频 (SPI 4.5M@sys 72M)
    // SPI_BaudRatePrescaler_256 256 分频 (SPI 281.25K@sys 72M)

    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //数据传输从 MSB 高位开始 低位为LSB
    SPI_InitStructure.SPI_CRCPolynomial = 7;           // CRC 值计算的多项式
    SPI_Init(SPI1, &SPI_InitStructure);                //根据指定的参数初始化外设 SPIx 寄存器
    SPI1->CR2 = 1 << 1;                                //允许DMA往缓冲区内发送

    SPI_Cmd(SPI1, ENABLE); //使能 SPI 外设
};

uint8_t OLED_SRAM[8][128]; //图像储存在SRAM里

// DMA初始化
void OLED_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //使能 DMA 时钟
    DMA_DeInit(DMA1_Channel3);                         //选择DMA通道

    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI1->DR;              // DMA 外设 ADC 基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)OLED_SRAM;                  // DMA 内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      //从储存器读取发送到外设
    DMA_InitStructure.DMA_BufferSize = 1024;                                // DMA 通道的 DMA 缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //外设地址不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //内存地址递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 8 位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // 8 位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         //工作在循环传输模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   // DMA 通道 x 拥有中优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            //非内存到内存传输
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);                            //根据指定的参数初始化

    DMA_Cmd(DMA1_Channel3, ENABLE); //使能DMA1 CH3所指示的通道
}

void OLED_SendCmd(u8 TxData) //发送命令
{
    OLED_DC_CMD(); //命令模式

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
    {
        for (u8 retry = 0; retry < 200; retry++) //等待回复
            ;
        return; //返回
    }

    Delay_ms(100); //延时100毫秒

    SPI_I2S_SendData(SPI1, TxData); //通过外设 SPIx 发送一个数据

    OLED_DC_DAT(); //数据模式
}

// OLED初始化函数
void OLED_Init(void)
{
    OLED_IO_Init(); //端口初始化

    Delay_s(1); //延时1秒稳定端口状态

    OLED_RST_OFF(); // OLED复位
    Delay_ms(10);   //复位延时
    OLED_RST_ON();  //结束复位

    OLED_SendCmd(0xae); //关闭显示

    OLED_SendCmd(0xd5); //设置时钟分频因子,震荡频率
    OLED_SendCmd(0x80); //[3:0],分频因子;[7:4],震荡频率

    OLED_SendCmd(0x81); //设置对比度
    OLED_SendCmd(0x7f); // 128

    OLED_SendCmd(0x8d); //设置电荷泵开关
    OLED_SendCmd(0x14); //开

    OLED_SendCmd(0x20); //设置模式
    OLED_SendCmd(0x00); //设置为水平地址模式

    OLED_SendCmd(0x21); //设置列地址的起始和结束的位置
    OLED_SendCmd(0x00); // 0
    OLED_SendCmd(0x7f); // 127

    OLED_SendCmd(0x22); //设置页地址的起始和结束的位置
    OLED_SendCmd(0x00); // 0
    OLED_SendCmd(0x07); // 7

    OLED_SendCmd(0xc8); // 0xc9上下反置 0xc8正常
    OLED_SendCmd(0xa1); // 0xa0左右反置 0xa1正常

    OLED_SendCmd(0xa4); //全局显示开启;0xa4正常,0xa5无视命令点亮全屏
    OLED_SendCmd(0xa6); //设置显示方式;bit0:1,反相显示;0,正常显示

    OLED_SendCmd(0xaf); //开启显示
    OLED_SendCmd(0x56);

    OLED_DMA_Init(); // DMA初始化
}

//指定位置显示单字符，X+Y+单字符
void OLED_Write(u8 x, u8 y, u8 *ascii)
{
    u8 i = 0, c = *ascii;

    for (i = 0; i < 6; i++)
        OLED_SRAM[y][x + i] = YIN_F6X8[(c - 32) * 6 + 1 + i];
}

//清屏--全灭
void OLED_Clear(void)
{
    for (u8 y = 0; y < 7; y++)
        for (u8 x = 0; x < 126; x += 6)
            OLED_ZFC(x, y, " ");
}

char OLED_zfc[] = {0}; //字符转化为字符串储存于此数组

// OLED显示多个字符，x+y+字符串，一行21个字符
void OLED_ZFC(u8 x, u8 y, u8 *chr)
{
    u8 j = 0;

    while (chr[j] != '\0')
    {
        u8 c = chr[j];

        for (u8 i = 0; i < 6; i++)
            OLED_SRAM[y][x + i] = YIN_F6X8[(c - 32) * 6 + 1 + i];

        x += 6;

        if (x > 120) //自动换行
        {
            x = 0;
            y++;
        }

        j++;
    }
}

// OLED显示字符串水平居中，y+字符串，一行21个字符
void OLED_ZFC_Horizontal_center(u8 y, u8 *chr)
{
    u8 num = 0, j = 0;

    //读取字符个数
    while (chr[num] != '\0')
        num++;

    if (num < 22)
    {
        num = ((21 - num) * 6) / 2; //计算字符居中时最佳的显示起点

        while (chr[j] != '\0')
        {
            u8 c = chr[j];

            for (u8 i = 0; i < 6; i++)
                OLED_SRAM[y][num + i] = YIN_F6X8[(c - 32) * 6 + 1 + i];

            num += 6;

            if (num > 120) //自动换行
            {
                num = 0;
                y++;
            }

            j++; //显示下一个字符
        }
    }
    else
        OLED_ZFC_Horizontal_center(y, "ERROR");
}

// OLED显示字符串垂直居中，x+字符串，一行21个字符
void OLED_ZFC_Vorizontal_center(u8 x, u8 *chr)
{
    u8 num = 0, j = 0;

    while (chr[num] != '\0') //读取字符个数
        num++;

    if (num < 44)
        num = 3;
    else if (num < 88)
        num = 2;
    else if (num < 176)
        num = 1;
    else
        num = 0;

    while (chr[j] != '\0')
    {
        u8 c = chr[j];

        for (u8 i = 0; i < 6; i++)
            OLED_SRAM[num][x + i] = YIN_F6X8[(c - 32) * 6 + 1 + i];

        x += 6;

        if (x > 120) //自动换行
        {
            x = 0;
            num++;
        }

        j++;
    }
}