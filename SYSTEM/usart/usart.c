#include "sys.h"
#include "usart.h"	
#include "rtc.h"
#include "mc323.h"
#include "adc.h"
#include "sxprotocol.h"
#include "power_drv.h"
#include "sxdowndata.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	


volatile u32 jishih=0;

vu8 TestBit=0;


//初始化IO 串口1 
//bound:波特率
void uart_init(u32 bound){
   //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
  USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif
	
}


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
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
				if((USART_RX_STA&0x8000)==0)//接收未完成
				{
					if(USART_RX_STA&0x4000)//接收到了0x0d
					{
						if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
						else USART_RX_STA|=0x8000;	//接收完成了 
					}
					else //还没收到0X0D
					{	
						if(Res==0x0d)USART_RX_STA|=0x4000;
						else
						{
							USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
							USART_RX_STA++;
							if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
						}		 
					}
				}  
			}
  } 
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
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

u8 * mid(u8 *dst,u8 *src, int n,int m) /*n为长度，m为位置*/
{
    u8 *p = src;
    u8 *q = dst;
    int len = strlen((const char*)src);
    if(n>len) n = len-m;    /*从第m个到最后*/
    if(m<0) m=0;    /*从第一个开始*/
    if(m>len) return NULL;
    p += m;
    while(n--) *(q++) = *(p++);
    *(q++)='\0'; /*有必要吗？很有必要*/
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

