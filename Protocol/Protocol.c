#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "includes.h" 
#include "usart2.h"   
#include "protocol.h"
#include "rtc.h"
#include "string.h"
#include "adc.h"

u8 ProtocolBuf[ProtocoMaxLen];

vu16 ProtocolFunc=0;

vu8 ProtocoWave=0;

Protocol_Send_packets Send_packets;


void Protocol_Send_Init(void)
{
	Send_packets.packets_Send_head[0]=0x7e;
	Send_packets.packets_Send_head[1]=0x7e;
	Send_packets.packets_Send_end[0]=0x0d;
	Send_packets.packets_Send_end[1]=0x0a;
}

void ProtocolDataInfoInit(void)
{
	u8 i;
	
}



void M35Get_NewRtc(void)
{
	calendar_get_time(&calendar);
	delay_ms(10);
	calendar_get_date(&calendar);
}


void  Send_InitHandData(u8 *sendlength)
{
	u8 i;
	u16 crcdata;
	u32 length=0x01;
	ProtocolBuf[0]=0x7E;
	ProtocolBuf[1]=0x7E;
	for(i=0;i<3;i++)ProtocolBuf[2+i]=systemset.ID[i];
	for(i=0;i<2;i++)ProtocolBuf[5+i]=Send_packets.passwd[i];
	ProtocolBuf[7]=SXProtocoHANDATA;
	ProtocolBuf[8]=SXProtocoDOWNDATA;
	ProtocolBuf[9]=(length&0xff0000)>>16;
	ProtocolBuf[10]=(length&0xff00)>>8;
	ProtocolBuf[11]=length&0xff;
	ProtocolBuf[12]=SXProtocoBEDATA;
	ProtocolBuf[13]=SXProtocoENDDATA;
	crcdata=SXProtoco_CRC16(ProtocolBuf,14);
	ProtocolBuf[14]=(crcdata>>8);
	ProtocolBuf[15]=(crcdata&0xff);
	*sendlength=16;
	
}


void  Send_InitRtcData(u8 *sendlength,u8 Func)
{
	u8 i;
	u16 crcdata;
	u32 length=0x01;
	ProtocolBuf[0]=0x7E;
	ProtocolBuf[1]=0x7E;
	for(i=0;i<3;i++)ProtocolBuf[2+i]=systemset.ID[i];
	for(i=0;i<2;i++)ProtocolBuf[5+i]=Send_packets.passwd[i];
	ProtocolBuf[7]=Func;
	ProtocolBuf[8]=SXProtocoDOWNDATA;
	ProtocolBuf[9]=(length&0xff0000)>>16;
	ProtocolBuf[10]=(length&0xff00)>>8;
	ProtocolBuf[11]=length&0xff;
	ProtocolBuf[12]=SXProtocoBEDATA;
	ProtocolBuf[13]=SXProtocoENDDATA;
	crcdata=SXProtoco_CRC16(ProtocolBuf,14);
	ProtocolBuf[14]=(crcdata>>8);
	ProtocolBuf[15]=(crcdata&0xff);
	*sendlength=16;
}

/*7E7E   00 00 00  0000  A0 10 00 00 11 12 00 01 16 08 24 15 59 54 01 01 
   0E0001100F00000000001B000000001E00001F0000EE0000
   0E0002100F00000000001B000000001E00001F0000EE0000
   0E0003100F00000000001B000000001E00001F0000EE0000
   EF FFFF 03 5D40*/

/*7E7E   FF FF12    0000 A0 10    00 00 11    12   00 07 16 08 24 18 31 37 02 01 

  0E0201100F00000000001B000000001E00001F0000EE0000
  
  0E0202100F00000000001B000000001E00001F0000EE0000
  
  0E0203100F00000000001B000000001E00001F0000EE0000
  
  0E0204100F00000000001B000000001E00001F0000EE0000
  
  0E0205100F00000000001B000000001E00001F0000EE0000

  
  EF FFFF 17 419F*/

