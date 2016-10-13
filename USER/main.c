#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "includes.h" 
#include "protocol.h"
#include "mc323.h"
#include "power_drv.h"
#include "usart2.h"   
#include "rtc.h"
#include "usart3.h"	
#include "usart6.h"	
#include "adc.h"
#include "sxprotocol.h"
 

 
 /////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//LED任务
//设置任务优先级
#define LED_TASK_PRIO       			7 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		256
//任务堆栈
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);

//触摸屏任务
//设置任务优先级
#define TOUCH_TASK_PRIO       		 	6
//设置任务堆栈大小
#define TOUCH_STK_SIZE  				256
//任务堆栈	
OS_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];
//任务函数
void touch_task(void *pdata);

//队列消息显示任务
//设置任务优先级
#define QMSGSHOW_TASK_PRIO    			5 
//设置任务堆栈大小
#define QMSGSHOW_STK_SIZE  		 		512
//任务堆栈	
OS_STK QMSGSHOW_TASK_STK[QMSGSHOW_STK_SIZE];
//任务函数
void qmsgshow_task(void *pdata);


//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			4 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					512
//任务堆栈	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);

//信号量集任务
//设置任务优先级
#define FLAGS_TASK_PRIO       			3 
//设置任务堆栈大小
#define FLAGS_STK_SIZE  		 		128
//任务堆栈	
OS_STK FLAGS_TASK_STK[FLAGS_STK_SIZE];
//任务函数
void flags_task(void *pdata);


//按键扫描任务
//设置任务优先级
#define KEY_TASK_PRIO       			2 
//设置任务堆栈大小
#define KEY_STK_SIZE  					128
//任务堆栈	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);

#define RS485ONE_TASK_PRIO       			8 
//设置任务堆栈大小
#define RS485ONE_STK_SIZE  					512
//任务堆栈	
OS_STK RS485ONE_TASK_STK[RS485ONE_STK_SIZE];
//任务函数
void rs485one_task(void *pdata);

#define RS485TWO_TASK_PRIO       			9 
//设置任务堆栈大小
#define RS485TWO_STK_SIZE  					512
//任务堆栈	
OS_STK RS485TWO_TASK_STK[RS485TWO_STK_SIZE];
//任务函数
void rs485two_task(void *pdata);


int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168); 
	uart_init(9600);
	uart6_init(115200);
	power_drv_init();
	W25QXX_Init();
	SYS_Parameter_Init();
	SX1278_Parameter_Init();
	SpiInit();
	rs485Init(sx1278data.modulepata.bdate);
	My_RTC_Init();
	UserdataInit();
	Adc_Init();
	OSInit();  	 	
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	    
}
 


void start_task(void *pdata)
{
   	OS_CPU_SR cpu_sr=0;	    
	pdata = pdata;	  
	OSStatInit();					
 	OS_ENTER_CRITICAL();			  
 	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);						   
 	OSTaskCreate(touch_task,(void *)0,(OS_STK*)&TOUCH_TASK_STK[TOUCH_STK_SIZE-1],TOUCH_TASK_PRIO);	 				   
 	OSTaskCreate(qmsgshow_task,(void *)0,(OS_STK*)&QMSGSHOW_TASK_STK[QMSGSHOW_STK_SIZE-1],QMSGSHOW_TASK_PRIO);	 				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	 				   
 	OSTaskCreate(flags_task,(void *)0,(OS_STK*)&FLAGS_TASK_STK[FLAGS_STK_SIZE-1],FLAGS_TASK_PRIO);	 				   
 	OSTaskCreate(key_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);
	OSTaskCreate(rs485one_task,(void *)0,(OS_STK*)&RS485ONE_TASK_STK[RS485ONE_STK_SIZE-1],RS485ONE_TASK_PRIO);
	OSTaskCreate(rs485two_task,(void *)0,(OS_STK*)&RS485TWO_TASK_STK[RS485TWO_STK_SIZE-1],RS485TWO_TASK_PRIO);
 	OSTaskSuspend(START_TASK_PRIO);	
	OS_EXIT_CRITICAL();				
}


void led_task(void *pdata)
{
	
	while(1)
	{
		delay_ms(1000);
		if(IPCONNCET==SUCCESS)SysCheckSVAsk();
	}									 
}


void touch_task(void *pdata)
{	
	while(1)
	{
		delay_ms(10);
		Usart1CommandAnalysis();
		if(IPCONNCET==SUCCESS)Usart6CommandAnalysis();
		Update_SysVol();		
	}
}     


void qmsgshow_task(void *pdata)
{
	u8 buf[30]={0};
	u8 BufferSize,i;
	while(1)
	{
		delay_ms(10);
		if(NeedForRtuTheData)
			{
				RecevisensorData();
			}
	}									 
}


