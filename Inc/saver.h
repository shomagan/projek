#include "stm32f1xx_hal.h"
typedef enum 
{
  STATUS_OK       = 0x00,
  STATUS_ERROR    = 0x01,
  STATUS_BUSY     = 0x02,
  STATUS_TIMEOUT  = 0x03
} StatusTypeDef;

#define FLASH_OFSSET 0x08000000
#define PAGE_FOR_WRITE 63
#define FLASH_SIZE_PAGE 1024
#define WRITABLE_FLASH_PAGE (FLASH_OFSSET + (PAGE_FOR_WRITE * FLASH_SIZE_PAGE))
/* Flash Control Register bits */
#define CR_PG_Set                ((uint32_t)0x00000001)
#define CR_PG_Reset              ((uint32_t)0x00001FFE) 
#define CR_PER_Set               ((uint32_t)0x00000002)
#define CR_PER_Reset             ((uint32_t)0x00001FFD)
#define CR_MER_Set               ((uint32_t)0x00000004)
#define CR_MER_Reset             ((uint32_t)0x00001FFB)
#define CR_OPTPG_Set             ((uint32_t)0x00000010)
#define CR_OPTPG_Reset           ((uint32_t)0x00001FEF)
#define CR_OPTER_Set             ((uint32_t)0x00000020)
#define CR_OPTER_Reset           ((uint32_t)0x00001FDF)
#define CR_STRT_Set              ((uint32_t)0x00000040)
#define CR_LOCK_Set              ((uint32_t)0x00000080)

StatusTypeDef FLASH_ErasePage(uint32_t Page_Address);
StatusTypeDef flash_program_u16(uint32_t Address, uint16_t Data);
u8 init_frame_struct(u16 frame_number);
u8 rewrite_page(void);
