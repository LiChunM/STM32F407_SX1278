#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "includes.h" 
#include "usart2.h"   
#include "sxprotocol.h"
#include "rtc.h"
#include "string.h"
#include "sxdowndata.h"


static unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
};
static unsigned char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
};




u8 SXProtocolBuf[SXProtocoMaxLen];

SXSenIDList SXAddListInfo;

volatile u8 CurrtWorkNum=0;

SXProtocol_Send_packets sxprotocolpackge;



u16 SXProtoco_CRC16(unsigned char *puchMsg, unsigned short usDataLen)
{
	unsigned char uchCRCHi = 0xFF ;
	unsigned char uchCRCLo = 0xFF ; 
	unsigned int uIndex ;
	while(usDataLen--)
	{
		uIndex = uchCRCHi ^ *puchMsg++ ; 
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	return (uchCRCLo << 8 | uchCRCHi) ;
}

u8 Dec2Hex(u8 num)
{
	u8 numbuf[2]={0};
	u8 hex;
	sprintf((char*)numbuf,"%02d",num);
	hex=strtol((const char*)numbuf,NULL,16);
	return hex;
}

u8 Hex2Dec(u8 num)
{
	u8 numbuf[2]={0};
	u8 dec;
	sprintf((char*)numbuf,"%02x",num);
	dec=strtol((const char*)numbuf,NULL,10);
	return dec;
}


void Get_NewRtc()
{
	calendar_get_time(&calendar);
	delay_ms(10);
	calendar_get_date(&calendar);
	sxprotocolpackge.sxcalendar.w_year=calendar.w_year-2000;
	sxprotocolpackge.sxcalendar.w_month=calendar.w_month;
	sxprotocolpackge.sxcalendar.w_date=calendar.w_date;
	sxprotocolpackge.sxcalendar.hour=calendar.hour;
	sxprotocolpackge.sxcalendar.min=calendar.min;
	sxprotocolpackge.sxcalendar.sec=calendar.sec;
}





u8 AddSensorIDList(u8 mainid,u16 subid)
{
	u8 res=0;
	if(mainid==SUBSENSOR_C_TEMHUM||mainid==SUBSENSORCO2||mainid==SUBSENSOR_T_TEMHUM)
		{
			res=Is_Error_Senid(mainid,subid);
			if(res==0)
				{
					res=addsubsensenoridlist(mainid,subid);
					if(res==1)
						{
							if(SystemDebug==2)printf("id is error\r\n");
							return 3;      
						}
					
				}
			else
				{
					if(SystemDebug==2)printf("id is exit\r\n");
					return 2;
				}
		}
	else
		{
			if(SystemDebug==2)printf("mainid is error\r\n");
			return 1;
		}
}



void sxsensoridinit(void)
{
	u8 i;
	SXAddListInfo.ListNum=0;
	for(i=0;i<SXIDLISTNUM;i++)
		{
			SXAddListInfo.IDList[i].subid=0;
			SXAddListInfo.IDList[i].mainid=0;
		}
	syssetid_save_para(&SXAddListInfo);
}


u8 Is_Error_Senid(u8 mainid,u16 subid)
{
	u8 i;
	for(i=0;i<SXIDLISTNUM;i++)
		{
			if(SXAddListInfo.IDList[i].subid==subid)
				{
					if(SXAddListInfo.IDList[i].mainid==mainid)return 1;
				}
		}
	return 0;
}



u8 addsubsensenoridlist(u8 mainid,u16 subid)
{
	if(subid!=0)
		{
			SXAddListInfo.IDList[SXAddListInfo.ListNum].mainid=mainid;
			SXAddListInfo.IDList[SXAddListInfo.ListNum].subid=subid;
			SXAddListInfo.ListNum++;
			if(SXAddListInfo.ListNum>=SXIDLISTNUM)SXAddListInfo.ListNum=0;
			syssetid_save_para(&SXAddListInfo);
			if(SystemDebug==2)printf("+setsxsenid ok\r\n");
		}
	else
		{
			return 1;
		}
	return 0;
}

void SXProtocolInitPara(void)
{
	sxprotocolpackge.sxpackets_Send_head[0]=0x7e;
	sxprotocolpackge.sxpackets_Send_head[1]=0x7e;
	sxprotocolpackge.sxpackets_Send_end[0]=0x0d;
	sxprotocolpackge.sxpackets_Send_end[1]=0x0a;

	mymemset(sxdatabuf,0,SINGNMAX);
}

void SXProtocolRspDataInit(u8 *length)
{
	u8 i;
	u16 crcdata;
	SXProtocolBuf[0]=sxprotocolpackge.sxpackets_Send_head[0];
	SXProtocolBuf[1]=sxprotocolpackge.sxpackets_Send_head[1];
	SXProtocolBuf[2]=SXAddListInfo.IDList[CurrtWorkNum].mainid;
	SXProtocolBuf[3]=(SXAddListInfo.IDList[CurrtWorkNum].subid&0xff00)>>8;
	SXProtocolBuf[4]=SXAddListInfo.IDList[CurrtWorkNum].subid&0xff;
	for(i=0;i<3;i++)SXProtocolBuf[5+i]=systemset.ID[i];
	for(i=0;i<2;i++)SXProtocolBuf[8+i]=sxprotocolpackge.passwd[0+i];
	SXProtocolBuf[10]=SXProtocoREPID;
	SXProtocolBuf[11]=SXProtocoUPDATA;
	for(i=0;i<3;i++)SXProtocolBuf[12+i]=0x00;
	SXProtocolBuf[15]=SXProtocoBEDATA;
	SXProtocolBuf[16]=SXProtocoENDDATA;
	crcdata=SXProtoco_CRC16(SXProtocolBuf,17);
	SXProtocolBuf[17]=(crcdata>>8);
	SXProtocolBuf[18]=(crcdata&0xff);
	SXProtocolBuf[19]=sxprotocolpackge.sxpackets_Send_end[0];
	SXProtocolBuf[20]=sxprotocolpackge.sxpackets_Send_end[1];
	*length=21;
}


void SXProtocolCheckDataInit(u8 *length)
{
	u8 i;
	u16 crcdata;
	SXProtocolBuf[0]=sxprotocolpackge.sxpackets_Send_head[0];
	SXProtocolBuf[1]=sxprotocolpackge.sxpackets_Send_head[1];
	SXProtocolBuf[2]=SXAddListInfo.IDList[CurrtWorkNum].mainid;
	SXProtocolBuf[3]=(SXAddListInfo.IDList[CurrtWorkNum].subid&0xff00)>>8;
	SXProtocolBuf[4]=SXAddListInfo.IDList[CurrtWorkNum].subid&0xff;
	for(i=0;i<3;i++)SXProtocolBuf[5+i]=systemset.ID[i];
	for(i=0;i<2;i++)SXProtocolBuf[8+i]=sxprotocolpackge.passwd[0+i];
	SXProtocolBuf[10]=SXProtocoCHECKID;
	SXProtocolBuf[11]=SXProtocoUPDATA;
	for(i=0;i<3;i++)SXProtocolBuf[12+i]=0x00;
	SXProtocolBuf[15]=SXProtocoBEDATA;
	SXProtocolBuf[16]=SXProtocoENQDATA;
	crcdata=SXProtoco_CRC16(SXProtocolBuf,17);
	SXProtocolBuf[17]=(crcdata>>8);
	SXProtocolBuf[18]=(crcdata&0xff);
	SXProtocolBuf[19]=sxprotocolpackge.sxpackets_Send_end[0];
	SXProtocolBuf[20]=sxprotocolpackge.sxpackets_Send_end[1];
	*length=21;
	
}


void SXProtocolSubTimeDataInit(u8 *length)
{
	u8 i;
	u16 crcdata;

	SXProtocolBuf[0]=sxprotocolpackge.sxpackets_Send_head[0];
	SXProtocolBuf[1]=sxprotocolpackge.sxpackets_Send_head[1];
	SXProtocolBuf[2]=SXAddListInfo.IDList[CurrtWorkNum].mainid;
	SXProtocolBuf[3]=(SXAddListInfo.IDList[CurrtWorkNum].subid&0xff00)>>8;
	SXProtocolBuf[4]=SXAddListInfo.IDList[CurrtWorkNum].subid&0xff;
	for(i=0;i<3;i++)SXProtocolBuf[5+i]=systemset.ID[i];
	for(i=0;i<2;i++)SXProtocolBuf[8+i]=sxprotocolpackge.passwd[0+i];
	SXProtocolBuf[10]=SXProtocoTIMEDATA;
	SXProtocolBuf[11]=SXProtocoUPDATA;
	for(i=0;i<3;i++)SXProtocolBuf[12+i]=0x00;
	SXProtocolBuf[15]=SXProtocoBEDATA;
	
	SXProtocolBuf[16]=(systemset.HandInter&0xff00)>>8;
	SXProtocolBuf[17]=systemset.HandInter&0xff;
	SXProtocolBuf[18]=(systemset.ATime[0]&0xff00)>>8;
	SXProtocolBuf[19]=systemset.ATime[0]&0xff;
	SXProtocolBuf[20]=(systemset.ATime[1]&0xff00)>>8;
	SXProtocolBuf[21]=systemset.ATime[1]&0xff;
	SXProtocolBuf[22]=(systemset.ATime[2]&0xff00)>>8;
	SXProtocolBuf[23]=systemset.ATime[2]&0xff;
	
	SXProtocolBuf[24]=SXProtocoENDDATA;

	
	crcdata=SXProtoco_CRC16(SXProtocolBuf,25);
	SXProtocolBuf[25]=(crcdata>>8);
	SXProtocolBuf[26]=(crcdata&0xff);
	SXProtocolBuf[27]=sxprotocolpackge.sxpackets_Send_end[0];
	SXProtocolBuf[28]=sxprotocolpackge.sxpackets_Send_end[1];
	*length=29;
	
}




void SXProtocolSetRTCDataInit(u8 *length)
{
	u8 i;
	u16 crcdata;
	Get_NewRtc();
	SXProtocolBuf[0]=sxprotocolpackge.sxpackets_Send_head[0];
	SXProtocolBuf[1]=sxprotocolpackge.sxpackets_Send_head[1];
	SXProtocolBuf[2]=SXAddListInfo.IDList[CurrtWorkNum].mainid;
	SXProtocolBuf[3]=(SXAddListInfo.IDList[CurrtWorkNum].subid&0xff00)>>8;
	SXProtocolBuf[4]=SXAddListInfo.IDList[CurrtWorkNum].subid&0xff;
	for(i=0;i<3;i++)SXProtocolBuf[5+i]=systemset.ID[i];
	for(i=0;i<2;i++)SXProtocolBuf[8+i]=sxprotocolpackge.passwd[0+i];
	SXProtocolBuf[10]=SXProtocoSETRTC;
	SXProtocolBuf[11]=SXProtocoUPDATA;
	for(i=0;i<3;i++)SXProtocolBuf[12+i]=0x00;
	SXProtocolBuf[15]=SXProtocoBEDATA;
	SXProtocolBuf[16]=Dec2Hex(sxprotocolpackge.sxcalendar.w_year);
	SXProtocolBuf[17]=Dec2Hex(sxprotocolpackge.sxcalendar.w_month);
	SXProtocolBuf[18]=Dec2Hex(sxprotocolpackge.sxcalendar.w_date);
	SXProtocolBuf[19]=Dec2Hex(sxprotocolpackge.sxcalendar.hour);
	SXProtocolBuf[20]=Dec2Hex(sxprotocolpackge.sxcalendar.min);
	SXProtocolBuf[21]=Dec2Hex(sxprotocolpackge.sxcalendar.sec);
	if(sxdownloaddata.sxtimeneedupdate)SXProtocolBuf[22]=SXProtocoENQDATA;
	else	SXProtocolBuf[22]=SXProtocoENDDATA;
	crcdata=SXProtoco_CRC16(SXProtocolBuf,23);
	SXProtocolBuf[23]=(crcdata>>8);
	SXProtocolBuf[24]=(crcdata&0xff);
	SXProtocolBuf[25]=sxprotocolpackge.sxpackets_Send_end[0];
	SXProtocolBuf[26]=sxprotocolpackge.sxpackets_Send_end[1];
	*length=27;
	
}


void CalcsensorData(void)
{
	u8 i,j,res;
	u8 length;
	u8 wingtime=0;
	for(CurrtWorkNum=0;CurrtWorkNum<SXAddListInfo.ListNum;CurrtWorkNum++)
		{
		
			SXProtocolCheckDataInit(&length);
RESENDSXDATA:
			sxsendbuf(SXProtocolBuf,length);
			if(SystemDebug==2)
				{
					printf(">>\r\n");
					for(j=0;j<length;j++)printf("%02X",SXProtocolBuf[j]);
					printf("\r\n");
				}
			res=waitsubsensorack(2000);
			if(res)
				{
					wingtime++;
					if(wingtime>=3)
						{
							wingtime=0;
						}
					else
						{
							goto RESENDSXDATA;
						}
				}
			else
				{
					wingtime=0;
					if(sxdownloaddata.sxdowndatainfo==1)
						{
							delay_ms(3000);
							sxdownloaddata.sxdowndatainfo=0;
							SXProtocolSetRTCDataInit(&length);
							sxsendbuf(SXProtocolBuf,length);
							if(SystemDebug==2)
								{
									printf(">>\r\n");
									for(j=0;j<length;j++)printf("%02X",SXProtocolBuf[j]);
									printf("\r\n");
								}
							delay_ms(3000);
						}
					if(sxdownloaddata.sxtimeneedupdate==1)
						{
							delay_ms(3000);
							SXProtocolSubTimeDataInit(&length);
							sxsendbuf(SXProtocolBuf,length);
							if(SystemDebug==2)
								{
									printf(">>\r\n");
									for(j=0;j<length;j++)printf("%02X",SXProtocolBuf[j]);
									printf("\r\n");
								}
							delay_ms(3000);
						}
				}
		}
	sxdownloaddata.sxtimeneedupdate=0;
}


void RecevisensorData(void)
{
	u8 res,i;
	u8 BufferSize;
	if(systemset.datamode==0)
		{
			if(systeminfo.SystemFlow==0)
				{
					res=Radio->Process();
					if(res==RF_RX_DONE)
					{
						Radio->GetRxPacket(sxdatabuf, ( uint16_t* )&BufferSize );
						if(SystemDebug==2)
							{
								printf("<<\r\n");
								for(i=0;i<BufferSize;i++)printf("%02X",sxdatabuf[i]);
								printf("\r\n");
							}
						res=sxdowndatasubsensoriduse(sxdatabuf);
						if(res)sxdowndataansysall(sxdatabuf);
						else
							{
								if(SystemDebug==2)printf("error sensor id info\r\n");
							}
							mymemset(sxdatabuf,0,BufferSize);
							Radio->StartRx();
						}
			}
						}
	if(systemset.datamode==1)
		{
			RS485ONE_Receive_Data(sxdatabuf,&BufferSize);
			if(BufferSize)
				{
					if(SystemDebug==2)
						{
							printf("<<\r\n");
							for(i=0;i<BufferSize;i++)printf("%02X",sxdatabuf[i]);
							printf("\r\n");
						}
					res=sxdowndatasubsensoriduse(sxdatabuf);
					if(res)sxdowndataansysall(sxdatabuf);
					else
						{	
							if(SystemDebug==2)printf("error sensor id info\r\n");
						}
					mymemset(sxdatabuf,0,BufferSize);
				}
		}
}

