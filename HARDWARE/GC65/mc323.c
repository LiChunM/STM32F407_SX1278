#include "sys.h"
#include "mc323.h"
#include "usart2.h"	
#include "delay.h"
#include "stdio.h"
#include "string.h"    
#include "includes.h"
#include "power_drv.h"
#include "protocol.h"
#include "usart.h"	
#include "usart6.h"	


const u8 *modetbl[2]={"UDP","TCP"};

volatile u8 InterCurLine=0;
volatile u8 IPCONNCET=0;
volatile u8 RtuSendIdentifier=0;
volatile u8 RtuSendFinish=0;
volatile u8 DataSensoCheck=0;
volatile u8 DataBiteRecive=0;
volatile u8 DataStatuRecive=0;


void M35PowerOn(void)
{
	DRV_GPRS_ON;
	delay_ms(1500);
	if(SystemDebug==2)printf("M35PowerON\r\n");
}

void M35PowerOff(void)
{
	u8 i=0;
	DRV_GPRS_OFF;
	if(SystemDebug==2)printf("M35PowerOff\r\n");
	for(i=0;i<40;i++)delay_ms(100);
}


u8* m35_check_cmd(u8 *str,u8 *drt)
{
	char *strx=0;
	strx=strstr((const char*)str,(const char*)drt); 
	return (u8*)strx;
}

void m35_at_response(u8 mode)
{
	if(SystemDebug==2)printf("%s\r\n",USART6_TX_BUF);
	if(mode)mymemset(USART6_TX_BUF, 0, sizeof(USART6_TX_BUF));
}



u8 m35_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0;
	u8 length;
	u8 buf[100]={0};
	OS_CPU_SR cpu_sr=0;
	USART6_RX_STA=0;
	sprintf((char*)buf,"%s\r\n",cmd);
	OS_ENTER_CRITICAL();
	USART6_CMD(buf);
	OS_EXIT_CRITICAL();
	if(ack&&waittime)		
	{
		while(--waittime)	
		{
			USART6_Receive_Data(USART6_TX_BUF,&length);
			if(m35_check_cmd(USART6_TX_BUF,ack))break;
			mymemset(USART6_TX_BUF, 0, sizeof(USART6_TX_BUF));
		}
		if(waittime==0)res=1; 
	}
	return res;
}



u8 *m35_sendipconfig_cmd(u8 *cmd,u16 waittime)
{
	u8 length;
	u8 *ack=0;
	u8 buf[100]={0};
	u16 tcent=waittime;
	OS_CPU_SR cpu_sr=0;
	USART6_RX_STA=0;
	sprintf((char*)buf,"%s\r\n",cmd);
	OS_ENTER_CRITICAL();
	USART6_CMD(buf);
	OS_EXIT_CRITICAL();
	while(tcent)
		{
			tcent--;
			USART6_Receive_Data(USART6_TX_BUF,&length);
			ack=strstr((const char*)USART6_TX_BUF,"OK");
			if(ack!=NULL)
				{
					ack=strstr((const char*)USART6_TX_BUF,"CONNECT");
					if(ack!=NULL)
						{
							if(SystemDebug==2)printf("%s\r\n",USART6_TX_BUF);
							ack=strstr((const char*)USART6_TX_BUF,"CONNECT OK");
							if(ack!=NULL)
								{
									mymemset(USART6_TX_BUF, 0, sizeof(USART6_TX_BUF));
									return (u8*)ack;
								}
							ack=strstr((const char*)USART6_TX_BUF,"ALREADY");
							if(ack!=NULL)
								{
									mymemset(USART6_TX_BUF, 0, sizeof(USART6_TX_BUF));
									return (u8*)ack;
								}
							else 
								{
									mymemset(USART6_TX_BUF, 0, sizeof(USART6_TX_BUF));
									return NULL;
								}
								
						}
					
				}
			
		}
	mymemset(USART6_TX_BUF, 0, sizeof(USART6_TX_BUF));
	return (u8*)ack;
	
}

