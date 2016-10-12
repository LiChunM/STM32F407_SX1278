
#ifndef __SPI_H__
#define __SPI_H__

#include "sys.h" 


#define 	MISO	GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2)
#define 	M0SI0	GPIO_ResetBits(GPIOC , GPIO_Pin_3)
#define 	M0SI1	GPIO_SetBits(GPIOC, GPIO_Pin_3)
#define 	SCK0	GPIO_ResetBits(GPIOB, GPIO_Pin_10)
#define 	SCK1	GPIO_SetBits(GPIOB, GPIO_Pin_10)




void SpiInit( void );
uint8_t SpiInOut( uint8_t outData );
void SPI1_Init(void);
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler);
u8 SPI1_ReadWriteByte(u8 TxData);
#endif 
