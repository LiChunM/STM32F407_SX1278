#ifndef __MC323_H__
#define __MC323_H__	 
#include "sys.h"


extern volatile u8 IPCONNCET;
extern volatile u8 InterCurLine;
extern volatile u8 RtuSendIdentifier;
extern volatile u8 RtuSendFinish;
extern volatile u8 DataSensoCheck;
extern const u8 *modetbl[2];
extern volatile u8 DataBiteRecive;
extern volatile u8 DataStatuRecive;
void m35_recive_data(u8 *mc35data,u8 len);
u8 m35_tcpudp_conncet(u8 mode,u8* ipaddr,u8* port,u8 curline);
void m35_sleep_off(void);
void m35_wakeup_on(void);
void M35SendBufNoAck(u8 *data,u16 length);
void SysIpSync(u8 *ipsyncbuf);
u8 M35SendDataNoAck(u8 *data,u16 length,u8 curline);
u8 M35SendDataCheckOK(u16 waittime);
void ProcessingTheirReply(u8 *RtuReplyBuf);
u8 waitforcenterrecall(u16 sdelaytime);
void SysIpSync(u8 *ipsyncbuf);
u8 WaitCenterSendBakProtocolXT(u16 sdelaytime);
u8 WaitCenterSendBakProtocolRT(u16 sdelaytime);
void M35PowerOn(void);
void M35PowerOff(void);
u8 M35InitForTheData(void);
void DingTimeSendForData(void);
#endif
