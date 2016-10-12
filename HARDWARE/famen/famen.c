#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "includes.h" 
#include "usart2.h"   
#include "protocol.h"
#include "rtc.h"
#include "string.h"
#include "famen.h"



FMIDList	FaMenInfo;

u8 FaMenDataBuf[28];




void FMIDListInit(void)
{
	u8 i,j;
	FaMenInfo.ListNum=0;
	for(i=0;i<4;i++)FaMenInfo.Func[i]=0x00;
	for(i=0;i<FMLISTNUM;i++)
		{
			for(j=0;j<IDTABLE;j++)FaMenInfo.IDList[i].efamenid[j]=0x00;
		}
	syssetfamen_save_para(&FaMenInfo);
}

static u8 checkbcc(u8 *buf,u8 len)
{
	u8 temp=0;
	u8 i;
	for(i=0;i<len;i++)
	{
		temp^=buf[i];
	}
	return ~(temp&0xff);
}


u8 Add2FamenList(u8 *idbuf)
{
	u8 i,id;
	u32 idtemp;
	idtemp=strtol((const char*)idbuf,NULL,16);
	syssetfamen_read_para(&FaMenInfo);
	for(i=0;i<FaMenInfo.ListNum;i++)
		{
			id=(idtemp&0xff000000)>>24;
			if(id==FaMenInfo.IDList[i].efamenid[0])
				{
					id=(idtemp&0xff0000)>>16;
					if(id==FaMenInfo.IDList[i].efamenid[1])
						{
							id=(idtemp&0xff00)>>8;
							if(id==FaMenInfo.IDList[i].efamenid[2])
								{
									id=idtemp&0xff;
									if(id==FaMenInfo.IDList[i].efamenid[3])return 1;
								}
						}
				}
		}
	FaMenInfo.IDList[FaMenInfo.ListNum].efamenid[0]=(idtemp&0xff000000)>>24;
	FaMenInfo.IDList[FaMenInfo.ListNum].efamenid[1]=(idtemp&0xff0000)>>16;
	FaMenInfo.IDList[FaMenInfo.ListNum].efamenid[2]=(idtemp&0xff00)>>8;
	FaMenInfo.IDList[FaMenInfo.ListNum].efamenid[3]=idtemp&0xff;
	FaMenInfo.ListNum++;
	if(FaMenInfo.ListNum>=FMLISTNUM)FaMenInfo.ListNum=0;
	syssetfamen_save_para(&FaMenInfo);
	return 0;
}


/*FF FF 15 12 01 AC 23        00 00 00 00   00 00 00 00     00 00 00 00     00 FF 12 01     00 00 00 00    9A */

void FamenInitData(u8 *fmid,u8 keycode,u8 *setbuf)
{
	u8 i;
	FaMenDataBuf[0]=0xff;
	FaMenDataBuf[1]=0xff;
	for(i=0;i<4;i++)FaMenDataBuf[2+i]=fmid[i];
	FaMenDataBuf[6]=keycode;
	for(i=0;i<12;i++)FaMenDataBuf[7+i]=0x00;
	for(i=0;i<4;i++)FaMenDataBuf[19+i]=setbuf[i];
	for(i=0;i<4;i++)FaMenDataBuf[23+i]=0x00;
	FaMenDataBuf[27]=checkbcc(FaMenDataBuf,27);
}


void FaMenUart(u8 *famenbuf)
{
	u8 *s;
	u8 res,i,j;
	u8 mybuf[10];
	s=(u8*)strstr((const char*)famenbuf,"$testvalv+++");
	if(s!=NULL)
		{
			TestBit=1;
			printf("+testvalv on\r\n");
		}
	s=(u8*)strstr((const char*)famenbuf,"$setvalveid");
	if(s!=NULL)
		{
			mymemset(mybuf,0,sizeof(mybuf));
			Get_Str_Use(mybuf,s);
			res=Add2FamenList(mybuf);
			if(res)printf("setvalveid error\r\n");
			else	printf("setvalveid sucess\r\n");
		}
	s=(u8*)strstr((const char*)famenbuf,"$setvalvallon");
	if(s!=NULL)
		{
			printf("valvall on .....\r\n");
			FaMenInfo.Func[0]=FAMENONDATA;
			FaMenInfo.Func[1]=FAMENROOTSU;
			FaMenInfo.Func[2]=0;
			FaMenInfo.Func[3]=0;
			for(i=0;i<FaMenInfo.ListNum;i++)
				{
					FamenInitData(FaMenInfo.IDList[i].efamenid,FAMENONOFFCOM,FaMenInfo.Func);
					printf("RS485_Send_Data\r\n");
					for(j=0;j<28;j++)printf("%02X ",FaMenDataBuf[j]);
					printf("\r\n");
					RS485ONE_Send_Data(FaMenDataBuf,28);
					delay_ms(2000);
				}
			printf("valvall on ok!\r\n");
		}
	s=(u8*)strstr((const char*)famenbuf,"$setvalvalloff");
	if(s!=NULL)
		{
			printf("valvall off .....\r\n");
			FaMenInfo.Func[0]=FAMEOFFNDATA;
			FaMenInfo.Func[1]=FAMENROOTSU;
			FaMenInfo.Func[2]=0;
			FaMenInfo.Func[3]=0;
			for(i=0;i<FaMenInfo.ListNum;i++)
				{
					FamenInitData(FaMenInfo.IDList[i].efamenid,FAMENONOFFCOM,FaMenInfo.Func);
					printf("RS485_Send_Data\r\n");
					for(j=0;j<28;j++)printf("%02X ",FaMenDataBuf[j]);
					printf("\r\n");
					RS485ONE_Send_Data(FaMenDataBuf,28);
					delay_ms(2000);
				}
			printf("valvall off ok!\r\n");
		}
	s=(u8*)strstr((const char*)famenbuf,"$seticealloff");
	if(s!=NULL)
		{
			printf("iceall off .....\r\n");
			FaMenInfo.Func[0]=0x11;
			FaMenInfo.Func[1]=0x00;
			FaMenInfo.Func[2]=0xff;
			FaMenInfo.Func[3]=0xff;
			for(i=0;i<FaMenInfo.ListNum;i++)
				{
					FamenInitData(FaMenInfo.IDList[i].efamenid,FAMENSETCOM,FaMenInfo.Func);
					printf("RS485_Send_Data\r\n");
					for(j=0;j<28;j++)printf("%02X ",FaMenDataBuf[j]);
					printf("\r\n");
					RS485ONE_Send_Data(FaMenDataBuf,28);
					delay_ms(2000);
				}
			printf("iceall off ok!\r\n");
		}
	s=(u8*)strstr((const char*)famenbuf,"$setchageall");
	if(s!=NULL)
		{
			printf("chageall  .....\r\n");
			FaMenInfo.Func[0]=0;
			FaMenInfo.Func[1]=0;
			FaMenInfo.Func[2]=0;
			FaMenInfo.Func[3]=0;
			for(i=0;i<FaMenInfo.ListNum;i++)
				{
					FamenInitData(FaMenInfo.IDList[i].efamenid,FAMENCHAGECOM,FaMenInfo.Func);
					printf("RS485_Send_Data\r\n");
					for(j=0;j<28;j++)printf("%02X ",FaMenDataBuf[j]);
					printf("\r\n");
					RS485ONE_Send_Data(FaMenDataBuf,28);
					delay_ms(2000);
				}
			printf("chageall  ok!\r\n");
		}
}











