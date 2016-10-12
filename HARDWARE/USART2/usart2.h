#ifndef __USART2_H
#define __USART2_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

#define EN_USART2_RX 	1			//0,不接收;1,接收.

extern u8 USART2_RX_BUF[100];
extern u8 USART2_TX_BUF[100];
extern u8 USART2_RX_CNT;
void uart2_init(u32 bound);
void USART2_DATA(u8 *buf,u16 len);
void USART2_CMD(unsigned char *lb);
void Usart2CommandAnalysis(void);
#endif

