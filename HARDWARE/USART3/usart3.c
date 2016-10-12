#include "sys.h"
#include "usart3.h"	
#include "protocol.h"


vu16 USART3_RX_STA=0;   	 
u8 RS485ONE_RX_BUF[100];  
   

u8 testbuf[10]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};

void USART3_IRQHandler(void)
{
	u8 res;	    
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{	 	
	 	res =USART_ReceiveData(USART3);
		if(USART3_RX_STA<100)
		{
			RS485ONE_RX_BUF[USART3_RX_STA]=res;		
			USART3_RX_STA++;						
		} 
	}  											 
} 


void rs485one_dir_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);

}

void usart3_init(u32 bound)
{  	
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 
	USART_DeInit(USART3);  
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART3时钟
	
 
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; //GPIOB11和GPIOB10初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); //初始化GPIOB11，和GPIOB10
	
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3); //GPIOB11复用为USART3
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_USART3); //GPIOB10复用为USART3	  
	
	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART3, &USART_InitStructure); //初始化串口3
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断  
		
	USART_Cmd(USART3, ENABLE);                    //使能串口 

	USART_ClearFlag(USART3, USART_FLAG_TC);
 
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	USART3_RX_STA=0;				//清零 

	RS485ONE_TX_EN=0;
}



void rs485Init(u32 bound)
{
	rs485one_dir_init();
	usart3_init(bound);
}


void RS485ONE_Send_Data(u8 *buf,u8 len)
{
	u8 t;
	RS485ONE_TX_EN=1;			
  	for(t=0;t<len;t++)		
	{
	  while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);
   	  USART_SendData(USART3,buf[t]); 
	}	 
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); 	
	USART3_RX_STA=0;	  
	RS485ONE_TX_EN=0;					
}

void RS485ONE_Receive_Data(u8 *buf,u8 *len)
{
	u16 rxlen=USART3_RX_STA;
	u16 i=0;
	*len=0;
	delay_ms(10);		
	if(rxlen==USART3_RX_STA&&rxlen)
	{
		
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485ONE_RX_BUF[i];	
		}		
		*len=USART3_RX_STA;
		
		USART3_RX_STA=0;		
	}
}


