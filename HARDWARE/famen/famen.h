#ifndef __FAMEN_H
#define __FAMEN_H



#include "sys.h"
#include "stm32f4xx_conf.h"
#include "rtc.h"


#define FMLISTNUM 	30
#define IDTABLE		4

#define FAMENCHAGECOM	0xEE
#define FAMENSETCOM		0x22
#define FAMENONOFFCOM	0x23
#define FAMENONDATA		0x01
#define FAMEOFFNDATA		0x00
#define FAMENROOTSU		0xff



typedef struct   
{
	u8 efamenid[IDTABLE];
}famenID;


typedef struct   
{
	u8 Func[4];
	u8 ListNum;
	famenID IDList[FMLISTNUM];
}FMIDList;


extern FMIDList	FaMenInfo;
extern u8 FaMenDataBuf[28];


u8 Add2FamenList(u8 *idbuf);
void FaMenUart(u8 *famenbuf);
void FMIDListInit(void);
#endif

