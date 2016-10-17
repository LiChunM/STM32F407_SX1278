#include "sys.h"  
#include "includes.h"
#include "stmflash.h"
#include "rtc.h"
#include "mc323.h"
#include "protocol.h"
#include "sx1276-LoRa.h"
#include "w25qxx.h" 
#include "adc.h"
#include "sxprotocol.h"
#include "sxdowndata.h"
#include "usart.h"	


u8 sxdatabuf[SINGNMAX];
vu32 MyOSTime=0;
tRadioDriver *Radio = NULL;
 _systeminfo systeminfo;
//////////////////////////////////////////////////////////////////////////////////	 


volatile u8 SystemDebug=0;
volatile u8 SystemFlow=0;
volatile u8 NeedForRtuTheData=0;
volatile u8 CHECK_END_BITE=0;
_system_setings systemset;

//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//关闭所有中断(但是不包括fault和NMI中断)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//开启所有中断
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}


void mymemset(void *s,u8 c,u32 count)  
{  
    u8 *xs = s;  
    while(count--)*xs++=c;  
}	   


void Sys_Soft_Reset(void)
{
	SCB->AIRCR =0X05FA0000|(u32)0x04;
}

void sysset_read_para(_system_setings * sysset)
{
	STMFLASH_Read(FLASH_SAVE_ADDR,(u32*)sysset,sizeof(_system_setings));
}

void sysset_save_para(_system_setings * sysset)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
	STMFLASH_Write(FLASH_SAVE_ADDR, (u32*)sysset, sizeof(_system_setings));
	OS_EXIT_CRITICAL();	

}


void syssetid_read_para(SXSenIDList * sysset)
{
	W25QXX_Read((u8*)sysset,100,sizeof(SXSenIDList));
}

void syssetid_save_para(SXSenIDList * sysset)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
	W25QXX_Write( (u8*)sysset,100,sizeof(SXSenIDList));
	OS_EXIT_CRITICAL();	

}



void syssetfamen_read_para(FMIDList * sysset)
{
	W25QXX_Read((u8*)sysset,100+100+sizeof(SXSenIDList),sizeof(FMIDList));
}

void syssetfamen_save_para(FMIDList * sysset)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
	W25QXX_Write( (u8*)sysset,100+100+sizeof(SXSenIDList),sizeof(FMIDList));
	OS_EXIT_CRITICAL();	

}



void sx1278data_read_para(_sx1278_setings *sxdata)
{
	W25QXX_Read((u8*)sxdata,FLASH_SIZE-100-sizeof(_sx1278_setings),sizeof(_sx1278_setings));
}



void sx1278data_save_para(_sx1278_setings *sxdata)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
	W25QXX_Write( (u8*)sxdata,FLASH_SIZE-100-sizeof(_sx1278_setings),sizeof(_sx1278_setings));
	OS_EXIT_CRITICAL();	
}

void SX1278_Parameter_Init(void)
{
	u8 i;
	sx1278data_read_para(&sx1278data);
	if(sx1278data.saveflag!=0X0A)
		{
			sx1278data.modulepata.bdate=9600;
			sx1278data.modulepata.parity=0;
			sx1278data.modulepata.customerid=0;
			sx1278data.modulepata.netid=0;
			sx1278data.modulepata.rfmode=0;
			sx1278data.modulepata.rfbw=7;
			sx1278data.modulepata.rffactor=12;
			sx1278data.modulepata.rffrequency=433;
			sx1278data.modulepata.errorcode=1;
			sx1278data.modulepata.rfpower=20;
			sx1278data.saveflag=0x0A;
			sx1278data_save_para(&sx1278data);
		}
}



