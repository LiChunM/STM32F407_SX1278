

#include "delay.h"  
#include "usart.h"   

#include "platform.h"
#include "radio.h"

#include "sx1276.h"
#include "sx12xxEiger.h"

#include "sx1276-Hal.h"
#include "sx1276-Fsk.h"
#include "sx1276-LoRa.h"

uint8_t SX1276Regs[0x70];

static bool LoRaOn = false;
static bool LoRaOnState = false;


 _sx1278_setings sx1278data;


void LoadSxSysPa(void)
{
	LoRaSettings.RFFrequency=(sx1278data.modulepata.rffrequency*1000000);
	LoRaSettings.SignalBw=sx1278data.modulepata.rfbw;
	LoRaSettings.Power=sx1278data.modulepata.rfpower;
	LoRaSettings.SpreadingFactor=sx1278data.modulepata.rffactor;
	LoRaSettings.ErrorCoding=sx1278data.modulepata.errorcode;
	LoRaSettings.CrcOn=1;
	LoRaSettings.ImplicitHeaderOn=0;
	LoRaSettings.RxSingleOn=0;
	LoRaSettings.FreqHopOn=0;
	LoRaSettings.HopPeriod=4;
	LoRaSettings.TxPacketTimeout=100;
	LoRaSettings.RxPacketTimeout=100;
	LoRaSettings.PayloadLength=128;
	
}




void SX1276Init( void )
{
	uint8_t TempReg;
	
    SX1276 = ( tSX1276* )SX1276Regs;
    SX1276LR = ( tSX1276LR* )SX1276Regs;

   
    SX1276InitIo( );
    TickCounter=0;
    TIM3_Int_Init(10-1,8400-1);
    SX1276Reset( );
	
    SX1276Read(0x06,&TempReg);
    while(TempReg != 0x6C)
	 {
		  printf("TempReg = %02X\r\n",TempReg);
		  delay_ms(100);
		  SX1276Read(0x06,&TempReg);
	 }

	LoadSxSysPa();
		
#if ( LORA == 0 )         

    LoRaOn = false;
    SX1276SetLoRaOn( LoRaOn );
    SX1276FskInit( );

#else

    LoRaOn = true;   
    SX1276SetLoRaOn( LoRaOn );
    SX1276LoRaInit( );
    
#endif


}

void SX1276Reset( void )
{
    SX1276SetReset( RADIO_RESET_ON );
}

