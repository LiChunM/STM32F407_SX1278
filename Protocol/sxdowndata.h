#ifndef __SXDOWNPRO_H
#define __SXDOWNPRO_H


#include "sys.h"
#include "rtc.h"
#include "stm32f4xx_conf.h"



typedef struct
{
	_calendar_obj sxsensorcalendar;
	u8 sxpacketssensorid;
	u8 sxpacketscusid;
	u8 sxdowndatainfo;
	u8 sxtimeneedupdate;
	u8 sxreciveinfo;
	u16 ctem;
	u16 chum;
	u16 clight;
	u16 co2;
	u16 vol;
}SXDowndata;



extern SXDowndata sxdownloaddata;

u8 waitsubsensorack(u16 time);
u8 sxdowndataansysallTest(u8 *sxbuf);

#endif




