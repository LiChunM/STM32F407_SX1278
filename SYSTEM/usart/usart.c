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