void SX1276SetLoRaOn( bool enable )
{
    if( LoRaOnState == enable )
    {
        return;
    }
    LoRaOnState = enable;
    LoRaOn = enable;

    if( LoRaOn == true )
    {
        SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
        
        SX1276LR->RegOpMode = ( SX1276LR->RegOpMode & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_ON;
        SX1276Write( REG_LR_OPMODE, SX1276LR->RegOpMode );
        
        SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
                                        // RxDone               RxTimeout                   FhssChangeChannel           CadDone
        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
                                        // CadDetected          ModeReady
        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
        SX1276WriteBuffer( REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2 );
        
        SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
    }
    else
    {
        SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
        
        SX1276LR->RegOpMode = ( SX1276LR->RegOpMode & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_OFF;
        SX1276Write( REG_LR_OPMODE, SX1276LR->RegOpMode );
        
        SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
        
        SX1276ReadBuffer( REG_OPMODE, SX1276Regs + 1, 0x70 - 1 );
    }
}

bool SX1276GetLoRaOn( void )
{
    return LoRaOn;
}

void SX1276SetOpMode( uint8_t opMode )
{
    if( LoRaOn == false )
    {
        SX1276FskSetOpMode( opMode );
    }
    else
    {
        SX1276LoRaSetOpMode( opMode );
    }
}

uint8_t SX1276GetOpMode( void )
{
    if( LoRaOn == false )
    {
        return SX1276FskGetOpMode( );
    }
    else
    {
        return SX1276LoRaGetOpMode( );
    }
}

double SX1276ReadRssi( void )
{
    if( LoRaOn == false )
    {
        return SX1276FskReadRssi( );
    }
    else
    {
        return SX1276LoRaReadRssi( );
    }
}

uint8_t SX1276ReadRxGain( void )
{
    if( LoRaOn == false )
    {
        return SX1276FskReadRxGain( );
    }
    else
    {
        return SX1276LoRaReadRxGain( );
    }
}

uint8_t SX1276GetPacketRxGain( void )
{
    if( LoRaOn == false )
    {
        return SX1276FskGetPacketRxGain(  );
    }
    else
    {
        return SX1276LoRaGetPacketRxGain(  );
    }
}

int8_t SX1276GetPacketSnr( void )
{
    if( LoRaOn == false )
    {
         while( 1 )
         {
            
         }
    }
    else
    {
        return SX1276LoRaGetPacketSnr(  );
    }
}

double SX1276GetPacketRssi( void )
{
    if( LoRaOn == false )
    {
        return SX1276FskGetPacketRssi(  );
    }
    else
    {
        return SX1276LoRaGetPacketRssi( );
    }
}

uint32_t SX1276GetPacketAfc( void )
{
    if( LoRaOn == false )
    {
        return SX1276FskGetPacketAfc(  );
    }
    else
    {
         while( 1 )
         {
             
         }
    }
}

void SX1276StartRx( void )
{
    if( LoRaOn == false )
    {
        SX1276FskSetRFState( RF_STATE_RX_INIT );
    }
    else
    {
        SX1276LoRaSetRFState( RFLR_STATE_RX_INIT );    
    }
}

void SX1276GetRxPacket( void *buffer, uint16_t *size )
{
    if( LoRaOn == false )
    {
        SX1276FskGetRxPacket( buffer, size );
    }
    else
    {
        SX1276LoRaGetRxPacket( buffer, size );
    }
}

void SX1276SetTxPacket( const void *buffer, uint16_t size )
{
    if( LoRaOn == false )
    {
        SX1276FskSetTxPacket( buffer, size );
    }
    else
    {
        SX1276LoRaSetTxPacket( buffer, size );
    }
}

uint8_t SX1276GetRFState( void )
{
    if( LoRaOn == false )
    {
        return SX1276FskGetRFState( );
    }
    else
    {
        return SX1276LoRaGetRFState( );
    }
}

void SX1276SetRFState( uint8_t state )
{
    if( LoRaOn == false )
    {
        SX1276FskSetRFState( state );
    }
    else
    {
        SX1276LoRaSetRFState( state );
    }
}

uint32_t SX1276Process( void )
{
    if( LoRaOn == false )
    {
        return SX1276FskProcess( );
    }
    else
    {
        return SX1276LoRaProcess( );
    }
}



void SX1276UsartAys(u8 *buf)
{
	u8 *pa,res=0;
	u32 idall=0;
	u32 value=0;
	u8 mybuf[10];
	u8 tempbuf[20]={0};
	pa=(u8*)strstr((const char *)buf,"AT++");
	if(pa!=NULL)
		{
			printf("\r\n");
			printf("AT+SXBDATE\r\n");
			printf("AT+SXPAR\r\n");
			printf("AT+SXRFRFE\r\n");
			printf("AT+SXRFMODE\r\n");
			printf("AT+SXRFFAC\r\n");
			printf("AT+SXRFBW\r\n");
			printf("AT+SXCTID\r\n");
			printf("AT+SXNETID\r\n");
			printf("AT+SXPOWER\r\n");
			printf("\r\n+OK\r\n");
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXBDATE");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXBDATE?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXBDATE:%d\r\n",sx1278data.modulepata.bdate);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXBDATE=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_db(value);
					if(res)
						{
							sx1278data.modulepata.bdate=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXPAR");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXPAR?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXPAR:%d\r\n",sx1278data.modulepata.parity);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXPAR=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_pt(value);
					if(res)
						{
							sx1278data.modulepata.parity=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFRFE");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFRFE?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFRFE:%d\r\n",sx1278data.modulepata.rffrequency);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFRFE=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_fre(value);
					if(res)
						{
							sx1278data.modulepata.rffrequency=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFMODE");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFMODE?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFMODE:%d\r\n",sx1278data.modulepata.rfmode);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFMODE=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_mode(value);
					if(res)
						{
							sx1278data.modulepata.rfmode=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFFAC");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFFAC?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFFAC:%d\r\n",sx1278data.modulepata.rffactor);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFFAC=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_fac(value);
					if(res)
						{
							sx1278data.modulepata.rffactor=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFBW");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFBW?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFBW:%d\r\n",sx1278data.modulepata.rfbw);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFBW=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_bw(value);
					if(res)
						{
							sx1278data.modulepata.rfbw=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFPOWER");
	if(pa!=NULL)
		{
			pa=(u8*)strstr((const char *)buf,"AT+SXRFPOWER?");
			if(pa!=NULL)
				{
					printf("\r\n");
					printf("+SXRFPOWER:%d\r\n",sx1278data.modulepata.rfpower);
					printf("\r\n+OK\r\n");
				}
			pa=(u8*)strstr((const char *)buf,"AT+SXRFPOWER=");
			if(pa!=NULL)
				{
					Get_Str_Use2(tempbuf,buf);
					value=strtol((const char*)tempbuf,NULL,10);
					res=Is_Error_rfpower(value);
					if(res)
						{
							sx1278data.modulepata.rfpower=value;
							sx1278data_save_para(&sx1278data);
							printf("\r\n");
							printf("\r\n+OK\r\n");
						}
					else
						{
							printf("\r\n");
							printf("\r\n+ERROR\r\n");
						}
				}
		}
	pa=(u8*)strstr((const char *)buf,"AT+SXRFCSQ?");
	if(pa!=NULL)
		{
			printf("\r\n");
			printf("+SXRFCSQ:-%d\r\n",SX1276LoRaReadRssiChar());
			printf("\r\n+OK\r\n");
		}
}