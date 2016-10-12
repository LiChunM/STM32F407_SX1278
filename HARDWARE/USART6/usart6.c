#include "sys.h"
#include "usart6.h"	
#include "protocol.h"


u16 USART6_RX_STA=0;   	 
u8 USART6_RX_BUF[100];  
u8 USART6_TX_BUF[100];  


   
void USART6_IRQHandler(void)
{
	u8 res;	    
	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
	{	 	
	  	res =USART_ReceiveData(USART6);
		if(USART6_RX_STA<100)
		{
			USART6_RX_BUF[USART6_RX_STA]=res;		
			USART6_RX_STA++;						
		} 
	}  											 
} 


void uart6_init(u32 bound){
  
  	GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); 
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,ENABLE);

       
        GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6); 
        GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); 
        
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOA9��GPIOA10
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        //�ٶ�50MHz
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
        GPIO_Init(GPIOC,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
        USART_InitStructure.USART_BaudRate = bound;//����������
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
        USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;        //�շ�ģʽ
  	USART_Init(USART6, &USART_InitStructure); //��ʼ������1
        
 	USART_Cmd(USART6, ENABLE);  //ʹ�ܴ���1 
        
        USART_ClearFlag(USART6, USART_FLAG_TC);
        
        USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);//��������ж�

      
  	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;//����1�ж�ͨ��
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;                //�����ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                        //IRQͨ��ʹ��
        NVIC_Init(&NVIC_InitStructure);        //����ָ���Ĳ�����ʼ��VIC�Ĵ�����

	USART6_RX_STA=0;			
        
}




void USART6_DATA(u8 *buf,u16 len)
{
	u8 t;
  	for(t=0;t<len;t++)		
	{
	  	while(USART_GetFlagStatus(USART6,USART_FLAG_TC)==RESET); 
    	        USART_SendData(USART6,buf[t]); 
	}	 
	while(USART_GetFlagStatus(USART6,USART_FLAG_TC)==RESET);	
	USART6_RX_STA=0;	  
	
}


void USART6_CMD(unsigned char *lb)
{
    while(*lb)
    {
        USART_SendData(USART6,*lb);
        while(USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET)
        {
        }
        lb ++;
    }
}


void USART6_Receive_Data(u8 *buf,u8 *len)
{
	u8 rxlen=USART6_RX_STA;
	u8 i=0;
	*len=0;				
	delay_ms(10);		
	if(rxlen==USART6_RX_STA&&rxlen)
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=USART6_RX_BUF[i];	
		}		
		*len=USART6_RX_STA;	
		USART6_RX_STA=0;	
	}
}


void USART6_Receive_Data_NoClear(u8 *buf,u8 *len)
{
	u8 rxlen=USART6_RX_STA;
	u8 i=0;
	*len=0;				
	delay_ms(10);		
	if(rxlen==USART6_RX_STA&&rxlen)
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=USART6_RX_BUF[i];	
		}		
		*len=USART6_RX_STA;
	}
}

void Usart6CommandAnalysis(void)
{
	u8 rxlen=0;
	mymemset(USART6_TX_BUF, 0, sizeof(USART6_TX_BUF));
	USART6_Receive_Data(USART6_TX_BUF,&rxlen);
	if(rxlen)
		{
			m35_recive_data(USART6_TX_BUF,rxlen);
		}
}


