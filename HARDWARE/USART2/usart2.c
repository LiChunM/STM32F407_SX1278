#include "sys.h"
#include "usart2.h"	


#if EN_USART2_RX   		
u8 USART2_RX_BUF[100];
u8 USART2_TX_BUF[100];
u8 USART2_RX_CNT=0;   


void USART2_IRQHandler(void)
{
	u8 res;	    
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{	 	
	 	res =USART_ReceiveData(USART2);
		if(USART2_RX_CNT<100)
		{
			USART2_RX_BUF[USART2_RX_CNT]=res;		
			USART2_RX_CNT++;					
		} 
	}  											 
} 

#endif										 


void uart2_init(u32 bound)
{  	 
  	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_USART2); 
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource6,GPIO_AF_USART2); 
	
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIOD,&GPIO_InitStructure); 
		

	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	
 	 USART_Cmd(USART2, ENABLE);  //使能串口 2
	
	USART_ClearFlag(USART2, USART_FLAG_TC);
	
#if EN_USART2_RX	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启接受中断

 	 NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif	
}


void USART2_DATA(u8 *buf,u16 len)
{
	u8 t;
  	for(t=0;t<len;t++)		
	{
	  	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); 
    	        USART_SendData(USART2,buf[t]); 
	}	 
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);	
	USART2_RX_CNT=0;	  
	
}


void USART2_CMD(unsigned char *lb)
{
    while(*lb)
    {
        USART_SendData(USART2,*lb);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
        {
        }
        lb ++;
    }
}


void USART2_Receive_Data(u8 *buf,u8 *len)
{
	u8 rxlen=USART2_RX_CNT;
	u8 i=0;
	*len=0;				
	delay_ms(10);		
	if(rxlen==USART2_RX_CNT&&rxlen)
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=USART2_RX_BUF[i];	
		}		
		*len=USART2_RX_CNT;	
		USART2_RX_CNT=0;	
	}
}


void USART2_Receive_Data_NoClear(u8 *buf,u8 *len)
{
	u8 rxlen=USART2_RX_CNT;
	u8 i=0;
	*len=0;				
	delay_ms(10);		
	if(rxlen==USART2_RX_CNT&&rxlen)
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=USART2_RX_BUF[i];	
		}		
		*len=USART2_RX_CNT;
	}
}

void Usart2CommandAnalysis(void)
{
	u8 rxlen=0;
	mymemset(USART2_TX_BUF, 0, sizeof(USART2_TX_BUF));
	USART2_Receive_Data(USART2_TX_BUF,&rxlen);
	
}


