#ifndef __WATER_H
#define	__WATER_H
#include "stm32f10x.h"
#include "adcx.h"
#include "Delay.h"
#include "math.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	TDS电导率传感器h文件                   
 * 版本			: V1.0
 * 日期			: 2025.2.2
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码						
 * IP账号		:	辰哥单片机设计（同BILIBILI|抖音|快手|小红书|CSDN|公众号|视频号等）
 * 作者			:	辰哥 
 * 工作室		: 异方辰电子工作室
 * 讲解视频	:	https://www.bilibili.com/video/BV1JZPXeUEvY/?share_source=copy_web
 * 官方网站	:	www.yfcdz.cn

**********************BEGIN***********************/

#define TDS_READ_TIMES	10  //TDS传感器ADC循环读取次数

/***************根据自己需求更改****************/
// TDS GPIO宏定义

#define		TDS_GPIO_CLK								RCC_APB2Periph_GPIOA
#define 	TDS_GPIO_PORT								GPIOA
#define 	TDS_GPIO_PIN								GPIO_Pin_0
#define   ADC_CHANNEL									ADC_Channel_0	// ADC 通道宏定义


/*********************END**********************/


void TDS_Init(void);
uint16_t TDS_GetData(void);
float TDS_GetData_PPM(void);

// 氮磷钾计算结构体
typedef struct {
    float nitrogen;    // 氮含量 (mg/L)
    float phosphorus;  // 磷含量 (mg/L) 
    float potassium;   // 钾含量 (mg/L)
} NPK_Values;

// 氮磷钾计算函数
NPK_Values TDS_Calculate_NPK(float tds_value);

#endif /* __TDS_H */
