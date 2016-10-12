#include "sys.h"
#include "usart.h"	
#include "rtc.h"
#include "mc323.h"
#include "adc.h"
#include "sxprotocol.h"
#include "power_drv.h"



void FRStopMode_Measure(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC |
						  RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOH |
						  RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	GPIO_Init(GPIOG, &GPIO_InitStructure); 
	GPIO_Init(GPIOF, &GPIO_InitStructure);	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC |
						  RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOH |
						  RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, DISABLE);
	
	PWR_LowRegulatorLowVoltageCmd(ENABLE);
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

}




void Calculate_Wakeup_Time(void)
{
	u32 idall=0;
	idall=systemset.HandInter;
	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,idall);
	DRV_LEDG_OFF;
	DRV_12VONE_OFF;
	DRV_9VONE_OFF;
	DRV_12VTWO_OFF;
	DRV_9VTWO_OFF;
	if(SystemDebug==2)printf("\r\nSystem Hibernates\r\n");
	FRStopMode_Measure();
}

