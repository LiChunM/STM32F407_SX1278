#include "sxdowndata.h"
#include "sxprotocol.h"

SXDowndata sxdownloaddata;


void CALCDATA(void)
{
	if(systemset.datamode==0)
		{
			
		}
	if(systemset.datamode==1)
		{
			
		}
}

u8  Is_NeedCheckRtc(_calendar_obj *calc)
{
	u8 res=0;
	calendar_get_time(&calendar);
	delay_ms(10);
	calendar_get_date(&calendar);

	if(calc->w_year!=(calendar.w_year-2000))res=1;
	if(calc->w_month!=calendar.w_month)res=1;
	if(calc->w_date!=calendar.w_date)res=1;
	if(calc->hour!=calendar.hour)res=1;

	return res;
}

void AddCtemhuminfoList(u8 *buf)
{
	if(buf[36]==0x1b)
		{
			sxdownloaddata.ctem=(buf[37]<<8)|buf[38];  //温度扩大10倍
			sxdownloaddata.chum=(buf[39]<<8)|buf[40]; //湿度扩大10倍  
		}
	if(buf[41]==0x1e)
		{
			sxdownloaddata.clight=(buf[42]<<8)|buf[43]; 
		}
	if(buf[44]==0x1f)
		{
			sxdownloaddata.co2=(buf[45]<<8)|buf[46]; 
		}
	if(buf[47]==0xee)
		{
			sxdownloaddata.vol=(buf[48]<<8)|buf[49]; 
		}
	if(SystemDebug==2)
		{
			printf("id = %02X%02X\r\n",buf[27],buf[28]);
			printf("time = %02X-%02X-%02X  %02X:%02X:%02X\r\n",buf[18],buf[19],buf[20],buf[21],buf[22],buf[23]);
			printf("ctem = %d.%d\r\n",sxdownloaddata.ctem/10,sxdownloaddata.ctem%10);
			printf("chum = %d.%d\r\n",sxdownloaddata.chum/10,sxdownloaddata.chum%10);
			printf("clight = %d\r\n",sxdownloaddata.clight);
			printf("co2   = %d\r\n",sxdownloaddata.co2);
			printf("vol   = %d.%d\r\n",sxdownloaddata.vol/10,sxdownloaddata.vol%10);
		}

	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.chum=(buf[39]<<8)|buf[40];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.ctem=(buf[37]<<8)|buf[38];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.clight=(buf[42]<<8)|buf[43];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.conum=(buf[45]<<8)|buf[46];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.vola=(buf[48]<<8)|buf[49]; 
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.sxsensorcalendar.w_year=buf[18];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.sxsensorcalendar.w_month=buf[19];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.sxsensorcalendar.w_date=buf[20];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.sxsensorcalendar.hour=buf[21];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.sxsensorcalendar.min=buf[22];
	SXAddListInfo.IDList[CurrtWorkNum].Sensordata.sxsensorcalendar.sec=buf[23];

}
void AddCO2infoList(u8 *buf)
{
	
}
void AddTtemhuminfoList(u8 *buf)
{
	
}
u8 sxdowndataanyasaddr(u8 *sxbuf)
{
	u8 res=0;
	if(sxbuf[0]==0x7e&&sxbuf[1]==0x7e)
		{
			if(sxbuf[2]==systemset.ID[0])
				{
					if(sxbuf[3]==systemset.ID[1])
						{
							if(sxbuf[4]==systemset.ID[2])
								{
									res=1;
								}
						}
				}
		}
	return res;
}


u8 Is_Error_SubId(u8 mainsensorid,u8 sensorsubid1,u8 sensorsubid2)
{
	u8 res=0;
	if(mainsensorid==SXAddListInfo.IDList[CurrtWorkNum].mainid)
		{
			if(sensorsubid1==((SXAddListInfo.IDList[CurrtWorkNum].subid&0xff00)>>8))
				{
					if(sensorsubid2==(SXAddListInfo.IDList[CurrtWorkNum].subid&0xff))res=1;
				}
		}
	return res;
}

u8 sxdowndatasubsensoriduse(u8 *sxbuf)
{
	u8 res=0;
	res=sxdowndataanyasaddr(sxbuf);
	if(res==1)
		{
			res=Is_Error_SubId(sxbuf[5],sxbuf[6],sxbuf[7]);
		}
	return res;
}



