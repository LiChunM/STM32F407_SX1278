#ifndef __PRO_H
#define __PRO_H


#include "sys.h"
#include "stm32f4xx_conf.h"
#include "rtc.h"



#define ProtocoMaxLen  250

#define RECIVELIST	50
#define RECIVEBUF		4


#define SENDBITWAIT	0
#define SENDBITOK		1
#define SENDBITDELAY	2
#define SENDBITNULL	3


#define EVEPACGE		5
#define EVESUBINFO	24


#define SVEREGPACKK	10

#define SVSETRTC		0x4A
#define SVHANDAT		0x3f
#define SVECHECK		0x14
#define SVREDAOK		0x3C
#define SVREGSUBID	0x3D
#define SVREGDLLID	0x3E
#define SVSETTIME		0x13



#define SVSYN			0x17
#define SVSTX			0x12
#define SVESC			0x1B
#define SVEOT			0x04


#define SVWAVEPRO	21
#define SVWAVEREG	22
#define SVWAVEDLLREG	23







typedef struct
{
	u8 packets_Send_head[2];
	u8 centeraddr[4];
	u8 subsensoraddr[2];
	u8 passwd[2];
	u8 packets_Send_end[2];
	u16 packetsNums;
}Protocol_Send_packets;


void DataCore(u8 *databuf,u8 *res);

extern u8 ProtocolBuf[ProtocoMaxLen];
extern vu16 ProtocolFunc;
extern vu8 ProtocoWave;

///////////////////////////////////////////////////////////////////////////


#endif



