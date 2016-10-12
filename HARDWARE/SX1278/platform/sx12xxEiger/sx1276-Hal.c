/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1276-Hal.c
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
//#include <stdint.h>
//#include <stdbool.h> 

#include "platform.h"

#include "spi.h"

#include "../../radio/sx1276-Hal.h"


#define RESET_IOPORT                                GPIOE
#define RESET_PIN                                   GPIO_Pin_10


#define NSS_IOPORT                                  GPIOE
#define NSS_PIN                                     GPIO_Pin_6  
#define HAL_SS_LOW()				GPIO_ResetBits(NSS_IOPORT,NSS_PIN)
#define HAL_SS_HIGH()				GPIO_SetBits(NSS_IOPORT,NSS_PIN)

#define DIO0_IOPORT                                 GPIOE
#define DIO0_PIN                                    GPIO_Pin_0

#define DIO1_IOPORT                                 GPIOE
#define DIO1_PIN                                    GPIO_Pin_1

#define DIO2_IOPORT                                 GPIOE
#define DIO2_PIN                                    GPIO_Pin_2

#define DIO3_IOPORT                                 GPIOE
#define DIO3_PIN                                    GPIO_Pin_3

#define DIO4_IOPORT                                 GPIOE
#define DIO4_PIN                                    GPIO_Pin_4

#define DIO5_IOPORT                                 GPIOE
#define DIO5_PIN                                    GPIO_Pin_5

#define RXTX_IOPORT                                 
#define RXTX_PIN                                    FEM_CTX_PIN


#define RXE_PORT       			GPIOE
#define RXE_PIN  				GPIO_Pin_8
#define RXE_HIGH()         		GPIO_SetBits(RXE_PORT,RXE_PIN)
#define RXE_LOW()          		GPIO_ResetBits(RXE_PORT,RXE_PIN)
#define RXE_STATE()        		GPIO_ReadOutputDataBit(RXE_PORT,RXE_PIN)

#define TXE_PORT       			GPIOE
#define TXE_PIN  				GPIO_Pin_9
#define TXE_HIGH()         		GPIO_SetBits(TXE_PORT,TXE_PIN)
#define TXE_LOW()          		GPIO_ResetBits(TXE_PORT,TXE_PIN)
#define TXE_STATE()        		GPIO_ReadOutputDataBit(TXE_PORT,TXE_PIN)

void Set_RF_Switch_RX(void)
{
	RXE_HIGH();
	TXE_LOW();
}

void Set_RF_Switch_TX(void)
{
	RXE_LOW();
	TXE_HIGH();
}


void SX1278SLEEP(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	 GPIO_ResetBits(GPIOE,GPIO_Pin_11);
	 delay_ms(100);
	 
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);


   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_6|GPIO_Pin_11;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(GPIOE, &GPIO_InitStructure);
	  
   GPIO_ResetBits(GPIOE,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_6|GPIO_Pin_11);

	 
}

void SX1276InitIo( void )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);


   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_6|GPIO_Pin_11;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(GPIOE, &GPIO_InitStructure);
}

void SX1276SetReset( uint8_t state )
{
     GPIO_ResetBits(GPIOE,GPIO_Pin_11);
     delay_ms(100);
     GPIO_SetBits(GPIOE,GPIO_Pin_11);
     delay_ms(500);

      GPIO_ResetBits( RESET_IOPORT, RESET_PIN );
      delay_ms(10);
       GPIO_SetBits(RESET_IOPORT,RESET_PIN);
       delay_ms(10);
}

void SX1276Write( uint8_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

void SX1276Read( uint8_t addr, uint8_t *data )
{
    SX1276ReadBuffer( addr, data, 1 );
}

void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

     HAL_SS_LOW();
     delay_ms(2);
    SpiInOut( addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }
     delay_ms(2);
     HAL_SS_HIGH();
}

void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

   
   HAL_SS_LOW();
   delay_ms(2);
    SpiInOut( addr & 0x7F );
    for( i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0xff );
    }
   delay_ms(2);
   HAL_SS_HIGH();
}

void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1276WriteBuffer( 0, buffer, size );
}

void SX1276ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1276ReadBuffer( 0, buffer, size );
}

inline uint8_t SX1276ReadDio0( void )
{
    return GPIO_ReadInputDataBit( DIO0_IOPORT, DIO0_PIN );
}

inline uint8_t SX1276ReadDio1( void )
{
    return GPIO_ReadInputDataBit( DIO1_IOPORT, DIO1_PIN );
}

inline uint8_t SX1276ReadDio2( void )
{
    return GPIO_ReadInputDataBit( DIO2_IOPORT, DIO2_PIN );
}

inline uint8_t SX1276ReadDio3( void )
{
    return GPIO_ReadInputDataBit( DIO3_IOPORT, DIO3_PIN );
}

inline uint8_t SX1276ReadDio4( void )
{
    return GPIO_ReadInputDataBit( DIO4_IOPORT, DIO4_PIN );
}

inline uint8_t SX1276ReadDio5( void )
{
    return GPIO_ReadInputDataBit( DIO5_IOPORT, DIO5_PIN );
}


inline void SX1276WriteRxTx( uint8_t txEnable )
{
    if( txEnable != 0 )
    {
		Set_RF_Switch_TX();
    }
    else
    {
		Set_RF_Switch_RX();  
    }
}

