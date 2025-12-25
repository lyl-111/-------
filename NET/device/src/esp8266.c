/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名: 	esp8266.c
	*
	*	作者: 		赵小明
	*
	*	日期: 		2017-05-08
	*
	*	版本号: 		V1.0
	*
	*	说明: 		ESP8266文件说明
	*
	*	修改记录:
	************************************************************
	************************************************************
	************************************************************
**/

//芯片头文件
#include "stm32f10x.h"

//网络设备驱动
#include "esp8266.h"

//硬件驱动
#include "Delay.h"
#include "usart.h"

//C库
#include <string.h>
#include <stdio.h>


#define ESP8266_WIFI_INFO		"AT+CWJAP=\"lyl\",\"12345678\"\r\n"


unsigned char esp8266_buf[512];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;


//==========================================================
//	函数名称:	ESP8266_Clear
//
//	函数功能:	清空缓存
//
//	入口参数:	无
//
//	返回值:	无
//
//	说明:
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	函数名称:	ESP8266_WaitRecive
//
//	函数功能:	等待接收
//
//	入口参数:	无
//
//	返回值:	REV_OK-接收成功		REV_WAIT-接收超时未完成
//
//	说明:		循环检测是否接收到数据
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//如果接收计数为0 说明没有正在接收数据，直接返回等待
		return REV_WAIT;

	if(esp8266_cnt == esp8266_cntPre)				//如果上一秒的值相同，说明接收完成
	{
		esp8266_cnt = 0;							//清零计数

		return REV_OK;								//返回接收完成标志
	}

	esp8266_cntPre = esp8266_cnt;					//置为相同

	return REV_WAIT;								//返回接收未完成标志

}

//==========================================================
//	函数名称:	ESP8266_SendCmd
//
//	函数功能:	发送命令
//
//	入口参数:	cmd指令字符串
//				res需要获取的返回值
//
//	返回值:	0-成功	1-失败
//
//	说明:
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{

	unsigned char timeOut = 200;

	Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));

	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键字
			{
				ESP8266_Clear();									//清空缓存

				return 0;
			}
		}

		Delay_ms(10);
	}

	return 1;

}

//==========================================================
//	函数名称:	ESP8266_SendData
//
//	函数功能:	发送数据
//
//	入口参数:	data数据指针
//				len数据长度
//
//	返回值:	无
//
//	说明:
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];

	ESP8266_Clear();								//清空缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到>时可以发送数据
	{
		Usart_SendString(USART2, data, len);		//向设备发送数据
	}

}

//==========================================================
//	函数名称:	ESP8266_GetIPD
//
//	函数功能:	获取平台返回的数据
//
//	入口参数:	等待超时(单位10ms)
//
//	返回值:	平台返回的原始数据
//
//	说明:		不同平台的设备返回的格式不同，需要去解析
//				ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy代表数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;

	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收到数据
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//搜索IPD开头
			if(ptrIPD == NULL)											//如果没有找到IPD头，再等一会儿，如果超出设置的超时时间
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;

			}
		}

		Delay_ms(5);													//延时等待
	} while(timeOut--);

	return NULL;														//超时未找到，返回空指针

}

//==========================================================
//	函数名称:	ESP8266_Init
//
//	函数功能:	初始化ESP8266
//
//	入口参数:	无
//
//	返回值:	无
//
//	说明:
//==========================================================
void ESP8266_Init(void)
{

	ESP8266_Clear();

//	UsartPrintf(USART_DEBUG, "1. AT\r\n");
	OLED_Clear(); OLED_ShowString(0,0,"1.AT...",8);
	while(ESP8266_SendCmd("AT\r\n", "OK"))
		Delay_ms(500);

//	UsartPrintf(USART_DEBUG, "2. CWMODE\r\n");
	OLED_ShowString(0,2,"2.CWMODE...",8);
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		Delay_ms(500);

//	UsartPrintf(USART_DEBUG, "3. AT+CWDHCP\r\n");
	OLED_ShowString(0,4,"3.AT+CWDHCP...",8);
	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		Delay_ms(500);

//	UsartPrintf(USART_DEBUG, "4. CWJAP\r\n");
	OLED_ShowString(0,6,"4.CWJAP...",8);
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
		Delay_ms(500);

	UsartPrintf(USART_DEBUG, "5. ESP8266 Init OK\r\n");
	OLED_Clear(); OLED_ShowString(0,0,"ESP8266 Init OK",16); Delay_ms(500);

}

//==========================================================
//	函数名称:	USART2_IRQHandler
//
//	函数功能:	串口2接收中断
//
//	入口参数:	无
//
//	返回值:	无
//
//	说明:
//==========================================================
void USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止数组溢出
		esp8266_buf[esp8266_cnt++] = USART2->DR;

		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

}