void main_task(void *pdata)
{	
	u8 res;
	u32 j=0;
 	while(1)
	{
		delay_ms(10);
		if(SystemFlow==0)
			{
				j++;
				if(j>=20000)
				{
					SystemFlow=1;
					j=0;
				}
			}
		if(SystemFlow==1)
			{
				if(SXAddListInfo.ListNum==0)
					{
						delay_ms(1000);
						printf("\r\nSensor ID Is NULL!!!\r\n");
						printf("Need to Update From Center!\r\n");
						if(IPCONNCET==ERROR)
							{
M35RESETPRO:
								M35PowerOn();
								m35_init();
								mc35_gprs_init();
								res=m35_tcpudp_conncet((u8)systemset.TCPorUDP,systemset.CenterIP,systemset.CenterPort,0);
								if(res==0)IPCONNCET=SUCCESS;
								else
								{
									IPCONNCET=ERROR;
									delay_ms(3000);
									if(SystemDebug==2)printf("IP Conncet agine\r\n");
									M35PowerOff();
									delay_ms(6000);
									goto M35RESETPRO;
								}
								
							}
						SystemFlow=0xF1;
						
					}
				else
					{
						printf("Normal Work Begin.....\r\n");
						if(systemset.workmode==0)
							{
M35RESET:
								if(IPCONNCET==ERROR)
									{
										M35PowerOn();
										m35_init();
										mc35_gprs_init();
										res=m35_tcpudp_conncet((u8)systemset.TCPorUDP,systemset.CenterIP,systemset.CenterPort,0);
										if(res==0)IPCONNCET=SUCCESS;
										else
										{
											IPCONNCET=ERROR;
											delay_ms(3000);
											if(SystemDebug==2)printf("IP Conncet agine\r\n");
											M35PowerOff();
											delay_ms(6000);
											goto M35RESET;
										}
									}
								
							}
						if(systemset.workmode==1)
							{
								Calculate_Wakeup_Time();
							}
						SystemFlow=2;
					}
			}
		
	}
}		   


void rs485one_task(void *pdata)
{	
	u8 res;
	u16 time=0;
	u8 length;
	while(1)
	{
		delay_ms(20);
		if(SystemFlow==2)
			{
				if(NeedForRtuTheData==0)
					{
						if(systemset.workmode==1)
							{
								if(TIMESE==1)
									{
										TIMESE=0;
										HeaderWare_Init();
										if(SystemDebug==2)printf("system wakeup\r\n");
										NeedForRtuTheData=1;
									}
							}
						if(systemset.workmode==0)
							{
								if(SXAddListInfo.ListNum==0)
									{
										
									}
								else
									{
										jishih++;
										if(jishih==(systemset.HandInter*50))
											{
												jishih=0;
												NeedForRtuTheData=1;
											}
									}
							}
						if(CHECK_END_BITE==1)
							{
								time++;
								if(time==200)
									{
										time=0;
										IPCONNCET=ERROR;
										M35PowerOff();
										CHECK_END_BITE=3;
										Calculate_Wakeup_Time();
									}
							}
					}
			}
		if(SystemFlow==0xF1) //repoce sensor id code
			{
REPITCODE:
				Send_InitHandData(&length);
				res=M35SendDataNoAck(ProtocolBuf,length,0);
				if(res)
					{
						IPCONNCET=ERROR;
						SystemFlow=1;
					}
				else
					{
						while(SXAddListInfo.ListNum==0)
							{
								delay_ms(10);
								time++;
								if(time>=500)
									{
										time=0;
										goto REPITCODE;
									}
							}
						
					}
				SystemFlow=1;
				
			}
		
	}
}

void rs485two_task(void *pdata)
{	
	u8 res=0;
	while(1)
	{
		delay_ms(10);
		if(NeedForRtuTheData)
			{
				CalcsensorData();
				if(systemset.workmode==1)
					{
						res=M35InitForTheData();
						if(res==0)DingTimeSendForData();
					}
				if(systemset.workmode==0)
					{
					        DingTimeSendForData();
					}
				NeedForRtuTheData=0;
			}
		
	}
}


void flags_task(void *pdata)
{	
							 
	while(1)
	{	
		delay_ms(10);
 	}
}



void key_task(void *pdata)
{	
	while(1)
	{
		delay_ms(10);
	}
}



void HardFault_Handler(void)
{
	
	u32 temp;
	temp=SCB->CFSR;					
 	printf("CFSR:%8X\r\n",temp);	
	temp=SCB->HFSR;					
 	printf("HFSR:%8X\r\n",temp);	
 	temp=SCB->DFSR;					
 	printf("DFSR:%8X\r\n",temp);	
   	temp=SCB->AFSR;					
 	printf("AFSR:%8X\r\n",temp);	
}


