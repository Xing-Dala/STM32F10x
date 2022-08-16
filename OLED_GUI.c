#include "STM32F103.h"

//开机自检动画
void Welcome_o(u8 i)
{
    if (i == 0) //开机显示
        OLED_ZFC_Horizontal_center(3, "Staring");
    else if (i == 1) // ESP8266 AT 联网
        OLED_ZFC_Horizontal_center(3, "Staring.");
    else if (i == 2) //各单元初始化
        OLED_ZFC_Horizontal_center(3, "Staring..");
    else if (i == 3) //初始化异常的单元再次初始化1
        OLED_ZFC_Horizontal_center(3, "Staring...");
    else if (i == 4) //初始化异常的单元再次初始化2
        OLED_ZFC_Horizontal_center(3, "Staring....");
    else if (i == 5) //初始化异常的单元再次初始化2
        OLED_ZFC_Horizontal_center(3, "Staring.....");
    else if (i == 6) //事不过三，读取异常
        OLED_ZFC_Horizontal_center(3, "   ERROR!   ");
}

void Wifi_o(void)
{
    OLED_ZFC_Horizontal_center(4,"WIFI");
}