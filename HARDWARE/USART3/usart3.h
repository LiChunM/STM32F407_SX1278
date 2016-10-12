#ifndef __USART3_H
#define __USART3_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 


extern u8 RS485ONE_RX_BUF[100]; 	
extern vu16 USART3_RX_STA;   	
extern u8 testbuf[10];
#define RS485ONE_TX_EN		PBout(15)

void usart3_init(u32 bound);
void RS485ONE_Send_Data(u8 *buf,u8 len);
void RS485ONE_Receive_Data(u8 *buf,u8 *len);
void rs485Init(u32 bound);
#endif

