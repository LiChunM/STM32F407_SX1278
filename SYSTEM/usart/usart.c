#include "sys.h"
#include "usart.h"	
#include "rtc.h"
#include "mc323.h"
#include "adc.h"
#include "sxprotocol.h"
#include "power_drv.h"
#include "sxdowndata.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	


volatile u32 jishih=0;

vu8 TestBit=0;


//��ʼ��IO ����1 
//bound:������
void uart_init(u32 bound){
   //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
  USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�

	//Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

#endif
	
}


void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		if(TestBit&&Res!='$')
			{
				if(USART_RX_STA<100)
				{
					USART_RX_BUF[USART_RX_STA]=Res;		
					USART_RX_STA++;						
				} 
			}
		else
			{
				TestBit=0;
				if((USART_RX_STA&0x8000)==0)//����δ���
				{
					if(USART_RX_STA&0x4000)//���յ���0x0d
					{
						if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
						else USART_RX_STA|=0x8000;	//��������� 
					}
					else //��û�յ�0X0D
					{	
						if(Res==0x0d)USART_RX_STA|=0x4000;
						else
						{
							USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
							USART_RX_STA++;
							if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
						}		 
					}
				}  
			}
  } 
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();  											 
#endif
} 
#endif	

 

void Usart1CommandAnalysis(void)
{
	if(TestBit)
		{
			u16 rxlen=USART_RX_STA;
			u16 i=0;
			delay_ms(10);		
			if(rxlen==USART_RX_STA&&rxlen)
			{
				RS485ONE_Send_Data(USART_RX_BUF,rxlen);
				printf("RS485_Send_Data\r\n");
				for(i=0;i<rxlen;i++)printf("%02X ",USART_RX_BUF[i]);
				printf("\r\n");
				USART_RX_STA=0;		
			}
		}
	else
		{
			u8 res=0,t=0;
			u8 USART_TX_BUF[30]={0};
			if(USART_RX_STA&0x8000)	
			{
				res=USART_RX_STA&0x3FFF;
				if(res<=30)
					{
							for(t=0;t<res;t++)
							{
								USART_TX_BUF[t]=USART_RX_BUF[t];
								while((USART1->SR&0X40)==0);
							}
					}
			      USART_RX_STA=0;
			      USART_TX_BUF[t]=0;
			     User_Command_Analysis((u8*)USART_TX_BUF);					   
			}
		}
}


void Get_Str_Use(u8 *ipstr,u8 *buf)
{
	mymemset(ipstr,'\0',sizeof(ipstr));
	while(*buf!=0x20)buf++;
	buf++;
	strcpy((char*)ipstr,(char*)buf);
}

u8 * mid(u8 *dst,u8 *src, int n,int m) /*nΪ���ȣ�mΪλ��*/
{
    u8 *p = src;
    u8 *q = dst;
    int len = strlen((const char*)src);
    if(n>len) n = len-m;    /*�ӵ�m�������*/
    if(m<0) m=0;    /*�ӵ�һ����ʼ*/
    if(m>len) return NULL;
    p += m;
    while(n--) *(q++) = *(p++);
    *(q++)='\0'; /*�б�Ҫ�𣿺��б�Ҫ*/
    return dst;
}


void DeleteMid(u8 *sbuf,u16 *year,u8 *month,u8 *date)
{
	u8 buf[3]={0};
	u8 *p1=buf;
	while(*sbuf!='-')
	{
		*p1=*sbuf;
		p1++;
		sbuf++;
	}
	sbuf++;
	*year=(strtol((char*)buf,NULL,10));
	mymemset(buf,0,sizeof(buf));
	p1=buf;
	while(*sbuf!='-')
	{
		*p1=*sbuf;
		p1++;
		sbuf++;
	}
	sbuf++;
	*month=(u8)(strtol((char*)buf,NULL,10));
	*date=(u8)(strtol((char*)sbuf,NULL,10));
}


void GetTime2Use(u16 *year,u8 *month,u8 *date,u8 *hour,u8 *min,u8 *seond,u8 *sbuf)
{
	u8 timebuf[10]={0};
	u8 timebuf1[10]={0};
	u8 *p3=timebuf1;
	u8 buf[3]={0};
	while(*sbuf!=0x20)sbuf++;
	sbuf++;
	while(*sbuf!=0x20)
		{
			*p3=*sbuf;
			sbuf++;
			p3++;
		}
	sbuf++;
	DeleteMid(timebuf1,year,month,date);
	strcpy((char*)timebuf,(char*)sbuf);
	mid(buf,timebuf,2,0);
	*hour=(u8)(strtol((char*)buf,NULL,10));
	mid(buf,timebuf,2,3);
	*min=(u8)(strtol((char*)buf,NULL,10));
	mid(buf,timebuf,2,6);
	*seond=(u8)(strtol((char*)buf,NULL,10));

}


void Get_Str_Use2(u8 *ipstr,u8 *buf)
{
	memset(ipstr,'\0',sizeof(ipstr));
	while(*buf!='=')buf++;
	buf++;
	strcpy((char*)ipstr,(char*)buf);
}



u8 Is_Error_db(u32 bd)
{
	if(bd==1200||bd==2400||bd==4800||bd==9600||bd==19200||bd==38400||bd==57600||bd==115200)return 1;
	else return 0;
}

u8 Is_Error_pt(u32 pt)
{
	if(pt>=3)return 0;
	return 1;
}

u8 Is_Error_fre(u32 fre)
{
	if(fre>=500)return 0;
	if(fre<400)return 0;
	return 1;
}
u8 Is_Error_mode(u32 md)
{
	if(md>=3)return 0;
	return 1;
}
u8 Is_Error_fac(u32 fac)
{
	return 1;
}

u8 Is_Error_bw(u32 bw)
{
	return 1;
}

u8 Is_Error_ctid(u32 ctid)
{
	return 1;
}

u8 Is_Error_netid(u32 netid)
{
	return 1;
}

u8 Is_Error_rfpower(u32 rfpower)
{
	return 1;
}

u8 Is_Error_rflength(u16 length)
{
	return 1;
}

void User_Command_Analysis(u8 *buf)
{
	UserSysCommad(buf);
	SX1276UsartAys(buf);
	FaMenUart(buf);

}

