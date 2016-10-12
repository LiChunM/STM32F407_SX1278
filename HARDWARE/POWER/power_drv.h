#ifndef _POWER_DRV_H_
#define _POWER_DRV_H_
#include "sys.h"


#define DRV_GPRS_ON	GPIO_SetBits(GPIOC,GPIO_Pin_13)
#define DRV_GPRS_OFF	GPIO_ResetBits(GPIOC,GPIO_Pin_13)

#define DRV_LEDG_ON	GPIO_ResetBits(GPIOB,GPIO_Pin_0)
#define DRV_LEDG_OFF	GPIO_SetBits(GPIOB,GPIO_Pin_0)


#define DRV_12VONE_ON	GPIO_SetBits(GPIOD,GPIO_Pin_14)
#define DRV_12VONE_OFF	GPIO_ResetBits(GPIOD,GPIO_Pin_14)

#define DRV_9VONE_ON		GPIO_SetBits(GPIOD,GPIO_Pin_13)
#define DRV_9VONE_OFF	GPIO_ResetBits(GPIOD,GPIO_Pin_13)

#define DRV_12VTWO_ON	GPIO_SetBits(GPIOD,GPIO_Pin_12)
#define DRV_12VTWO_OFF	GPIO_ResetBits(GPIOD,GPIO_Pin_12)

#define DRV_9VTWO_ON	GPIO_SetBits(GPIOD,GPIO_Pin_11)
#define DRV_9VTWO_OFF	GPIO_ResetBits(GPIOD,GPIO_Pin_11)


#define DRV_SX_ON	GPIO_SetBits(GPIOE,GPIO_Pin_11)
#define DRV_SX_OFF	GPIO_ResetBits(GPIOE,GPIO_Pin_11)


void power_drv_init(void);
void IWDG_Init(u8 prer,u16 rlr);
void IWDG_Feed(void);
#endif