void  Send_InitMasterData(u8 *sendlength,u8 packall,u8 packge,u8 subsensornum)
{
	u8 i;
	u16 crcdata;
	u32 length=0x11;
	Send_packets.packetsNums++;
	M35Get_NewRtc();
	if(packall==packge)length=subsensornum*EVESUBINFO;
	else	length=EVEPACGE*EVESUBINFO;
	ProtocolBuf[0]=0x7E;
	ProtocolBuf[1]=0x7E;
	for(i=0;i<3;i++)ProtocolBuf[2+i]=systemset.ID[i];
	for(i=0;i<2;i++)ProtocolBuf[5+i]=Send_packets.passwd[i];
	ProtocolBuf[7]=SXProtocoMastData;
	ProtocolBuf[8]=SXProtocoDOWNDATA;
	ProtocolBuf[9]=(length&0xff0000)>>16;
	ProtocolBuf[10]=(length&0xff00)>>8;
	ProtocolBuf[11]=length&0xff;
	ProtocolBuf[12]=SXProtocoBEDATA;
	ProtocolBuf[13]=(Send_packets.packetsNums&0xff00)>>8;
	ProtocolBuf[14]=Send_packets.packetsNums&0xff;
	ProtocolBuf[15]=Dec2Hex(calendar.w_year-2000);
	ProtocolBuf[16]=Dec2Hex(calendar.w_month);
	ProtocolBuf[17]=Dec2Hex(calendar.w_date);
	ProtocolBuf[18]=Dec2Hex(calendar.hour);
	ProtocolBuf[19]=Dec2Hex(calendar.min);
	ProtocolBuf[20]=Dec2Hex(calendar.sec);
	ProtocolBuf[21]=packall;
	ProtocolBuf[22]=packge;
	if(packall==packge)
		{
			for(i=0;i<subsensornum;i++)
				{
					ProtocolBuf[23+EVESUBINFO*i]=0x0E;
					ProtocolBuf[24+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].subid&0xff00)>>8;
					ProtocolBuf[25+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].subid&0xff;
					ProtocolBuf[26+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].mainid;
					ProtocolBuf[27+EVESUBINFO*i]=0x0F;
					ProtocolBuf[28+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.w_year;
					ProtocolBuf[29+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.w_month;
					ProtocolBuf[30+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.w_date;
					ProtocolBuf[31+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.hour;
					ProtocolBuf[32+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.min;
					ProtocolBuf[33+EVESUBINFO*i]=0x1B;
					ProtocolBuf[34+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.ctem&0xff00)>>8;
					ProtocolBuf[35+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.ctem&0xff;
					ProtocolBuf[36+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.chum&0xff00)>>8;
					ProtocolBuf[37+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.chum&0xff;
					ProtocolBuf[38+EVESUBINFO*i]=0x1E;
					ProtocolBuf[39+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.clight&0xff00)>>8;
					ProtocolBuf[40+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.clight&0xff;
					ProtocolBuf[41+EVESUBINFO*i]=0x1F;
					ProtocolBuf[42+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.conum&0xff00)>>8;
					ProtocolBuf[43+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.conum&0xff;
					ProtocolBuf[44+EVESUBINFO*i]=0xEE;
					ProtocolBuf[45+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.vola&0xff00)>>8;
					ProtocolBuf[46+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.vola&0xff;
				}
		}
	else
		{
			for(i=0;i<EVEPACGE;i++)
				{
					ProtocolBuf[23+EVESUBINFO*i]=0x0E;
					ProtocolBuf[24+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].subid&0xff00)>>8;
					ProtocolBuf[25+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].subid&0xff;
					ProtocolBuf[26+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].mainid;
					ProtocolBuf[27+EVESUBINFO*i]=0x0F;
					ProtocolBuf[28+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.w_year;
					ProtocolBuf[29+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.w_month;
					ProtocolBuf[30+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.w_date;
					ProtocolBuf[31+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.hour;
					ProtocolBuf[32+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.sxsensorcalendar.min;
					ProtocolBuf[33+EVESUBINFO*i]=0x1B;
					ProtocolBuf[34+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.ctem&0xff00)>>8;
					ProtocolBuf[35+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.ctem&0xff;
					ProtocolBuf[36+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.chum&0xff00)>>8;
					ProtocolBuf[37+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.chum&0xff;
					ProtocolBuf[38+EVESUBINFO*i]=0x1E;
					ProtocolBuf[39+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.clight&0xff00)>>8;
					ProtocolBuf[40+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.clight&0xff;
					ProtocolBuf[41+EVESUBINFO*i]=0x1F;
					ProtocolBuf[42+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.conum&0xff00)>>8;
					ProtocolBuf[43+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.conum&0xff;
					ProtocolBuf[44+EVESUBINFO*i]=0xEE;
					ProtocolBuf[45+EVESUBINFO*i]=(SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.vola&0xff00)>>8;
					ProtocolBuf[46+EVESUBINFO*i]=SXAddListInfo.IDList[i+EVEPACGE*(packge-1)].Sensordata.vola&0xff;
				}
			
		}
	ProtocolBuf[47+EVESUBINFO*(i-1)]=0xEF;
	ProtocolBuf[48+EVESUBINFO*(i-1)]=(adcv&0xff00)>>8;
	ProtocolBuf[49+EVESUBINFO*(i-1)]=adcv&0xff;
	if(packall==packge)ProtocolBuf[50+EVESUBINFO*(i-1)]=0x03;
	else	ProtocolBuf[50+EVESUBINFO*(i-1)]=0x17;
	crcdata=SXProtoco_CRC16(ProtocolBuf,51+EVESUBINFO*(i-1));
	ProtocolBuf[51+EVESUBINFO*(i-1)]=(crcdata>>8);
	ProtocolBuf[52+EVESUBINFO*(i-1)]=(crcdata&0xff);
	*sendlength=53+EVESUBINFO*(i-1);
	
}


/***********************************************************************/


static u8 SV_SetSysTime(u8 *databuf)
{
	u8 res=0;
	if(databuf[12]==SVSTX)
		{
			calendar.w_year=Hex2Dec(databuf[21])+2000;
			calendar.w_month=Hex2Dec(databuf[22]);
			calendar.w_date=Hex2Dec(databuf[23]);
			calendar.hour=Hex2Dec(databuf[24]);
			calendar.min=Hex2Dec(databuf[25]);
			calendar.sec=Hex2Dec(databuf[26]);
			RTC_Set(calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
		}
	if(databuf[27]==SVEOT)res=0;
	if(databuf[27]==SVESC)res=1;

	return res;
}


static u8 SV_SetHandTime(u8 *databuf)
{
	u8 res=0;
	if(databuf[12]==SVSTX)
		{
			systemset.HandInter=(((databuf[21]<<8)&0xff00)|databuf[22]&0xff)*60;
			if(databuf[27]==SVEOT)res=0;
			if(databuf[27]==SVESC)res=1;
		}
	return res;
}

static u8 SV_CheckData(u8 *databuf)
{
	u8 res=0;
	if(databuf[12]==SVSTX)
		{
			if(databuf[27]==SVEOT)res=0;
			if(databuf[27]==SVESC)res=1;
		}
	return res;
}


static u8 SV_RegSubID(u8 *databuf)
{
	u8 i,length;
	u8 res=0;
	u8 packall,pakage;
	u16 mainid;
	if(databuf[12]==SVSTX)
		{
			length=databuf[11]/3;
			for(i=0;i<length;i++)
				{	
					mainid=((databuf[22+i*3]<<8)&0xff00)|databuf[23+i*3];
					AddSensorIDList(databuf[21+i*3],mainid);
				}
		}
	if(databuf[12]==SVSYN)
		{
			packall=databuf[13];
			pakage=databuf[14];
			if(packall==pakage)
				{
					length=databuf[11]/3;
					for(i=0;i<length;i++)
						{	
							mainid=((databuf[24+i*3]<<8)&0xff00)|databuf[25+i*3];
							AddSensorIDList(databuf[23+i*3],mainid);
						}
				}
			else
				{
					for(i=0;i<SVEREGPACKK;i++)
						{
							mainid=((databuf[24+i*3]<<8)&0xff00)|databuf[25+i*3];
							AddSensorIDList(databuf[23+i*3],mainid);
						}
				}	
		}
}


static u8 SV_RegDLLID(u8 *databuf)
{
	u8 res=0;
	if(databuf[12]==SVSTX)
		{
			sxsensoridinit();
			if(databuf[27]==SVEOT)res=0;
			if(databuf[27]==SVESC)res=1;
		}
	return res;
}

static u8 SV_RegWaveID(u8 *databuf)
{
	u8 i,res;
	u8 tempidbuf[15]={0};
	for(i=0;i<5;i++)
		{
			if(databuf[28+i*4]!=0||databuf[29+i*4]!=0||databuf[30+i*4]!=0||databuf[31+i*4]!=0)
				{
					sprintf((char*)tempidbuf,"%02X%02X%02X%02X",databuf[28+i*4],databuf[29+i*4],databuf[30+i*4],databuf[31+i*4]);
					res=Add2FamenList(tempidbuf);
				}
		}
	return 0;
}




static u8 SV_OpenWaveChan(u8 *databuf)
{
	u8 i;
	for(i=0;i<28;i++)
		{
			FaMenDataBuf[i]=databuf[21+i];
		}
	return 0;
}


static u8 SV_DLLWaveID(u8 *databuf)
{

	FMIDListInit();
	return 0;
}

static u8 SV_AnysWaveData(u8 *databuf)

{
	u8 res=0;
	if(databuf[12]==SVSTX)
		{
			if(databuf[21]==0xff&&databuf[22]==0xff)
				{
					 switch (databuf[27])
					 	{
					 		case SVWAVEREG:
								SV_RegWaveID(databuf);
								ProtocoWave|=1<<0;
								break;
							case SVWAVEDLLREG:
								SV_DLLWaveID(databuf);
								ProtocoWave|=1<<1;
								break;
							case	0x10:
							case	0x11:
							case	0x20:
							case	0x21:
							case	0x22:
							case	0x23:
							case	0x2f:
							case	0x30:
							case	0x31:
							case	0x32:
							case	0x33:
							case	0xee:
								SV_OpenWaveChan(databuf);
								ProtocoWave|=1<<2;
								break;
							default:
								break;
					 	}
				}
			if(databuf[27]==SVEOT)res=0;
			if(databuf[27]==SVESC)res=1;
		}
	return res;
}

/***********************************************************************/

/*校时数据*/
/*7E 7E 01 01 FF 12 34 4A 10 00 00 06 12 00 01 16 10 13 17 42 00 16 10 13 17 42 10 1B 1E 2D*/

/*注册sensor id*/
/*7E 7E 01 01 FF 12 34 3D 10 00 00 06 12 00 01 16 10 13 17 42 00  10 02 08 10 02 09 04 1E 2D*/ /*不分包*/

/*删除sensor id*/

/*7E 7E  01 01 FF 12 34 3E 10 00 00 01 12 00 01 16 10 13 17 42 00 FF 04 FF FF*/

/*删除指定sensor id*/

/*7E 7E  01 01 FF 12 34 3E 10 00 00 06 12 00 01 16 10 13 17 42 00 10 02 08 10 02 09 04 FF FF*/

/*查询数据指令*/

/*7E 7E  01 01 FF 12 34 14 10 00 00 01 12 00 01 16 10 13 17 42 00 14 04 FF FF*/


/*注册阀门ID*/

/*7E 7E  01 01 FF 12 34 EA 10 00 00 1C 12 00 01 16 10 13 17 42 00  FF FF 00 00 00 05 EB 12 12 12 12 13 13 13 13 14 14 14 14 15 15 15 15 16 16 16 16 23 04 FF FF*/


/*删除阀门ID*/

/*7E 7E  01 01 FF 12 34 EA 10 00 00 1C 12 00 01 16 10 13 17 42 00  FF FF 00 00 00 05 EC 12 12 12 12 13 13 13 13 14 14 14 14 15 15 15 15 16 16 16 16 23 04 FF FF*/


void DataCore(u8 *databuf,u8 *res)
{
	if(databuf[0]==0x7e&&databuf[1]==0x7e)
		{
			systemset.ID[0]=databuf[4];
			systemset.ID[1]=databuf[2];
			systemset.ID[2]=databuf[3];
			Send_packets.passwd[0]=databuf[9];
			Send_packets.passwd[1]=databuf[10];
			 switch (databuf[7])
			 	{
			 		case SVHANDAT:
						ProtocolFunc|=1<<0;
						break;
					case SVSETRTC:
						*res=SV_SetSysTime(databuf);
						ProtocolFunc|=1<<1;
						break;
					case SVECHECK:
						*res=SV_CheckData(databuf);
						ProtocolFunc|=1<<2;
						break;
					case SVREDAOK:
						ProtocolFunc|=1<<3;
						break;
					case SVREGSUBID:
						*res=SV_RegSubID(databuf);
						ProtocolFunc|=1<<4;
						break;
					case SVREGDLLID:
						*res=SV_RegDLLID(databuf);
						ProtocolFunc|=1<<5;
						break;
					case SVWAVEPRO:
						*res=SV_AnysWaveData(databuf);
						ProtocolFunc|=1<<6;
						break;
					case SVSETTIME:
						*res=SV_SetHandTime(databuf);
						ProtocolFunc|=1<<7;
						break;
					default:
						break;
			 	}
		}
}



static void Send_WaveChdata(void)
{
	RS485ONE_Send_Data(FaMenDataBuf,28);
}

static u8 Re_WaveChata(u8 wtime)
{
	
}


void SysCheckSVAsk(void)
{
	u8 length;
	if(ProtocolFunc&0x02)
		{
			ProtocolFunc&=~(1<<1);
			Send_InitRtcData(&length,SVSETRTC);
			M35SendDataNoAck(ProtocolBuf,length,0);
		}
	if(ProtocolFunc&0x04)
		{
			ProtocolFunc&=~(1<<2);
			CalcsensorData();
			DingTimeSendForData();
		}
	if(ProtocolFunc&0x08)
		{
			ProtocolFunc&=~(1<<3);
		}
	if(ProtocolFunc&0x10)
		{
			ProtocolFunc&=~(1<<4);
			Send_InitRtcData(&length,SVREGSUBID);
			M35SendDataNoAck(ProtocolBuf,length,0);
		}
	if(ProtocolFunc&0x20)
		{
			ProtocolFunc&=~(1<<5);
			Send_InitRtcData(&length,SVREGDLLID);
			M35SendDataNoAck(ProtocolBuf,length,0);
			
		}
	if(ProtocolFunc&0x40)
		{
			ProtocolFunc&=~(1<<6);
			if(ProtocoWave&0x01)
				{
					ProtocoWave&=~(1<<0);
					Send_InitRtcData(&length,SVWAVEREG);
					M35SendDataNoAck(ProtocolBuf,length,0);
				}
			if(ProtocoWave&0x02)
				{
					ProtocoWave&=~(1<<1);
					Send_InitRtcData(&length,SVWAVEDLLREG);
					M35SendDataNoAck(ProtocolBuf,length,0);
				}
			if(ProtocoWave&0x04)
				{
					ProtocoWave&=~(1<<2);
					Send_WaveChdata();
					Re_WaveChata(100);
				}
		}
	
}