void m35_init(void)
{
	u8 tcent;
	delay_ms(10000);
	tcent=10;
	if(SystemDebug==2)printf("AT\r\n");
	while(m35_send_cmd("AT","OK",300))
		{
			tcent--;
			delay_ms(300);
			if(SystemDebug==2)printf("AT\r\n");
			if(tcent==0)
				{
					tcent=10;
					M35PowerOff();
					delay_ms(500);
					Sys_Soft_Reset();
					break;
				}
		}
	m35_at_response(1);
	
	if(SystemDebug==2)printf("ATE0\r\n");
	m35_send_cmd("ATE0","OK",1000);
	delay_ms(500);
	m35_at_response(1);
	
	if(SystemDebug==2)printf("AT+IPR=115200&W\r\n");
	m35_send_cmd("AT+IPR=115200&W","OK",300);
	delay_ms(500);
	m35_at_response(1);
	
	if(SystemDebug==2)printf("AT+CPIN?\r\n");
	m35_send_cmd("AT+CPIN?","+CPIN",5000);
	delay_ms(500);
	m35_at_response(1);
	
	if(SystemDebug==2)printf("AT+CREG?\r\n");
	m35_send_cmd("AT+CREG?","+CREG",1000);
	delay_ms(500);
	m35_at_response(1);
	
	if(SystemDebug==2)printf("AT+CSQ\r\n");
	m35_send_cmd("AT+CSQ","+CSQ",1000);
	delay_ms(500);
	m35_at_response(1);
	
	if(SystemDebug==2)printf("AT+CMGF=1\r\n");
	m35_send_cmd("AT+CMGF=1","OK",300);
	delay_ms(500);
	m35_at_response(1);

	if(SystemDebug==2)printf("AT+CSCS=\"GSM\"\r\n");
	m35_send_cmd("AT+CSCS=\"GSM\"","OK",200);
	delay_ms(500);
	m35_at_response(1);

	
	if(SystemDebug==2)printf("AT+CNMI=2,2\r\n");
	m35_send_cmd("AT+CNMI=2,2","OK",1500);
	delay_ms(500);
	m35_at_response(1);
	
}


void mc35_gprs_init(void)
{
	u8 bufs[35]={0};
	sprintf((char*)bufs,"AT+QICSGP=1,\"%s\"",systemset.Centerapn);
	delay_ms(1000);

	if(SystemDebug==2)printf("AT+QIFGCNT=0\r\n");
	m35_send_cmd("AT+QIFGCNT=0","OK",300);
	delay_ms(500);
	m35_at_response(1);

	if(SystemDebug==2)printf("%s",bufs);
	m35_send_cmd(bufs,"OK",500);
	delay_ms(500);
	m35_at_response(1);

	if(SystemDebug==2)printf("AT+QIMUX=1\r\n");
	m35_send_cmd("AT+QIMUX=1","OK",1000);
	delay_ms(500);
	m35_at_response(1);
	
	if(SystemDebug==2)printf("AT+QIMODE=0\r\n");
	m35_send_cmd("AT+QIMODE=0","OK",1000);
	delay_ms(500);
	m35_at_response(1);

	if(SystemDebug==2)printf("AT+QISTAT\r\n");
	m35_send_cmd("AT+QISTAT","QISTAT",600);
	delay_ms(500);
	m35_at_response(1);
	
}

u8 m35_tcpudp_conncet(u8 mode,u8* ipaddr,u8* port,u8 curline)
{
	u8 tcent=3;
	u16 scent=600;
	u8 p[100]={0};
	u8 closip[10]={0};
	u8 *strx=0;
	
SERESET:	
	sprintf((char*)p,"AT+QIOPEN=%d,\"%s\",\"%s\",%s",curline,modetbl[mode],ipaddr,port);
	if(SystemDebug==2)printf("%s\r\n",p);
	strx=m35_sendipconfig_cmd(p,1000);
	if(strx!=NULL)
		{
			delay_ms(300);			
			return 0;
		}
	else
		{
			tcent--;
			if(tcent==0)
				{						
					return 1;
				}
			while(tcent!=0)
				{
					delay_ms(3000);

					sprintf((char*)closip,"AT+QICLOSE=%d",curline);
					if(SystemDebug==2)printf("%s\r\n",closip);
					m35_send_cmd(closip,"OK",1000);
					delay_ms(500);
					m35_at_response(1);
				
					goto SERESET;
					
				}
		
		}
	delay_ms(100);
	return 1;
}

u8 M35SendCmdCheckAck(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	u8 buf[100]={0};
	sprintf((char*)buf,"%s\r\n",cmd);
	USART6_CMD(buf);
	if(ack&&waittime)		
	{
		while(--waittime)	
		{
			OSTimeDlyHMSM(0,0,0,10);
			if(RtuSendIdentifier)break;
		}
		if(waittime==0)res=1; 
	}
	RtuSendIdentifier=0;
	return res;
}

