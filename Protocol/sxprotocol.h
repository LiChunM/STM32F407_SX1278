#ifndef __SXPRO_H
#define __SXPRO_H


#include "sys.h"
#include "stm32f4xx_conf.h"
#include "rtc.h"

#define SXIDLISTNUM			100
#define SXProtocoMaxLen 		64
#define SXProtocoMastData		0xA0	   //自报数据功能码
#define SXProtocoREPID			0x02    //统一回复功能码
#define SXProtocoCHECKID		0x03	   // 查询数据功能码
#define SXProtocoSETRTC		0x11	   // 设置时间功能码
#define SXProtocoUPDATA		0x08    //上行数据标志
#define SXProtocoDOWNDATA	0x10    // 下行数据标志
#define SXProtocoBEDATA		0x12    //  报文起始符
#define SXProtocoENDDATA		0x04	  //报文结束
#define SXProtocoENQDATA		0x05   // 后续有报文
#define SXProtocoHANDATA		0x1A	//捂手数据功能码
#define SXProtocoTIMEDATA		0x1B	//设置定时上报时间



#define SUBSENSOR_C_TEMHUM		0x10   //空气温湿度光照
#define SUBSENSORCO2				0x11	 //空气CO2
#define SUBSENSOR_T_TEMHUM		0x12  //土壤温湿度



extern u8 SXProtocolBuf[SXProtocoMaxLen];


typedef struct
{
	u8 sxpacketspreaddr[3];
	u8 sxpacketssensorid[3];
}SXProtocolAddr;


typedef struct 
{
	u8 hour;
	u8 min;
	u8 sec;			
	u16 w_year;
	u8  w_month;
	u8  w_date;
	u8  week;	
}_mycalendar_obj;


typedef struct
{
	u16 vola;
	u16 ctem;
	u16 chum;
	u16 clight;
	u16 conum;
	_mycalendar_obj sxsensorcalendar;	
}CHTINFO;



typedef struct   
{
	u8 mainid;
	u16 subid;
	CHTINFO Sensordata;
}SXProtocolSenID;


typedef struct   
{
	u8 ListNum;
	SXProtocolSenID IDList[SXIDLISTNUM];
}SXSenIDList;




typedef struct
{
	u8 sxpackets_Send_head[2];
	u8 sxpackets_Send_end[2];
	u8 passwd[2];
	u32 packetsNums;
	SXProtocolAddr sxrotocoladdr;
	_mycalendar_obj sxcalendar;
}SXProtocol_Send_packets;

extern SXSenIDList SXAddListInfo;
extern SXProtocol_Send_packets sxprotocolpackge;

extern volatile u8 CurrtWorkNum;

void SXProtocolInitPara(void);
void SXProtocolRspDataInit(u8 *length);
void SXProtocolCheckDataInit(u8 *length);
void SXProtocolSetRTCDataInit(u8 *length);
void sxsensoridinit(void);
u8 Is_Error_Senid(u8 mainid,u16 subid);
u8 addsubsensenoridlist(u8 mainid,u16 subid);
void CalcsensorData(void);
void RecevisensorData(void);
u8 AddSensorIDList(u8 mainid,u16 subid);
void LoadSenidFlash(void);
#endif



