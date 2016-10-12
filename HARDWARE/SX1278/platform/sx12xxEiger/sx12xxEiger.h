#ifndef __SX12XXEIGER_H__
#define __SX12XXEIGER_H__

#include <stdint.h>
#include <stdbool.h>



#define BACKUP_REG_BOOTLOADER                       BKP_DR1          /* Booloader enter*/
#define FW_VERSION                                  "2.0.B2"
#define SK_NAME                                     "SX12xxEiger"

typedef enum
{
    SX_OK,
    SX_ERROR,
    SX_BUSY,
    SX_EMPTY,
    SX_DONE,
    SX_TIMEOUT,
    SX_UNSUPPORTED,
    SX_WAIT,
    SX_CLOSE,
    SX_YES,
    SX_NO,          
}tReturnCodes;

extern volatile uint32_t TickCounter;

void TIM3_Int_Init(u16 arr,u16 psc);

/**
  * @brief   Small printf for GCC/RAISONANCE
  */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

#endif /* __GNUC__ */


#endif // __SX12XXEIGER_H__