void ProcessingTheirReply(u8 *RtuReplyBuf)
{
	u8 *p1,*p2,*p3;
	p1=(u8*)strstr((const char *)RtuReplyBuf,">");
	if(p1!=NULL)
		{
			if(SystemDebug==2)printf("\r\n%s\r\n",RtuReplyBuf);
			RtuSendIdentifier=1;
		}
	p2=(u8*)strstr((const char *)RtuReplyBuf,"SEND");
	if(p2!=NULL)
		{
			if(SystemDebug==2)printf("\r\n%s\r\n",RtuReplyBuf);
			p3=(u8*)strstr((const char *)RtuReplyBuf,"SEND OK");
			if(p3!=NULL)RtuSendFinish=1;
		}
}

u8 M35SendDataNoAck(u8 *data,u16 length,u8 curline)
{
 
	u8 i;
	u8 res=0;
	u8 GcCmdIntfeer[20];
	if(GcCmdIntfeer==NULL)return;
	else
		{
			mymemset(GcCmdIntfeer,0,sizeof(GcCmdIntfeer));
			sprintf((char*)GcCmdIntfeer,"AT+QISEND=%d,%d",curline,length);
			if(SystemDebug==2)
			{
				printf("\r\n");
				printf("%s\r\n",GcCmdIntfeer);
			}
			if(!(M35SendCmdCheckAck(GcCmdIntfeer,">",800)))
			{
				if(SystemDebug==2)
				{
					printf("\r\n");
					for(i=0;i<length;i++)printf("%02X",data[i]);
				}
				USART6_DATA(data,length);
			}
			else
			{
				res=1;
				M35PowerOff();
			}
			delay_ms(10);
		}
	return res;
}

u8 M35SendDataCheckOK(u16 waittime)
{
	u8 res=0;
	while(--waittime)	
		{
			delay_ms(10);
			if(RtuSendFinish)break;
		}
	RtuSendFinish=0;
	if(waittime==0)res=1;
	return res;
}



void mystrcpy(u8 *src,u8 *drt,u16 *lens)
{
	u8 sbuf[5]={0};
	u8 *p=sbuf;
	u16 i,j;
	while(*src!=',')src++;
	src++;
	while(*src!=0x0D)
		{
			*p=*src;
			src++;
			p++;
		}
	*lens=strtol((char*)sbuf,NULL,10);
	j=*lens;
	while(*src!=0x7e)
		{
			src++;
		}
	for(i=0;i<j;i++)
		{
			*drt=*src;
			drt++;
			src++;
		}
}


void m35_recive_data(u8 *mc35data,u8 len)
{
	u8 *p=NULL;
	u8 mybuf[200]={0}; 
	u16 mylen=0;    
	u16 i;
	p=(u8*)strstr((char*)mc35data,"+RECEIVE");
	if(p!=NULL)
		{
			
			mystrcpy(mc35data,mybuf,&mylen);
			if(SystemDebug==2){printf("RE:\r\n");
			for(i=0;i<mylen;i++)printf("%02X",mybuf[i]);
			printf("\r\n");}
			DataCore(mybuf,&CHECK_END_BITE);
		}
	else	
		{
			ProcessingTheirReply(mc35data);
		}
}


u8 M35InitForTheData(void)
{
	u8 res=1;
	M35PowerOn();
	m35_init();
	mc35_gprs_init();
	res=m35_tcpudp_conncet((u8)systemset.TCPorUDP,systemset.CenterIP,systemset.CenterPort,0);
	if(res==0)IPCONNCET=SUCCESS;
	return res;
}



void DingTimeSendForData(void)
{
	u8 k,res=0;
	u8 i,j;
	u8 length;

	if(SXAddListInfo.ListNum==0)
		{
			
		}
	else
		{
			i=SXAddListInfo.ListNum/EVEPACGE;
			j=SXAddListInfo.ListNum%EVEPACGE;
			if(j==0)
				{
					for(k=1;k<i+1;k++)
						{
							Send_InitMasterData(&length,i,k,EVEPACGE);
							res=M35SendDataNoAck(ProtocolBuf,length,0);
							if(res)break;
							else
								{
									res=M35SendDataCheckOK(500);
									if(res)break;
								}
						}
				}
			else
				{
					for(k=1;k<i+2;k++)
						{
							Send_InitMasterData(&length,i+1,k,j);
							res=M35SendDataNoAck(ProtocolBuf,length,0);
							if(res)break;
							else
								{
									res=M35SendDataCheckOK(500);
									if(res)break;
								}
						}
				}
			if(res)
				{
					IPCONNCET=ERROR;
					SystemFlow=1;
				}
		}
	
}

