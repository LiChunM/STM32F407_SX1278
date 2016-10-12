#include "sys.h"  
#include "includes.h"
#include "stmflash.h"
#include "rtc.h"
#include "mc323.h"
#include "protocol.h"
#include "sx1276-LoRa.h"
#include "w25qxx.h" 

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
			systemset.HandInter=100;
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



