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
	u8 t;
	u8 res;
	u8 mybuf[10];
	u8 tempbuf[20]={0};
	u8 *p,*s,*pa;
	u32 idall=0;
	u32 value=0;
	p=(u8*)strstr((const char*)buf,"$$debug 2");
	if(p!=NULL)
		{
			SystemDebug=2;
			printf("DEBUG_GSM_ON\r\n");
		}
	p=(u8*)strstr((const char*)buf,"$$debug 4");
	if(p!=NULL)
		{
			SystemDebug=4;
			printf("DEBUG_RS485_ON\r\n");
		}
	p=(u8*)strstr((const char*)buf,"$$debug 5");
	if(p!=NULL)
		{
			SystemDebug=5;
			printf("DEBUG_Protocol_ON\r\n");
		}
	p=(u8*)strstr((const char*)buf,"$$debug 0");
	if(p!=NULL)
		{
			SystemDebug=0;
			printf("DEBUG_OFF\r\n");
		}
	p=(u8*)strstr((const char *)buf,"$setip");
	if(p!=NULL)
		{
			mymemset(systemset.CenterIP,0,sizeof(systemset.CenterIP));
			Get_Str_Use(systemset.CenterIP,p);
			sysset_save_para(&systemset);
		    	printf("+IP %s\r\n",(u8*)systemset.CenterIP);
		}
	p=(u8*)strstr((const char *)buf,"$setpot");
	if(p!=NULL)
		{
			mymemset(systemset.CenterPort,0,sizeof(systemset.CenterPort));
			Get_Str_Use(systemset.CenterPort,p);
			sysset_save_para(&systemset);
			printf("+PORT %s\r\n",(u8*)systemset.CenterPort);

		}
	p=(u8*)strstr((const char *)buf,"$setapn");
	if(p!=NULL)
		{
			mymemset(systemset.Centerapn,0,sizeof(systemset.Centerapn));
			Get_Str_Use(systemset.Centerapn,p);
			sysset_save_para(&systemset);
		    	printf("+APN %s\r\n",(u8*)systemset.Centerapn);
		}
	p=(u8*)strstr((const char*)buf,"$sethnd");
	if(p!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,p);
			systemset.HandInter=strtol((const char*)mybuf,NULL,10);
			sysset_save_para(&systemset);
			sxdownloaddata.sxtimeneedupdate=1;
			jishih=0;
			printf("+HAND %d\r\n",systemset.HandInter);
		}
	p=(u8*)strstr((const char *)buf,"$setsn");
	if(p!=NULL)
		{
			mymemset(systemset.SN,0,sizeof(systemset.SN));
			Get_Str_Use(systemset.SN,p);
			sysset_save_para(&systemset);
			printf("+SN %s\r\n",(u8*)systemset.SN);
		}
	p=(u8*)strstr((const char*)buf,"$info");
	if(p!=NULL)
		{
			calendar_get_time(&calendar);
			delay_ms(20);
			calendar_get_date(&calendar);
			printf("+time=%04d-%02d-%02d %02d:%02d:%02d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
			printf("+SN %s\r\n",systemset.SN);
			printf("+ip %s %s\r\n",systemset.CenterIP,systemset.CenterPort);
			printf("+apn %s\r\n",systemset.Centerapn);
			printf("+tcp-udp mode:%s\r\n",modetbl[systemset.TCPorUDP]);
			printf("+hand %d\r\n",systemset.HandInter);
			printf("+datamode %d\r\n",systemset.datamode);
			printf("+jishih %d\r\n",jishih);
			printf("+adcv %d\r\n",adcv/100,adcv%100);
		
		}
	p=(u8*)strstr((const char *)buf,"$settime");
	if(p!=NULL)
		{
			s=(u8*)strstr((const char *)buf,"-");
			if(s!=NULL)
				{
					GetTime2Use(&calendar.w_year,&calendar.w_month,&calendar.w_date,&calendar.hour,&calendar.min,&calendar.sec,p);
					RTC_Set(calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
					printf("+time=%04d-%02d-%02d %02d:%02d:%02d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);

				}
		}
	p=(u8*)strstr((const char*)buf,"$reset");
	if(p!=NULL)
		{
			if(SystemFlow==0)
				{	
					SystemFlow=1;
					printf("+Reset Ok\r\n");
				}
			else
				{
					printf("+Reset Error\r\n");
				}
		}
	p=(u8*)strstr((const char*)buf,"$tcp");
	if(p!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,p);
			systemset.TCPorUDP=strtol((const char*)mybuf,NULL,10);
			printf("+tcp-udp mode: %s\r\n",modetbl[systemset.TCPorUDP]);
		}
	p=(u8*)strstr((const char*)buf,"$setdelayt");
	if(p!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,p);
			systemset.UserDelayTime=strtol((const char*)mybuf,NULL,10);
			sysset_save_para(&systemset);
			printf("+delaytime %d\r\n",systemset.UserDelayTime);
		}

	p=(u8*)strstr((const char*)buf,"$setdatamod");
	if(p!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,p);
			systemset.datamode=strtol((const char*)mybuf,NULL,10);
			sysset_save_para(&systemset);
			printf("+datamode %d\r\n",systemset.datamode);
		}
	
	p=(u8*)strstr((const char*)buf,"$setworkmod");
	if(p!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,p);
			systemset.workmode=strtol((const char*)mybuf,NULL,10);
			sysset_save_para(&systemset);
			printf("+workmode %d\r\n",systemset.workmode);
		}



	pa=(u8*)strstr((const char *)buf,"AT++");
	if(pa!=NULL)
		{
			printf("\r\n");
			printf("AT+SXBDATE\r\n");
			printf("AT+SXPAR\r\n");
			printf("AT+SXRFRFE\r\n");
			printf("AT+SXRFMODE\r\n");
			printf("AT+SXRFFAC\r\n");
			printf("AT+SXRFBW\r\n");
			printf("AT+SXCTID\r\n");
			printf("AT+SXNETID\r\n");
			printf("AT+SXPOWER\r\n");
			printf("\r\n+OK\r\n");
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXBDATE");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXBDATE?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXBDATE:%d\r\n",sx1278data.modulepata.bdate);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXBDATE=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_db(value);
					if(res)
						{
							sx1278data.modulepata.bdate=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXPAR");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXPAR?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXPAR:%d\r\n",sx1278data.modulepata.parity);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXPAR=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_pt(value);
					if(res)
						{
							sx1278data.modulepata.parity=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFRFE");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFRFE?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFRFE:%d\r\n",sx1278data.modulepata.rffrequency);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFRFE=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_fre(value);
					if(res)
						{
							sx1278data.modulepata.rffrequency=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFMODE");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFMODE?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFMODE:%d\r\n",sx1278data.modulepata.rfmode);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFMODE=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_mode(value);
					if(res)
						{
							sx1278data.modulepata.rfmode=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFFAC");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFFAC?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFFAC:%d\r\n",sx1278data.modulepata.rffactor);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFFAC=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_fac(value);
					if(res)
						{
							sx1278data.modulepata.rffactor=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFBW");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFBW?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFBW:%d\r\n",sx1278data.modulepata.rfbw);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFBW=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_bw(value);
					if(res)
						{
							sx1278data.modulepata.rfbw=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFPOWER");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFPOWER?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFPOWER:%d\r\n",sx1278data.modulepata.rfpower);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFPOWER=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_rfpower(value);
					if(res)
						{
							sx1278data.modulepata.rfpower=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFCSQ?");
	if(pa!=NULL)
		{
			printf("\r\n");
			printf("+SXRFCSQ:-%d\r\n",SX1276LoRaReadRssiChar());
			printf("\r\n+OK\r\n");
		}




	
	s=(u8*)strstr((const char*)buf,"$setcenterid");
	if(s!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,s);
			idall=strtol((const char*)mybuf,NULL,16);
			sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[0]=(idall&0xff0000)>>16;
			sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[1]=(idall&0xff00)>>8;
			sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[2]=idall&0xff;
			systemset.ID[0]=sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[0];
			systemset.ID[1]=sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[1];
			systemset.ID[2]=sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[2];
			sysset_save_para(&systemset);
			printf("+setcenterid ok\r\n");
		}
	
	s=(u8*)strstr((const char*)buf,"$setsxsenid");
	if(s!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,s);
			idall=strtol((const char*)mybuf,NULL,16);
			res=(idall&0xff0000)>>16;
			AddSensorIDList(res,idall&0xffff);
			
		}

	s=(u8*)strstr((const char*)buf,"$clearsxsenid");
	if(s!=NULL)
		{
			sxsensoridinit();
			printf("+clearsxsenid ok\r\n");
		}
	
	s=(u8*)strstr((const char*)buf,"$setm35on");
	if(s!=NULL)
		{
			systeminfo.SystemFlow=3;
			printf("+setm35on\r\n");
		}
	FaMenUart(buf);

}