u8 sxdowndataansysall(u8 *sxbuf)
{
	u8 i;
	_calendar_obj tempcalendar;
	if(sxbuf[10]==SXProtocoCHECKID)
		{
			if(sxbuf[15]==SXProtocoBEDATA)
				{
					
					tempcalendar.w_year=Hex2Dec(sxbuf[18]);
					tempcalendar.w_month=Hex2Dec(sxbuf[19]);
					tempcalendar.w_date=Hex2Dec(sxbuf[20]);
					tempcalendar.hour=Hex2Dec(sxbuf[21]);
					tempcalendar.min=Hex2Dec(sxbuf[22]);
					tempcalendar.sec=Hex2Dec(sxbuf[23]);
					sxdownloaddata.sxdowndatainfo=Is_NeedCheckRtc(&tempcalendar);	
					
					sxdownloaddata.sxpacketssensorid=sxbuf[29];

					sxdownloaddata.sxsensorcalendar.w_year=Hex2Dec(sxbuf[31]);
					sxdownloaddata.sxsensorcalendar.w_month=Hex2Dec(sxbuf[32]);
					sxdownloaddata.sxsensorcalendar.w_date=Hex2Dec(sxbuf[33]);
					sxdownloaddata.sxsensorcalendar.hour=Hex2Dec(sxbuf[34]);
					sxdownloaddata.sxsensorcalendar.min=Hex2Dec(sxbuf[35]);
					if(sxdownloaddata.sxpacketssensorid==SUBSENSOR_C_TEMHUM)
						{
							AddCtemhuminfoList(sxbuf);
							sxdownloaddata.sxreciveinfo|=1<<0;
						}
					if(sxdownloaddata.sxpacketssensorid==SUBSENSORCO2)
						{
							AddCO2infoList(sxbuf);
							sxdownloaddata.sxreciveinfo|=1<<1;
						}
					if(sxdownloaddata.sxpacketssensorid==SUBSENSOR_T_TEMHUM)
						{
							AddTtemhuminfoList(sxbuf);
							sxdownloaddata.sxreciveinfo|=1<<2;
						}
				}
		}
	if(sxbuf[10]==SXProtocoSETRTC)
		{
			sxdownloaddata.sxreciveinfo|=1<<3;
		}
}



u8 waitsubsensorack(u16 time)
{
	u8 res=0;
	while(--time)	
		{
			delay_ms(10);
			if(sxdownloaddata.sxreciveinfo&0x01)break;
		}
	sxdownloaddata.sxreciveinfo&=~(1<<0);
	if(time==0)res=1;
	return res;
}


u8 waitsubsensorackTest(u16 time)
{
	u8 res=0;
	while(--time)	
		{
			delay_ms(10);
			if(sxdownloaddata.sxreciveinfo&0x20)break;
		}
	sxdownloaddata.sxreciveinfo&=~(1<<5);
	if(time==0)res=1;
	return res;
}

u8 sxdowndataansysallTest(u8 *sxbuf)
{
	u8 i;
	_calendar_obj tempcalendar;
	if(sxbuf[10]==SXProtocoCHECKID)
		{
			if(sxbuf[15]==SXProtocoBEDATA)
				{
					
					tempcalendar.w_year=Hex2Dec(sxbuf[18]);
					tempcalendar.w_month=Hex2Dec(sxbuf[19]);
					tempcalendar.w_date=Hex2Dec(sxbuf[20]);
					tempcalendar.hour=Hex2Dec(sxbuf[21]);
					tempcalendar.min=Hex2Dec(sxbuf[22]);
					tempcalendar.sec=Hex2Dec(sxbuf[23]);
					sxdownloaddata.sxdowndatainfo=Is_NeedCheckRtc(&tempcalendar);	
					
					sxdownloaddata.sxpacketssensorid=sxbuf[29];

					sxdownloaddata.sxsensorcalendar.w_year=Hex2Dec(sxbuf[31]);
					sxdownloaddata.sxsensorcalendar.w_month=Hex2Dec(sxbuf[32]);
					sxdownloaddata.sxsensorcalendar.w_date=Hex2Dec(sxbuf[33]);
					sxdownloaddata.sxsensorcalendar.hour=Hex2Dec(sxbuf[34]);
					sxdownloaddata.sxsensorcalendar.min=Hex2Dec(sxbuf[35]);
					if(sxdownloaddata.sxpacketssensorid==SUBSENSOR_C_TEMHUM)
						{
							AddCtemhuminfoList(sxbuf);
							sxdownloaddata.sxreciveinfo|=1<<5;
							
						}
					if(sxdownloaddata.sxpacketssensorid==SUBSENSORCO2)
						{
							AddCO2infoList(sxbuf);
							
						}
					if(sxdownloaddata.sxpacketssensorid==SUBSENSOR_T_TEMHUM)
						{
							AddTtemhuminfoList(sxbuf);
							
						}
				}
		}
}

