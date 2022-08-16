#include "STM32F103.h"

//延时函数初始化
void Delay_Init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); //选择外部时钟
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		  //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
}

//微秒延时
void Delay_us(u16 n)
{
	u32 temp;
	SysTick->LOAD = n * 9;					  //时间加载
	SysTick->VAL = 0x00;					  //清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //开始倒数
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (1 << 16))); //等待时间到达
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;		//关闭计数器
	SysTick->VAL = 0X00;							//清空计数器
}

//毫秒延时
void Delay_ms(u16 n)
{
	u32 temp;
	SysTick->LOAD = (u32)n * 9000;			  //时间加载
	SysTick->VAL = 0x00;					  //清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //开始倒数
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (1 << 16))); //等待时间到达
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;		//关闭计数器
	SysTick->VAL = 0X00;							//清空计数器
}

//秒延时
void Delay_s(u16 n)
{
	for (u16 x = 0; x < n; x++)
	{
		u32 temp;
		SysTick->LOAD = 9000000;				  //时间加载
		SysTick->VAL = 0x00;					  //清空计数器
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //开始倒数
		do
		{
			temp = SysTick->CTRL;
		} while ((temp & 0x01) && !(temp & (1 << 16))); //等待时间到达
		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;		//关闭计数器
		SysTick->VAL = 0X00;							//清空计数器
	}
}