#ifndef __RTC_H
#define __RTC_H	 
#include "sys.h" 





typedef struct 
{
	u8 hour;
	u8 min;
	u8 sec;			
	u16 w_year;
	u8  w_month;
	u8  w_date;
	u8  week;	
	}_calendar_obj;

extern _calendar_obj calendar;	//日历结构体
extern u8 TIMESE;
u8 My_RTC_Init(void);						
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm);			//RTC时间设置
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week); 		//RTC日期设置
void RTC_Get_Time(u8 *hour,u8 *min,u8 *sec,u8 *ampm);	//获取RTC时间
void RTC_Get_Date(u8 *year,u8 *month,u8 *date,u8 *week);//获取RTC日期
void RTC_Set_AlarmA(u8 week,u8 hour,u8 min,u8 sec);		//设置闹钟时间(按星期闹铃,24小时制)
void RTC_Set_WakeUp(u32 wksel,u16 cnt);					//周期性唤醒定时器设置
u8 RTC_Get_Week(u16 year,u8 month,u8 day);
void RTC_Set(u16 year,u8 month,u8 date,u8 hour,u8 min,u8 sec);
void calendar_get_time(_calendar_obj *calendarx);
void calendar_get_date(_calendar_obj *calendarx);
void SysTimeSync(u8 *timebuf);
#endif

