void 	SYS_Parameter_Init(void)
{
	printf("+WLW-STM32F4 Version 1.0-0\r\n");
	sysset_read_para(&systemset);
	if(systemset.saveflag!=0X0A)
		{
			systemset.HandInter=60;
			systemset.ATime[0]=180;
			systemset.ATime[1]=30;
			systemset.ATime[2]=30;
			sprintf((char*)systemset.CenterIP,"192.168.1.100");
			sprintf((char*)systemset.CenterPort,"8888");
			sprintf((char*)systemset.SN,"000000000");
			sprintf((char*)systemset.Centerapn,"cmnet");
			systemset.TCPorUDP=1;
			systemset.UserDelayTime=10;
			systemset.datamode=0;
			systemset.workmode=0;
			systemset.ID[0]=0x00;
			systemset.ID[1]=0x00;
			systemset.ID[2]=0x00;
			systemset.saveflag=0x0A;
			sysset_save_para(&systemset);
			sxsensoridinit();
			FMIDListInit();
			
		}
	else
		{
			calendar_get_time(&calendar);
			delay_ms(10);
			calendar_get_date(&calendar);
			printf("+time=%04d-%02d-%02d %02d:%02d:%02d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
			printf("+SN %s\r\n",systemset.SN);
			printf("+ip %s %s\r\n",systemset.CenterIP,systemset.CenterPort);
			printf("+apn %s\r\n",systemset.Centerapn);
			printf("+tcp-udp mode:%s\r\n",modetbl[systemset.TCPorUDP]);
			printf("+hand %d\r\n",systemset.HandInter);
			printf("+datamode %d\r\n",systemset.datamode);
			syssetid_read_para(&SXAddListInfo);
			syssetfamen_read_para(&FaMenInfo);

		}
	sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[0]=systemset.ID[0];
	sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[1]=systemset.ID[1];
	sxprotocolpackge.sxrotocoladdr.sxpacketspreaddr[2]=systemset.ID[2];
	SystemFlow=0;
}


void System_Restart(void)
{
	SystemFlow=SYSRESETBIT;
	M35PowerOff();
	delay_ms(500);
	Sys_Soft_Reset();
}


void UserdataInit(void)
{
	Radio = RadioDriverInit( );
	Radio->Init( );
	SXProtocolInitPara();
}

void sxsendbuf(u8 *buf,u8 length)
{
     u8 t;
     if(systemset.datamode==0)
     	{
		     Radio->SetTxPacket( buf, length );
		     systeminfo.SystemFlow=1;
		     while(1)
		     	{
		     		t=Radio->Process();
				if(t==RF_TX_DONE)
					{
						Radio->StartRx();
						break;
					}
		     		delay_ms(10);
		     	}
		     systeminfo.SystemFlow=0;
     	}
     if(systemset.datamode==1)
     	{
     		RS485ONE_Send_Data(buf,length);
     	}
}


void HeaderWare_Init(void)
{
	uart6_init(115200);
	power_drv_init();
	W25QXX_Init();
	SpiInit();
	SX1276InitIo( );
}




void UserSysCommad(u8 *buf)
{
	u8 res;
	u8 mybuf[10];
	u8 tempbuf[20]={0};
	u8 *p,*s;
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
	p=(u8*)strstr((const char*)buf,"$setinfo");
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
			printf("+adcv %d.%d\r\n",adcv/100,adcv%100);
		
		}
	p=(u8*)strstr((const char*)buf,"$info-all");
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
			printf("+adcv %d.%d\r\n",adcv/100,adcv%100);
			printf("+ATime[0] %d\r\n",systemset.ATime[0]);
			printf("+ATime[1] %d\r\n",systemset.ATime[1]);
			printf("+ATime[2] %d\r\n",systemset.ATime[2]);
		
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
			sysset_save_para(&systemset);
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
	s=(u8*)strstr((const char*)buf,"$setslwkt");
	if(s!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,s);
			idall=strtol((const char*)mybuf,NULL,16);
			systemset.HandInter=(idall>>16)&0xffff;
			systemset.ATime[0]=idall&0xffff;
			sysset_save_para(&systemset);
			printf("+setslwkt ok\r\n");
		}
	s=(u8*)strstr((const char*)buf,"$setwkslet");
	if(s!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,s);
			idall=strtol((const char*)mybuf,NULL,16);
			systemset.ATime[1]=(idall>>16)&0xffff;
			systemset.ATime[2]=idall&0xffff;
			sysset_save_para(&systemset);
			printf("+setwkslet ok\r\n");
		}
}


