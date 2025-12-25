#include "tds.h"

/***********************************
											
	TDS电导率传感器c文件                   

**********************BEGIN***********************/

void TDS_Init(void)
{
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (TDS_GPIO_CLK, ENABLE );	// 打开 ADC IO端口时钟
		GPIO_InitStructure.GPIO_Pin = TDS_GPIO_PIN;					// 配置 ADC IO 引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		// 设置为模拟输入
		
		GPIO_Init(TDS_GPIO_PORT, &GPIO_InitStructure);				// 初始化 ADC IO

		ADCx_Init();
	}

}


uint16_t TDS_ADC_Read(void)
{
	//设置指定ADC的规则组通道，采样时间
	return ADC_GetValue(ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}


uint16_t TDS_GetData(void)
{
	

	uint32_t  tempData = 0;
	
	for (uint8_t i = 0; i < TDS_READ_TIMES; i++)
	{
		tempData += TDS_ADC_Read();
		Delay_ms(5);
	}

	tempData /= TDS_READ_TIMES;
	return tempData;
	
}

float TDS_GetData_PPM(void)
{
	float  tempData = 0;
	float TDS_DAT;

	for (uint8_t i = 0; i < TDS_READ_TIMES; i++)
	{
		tempData += TDS_ADC_Read();
		Delay_ms(5);
	}
	tempData /= TDS_READ_TIMES;
	
	TDS_DAT = (tempData/4095.0)*3.3;
	TDS_DAT = 66.71*TDS_DAT*TDS_DAT*TDS_DAT-127.93*TDS_DAT*TDS_DAT+428.7*TDS_DAT;
	if(TDS_DAT<20) TDS_DAT = 0;
	
	return TDS_DAT;

}

// 根据TDS值计算氮磷钾含量
// 针对土壤溶液优化的经验公式
// 注意：这是估算值，实际应用中需要根据具体土壤类型进行校准
NPK_Values TDS_Calculate_NPK(float tds_value)
{
    NPK_Values npk;
    
    // 土壤溶液中的营养比例特征
    // 土壤溶液与水培营养液不同，含有更多其他离子和有机物
    // 一般土壤溶液中N、P、K在有效养分中的占比较低：
    // 氮(N): 8-15% (主要为硝态氮和铵态氮)
    // 磷(P): 3-8% (主要为磷酸盐，易被土壤固定)
    // 钾(K): 5-12% (钾离子相对活跃)
    const float nitrogen_ratio = 0.12f;    // 土壤溶液中氮的比例调整为12%
    const float phosphorus_ratio = 0.05f;  // 土壤溶液中磷的比例调整为5%
    const float potassium_ratio = 0.08f;   // 土壤溶液中钾的比例调整为8%
    
    // 土壤溶液校准系数
    // 考虑因素：
    // 1. 土壤溶液中大量其他离子(Ca²⁺, Mg²⁺, Na⁺等)干扰
    // 2. 有机质和胶体物质影响电导率
    // 3. 土壤颗粒吸附和解吸过程的动态平衡
    // 4. 土壤类型差异(沙土、壤土、粘土)
    const float calibration_factor = 0.6f;  // 土壤溶液校准系数调整为0.6
    
    // 计算各成分含量
    npk.nitrogen = tds_value * nitrogen_ratio * calibration_factor;
    npk.phosphorus = tds_value * phosphorus_ratio * calibration_factor;
    npk.potassium = tds_value * potassium_ratio * calibration_factor;
    
    // 限制最小值，避免负值
    if(npk.nitrogen < 0) npk.nitrogen = 0;
    if(npk.phosphorus < 0) npk.phosphorus = 0;
    if(npk.potassium < 0) npk.potassium = 0;
    
    return npk;
}
