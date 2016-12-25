#include "saver.h"
#include "data_transfer.h"


HAL_StatusTypeDef save_settings(){
    HAL_FLASH_OB_Unlock();
    HAL_FLASH_Unlock();
    return FLASH_ErasePage(WRITABLE_FLASH_PAGE);
}

HAL_StatusTypeDef FLASH_ErasePage(uint32_t Page_Address){
  HAL_StatusTypeDef status = HAL_OK;
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Page_Address));

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  
  if(status == HAL_OK){ 
    /* if the previous operation is completed, proceed to erase the page */
    FLASH->CR|= CR_PER_Set;
    FLASH->AR = Page_Address; 
    FLASH->CR|= CR_STRT_Set;
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
    /* Disable the PER Bit */
    FLASH->CR &= CR_PER_Reset;
  }

  /* Return the Erase Status */
  return status;
}
HAL_StatusTypeDef flash_program_u16(uint32_t Address, uint16_t Data){
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tmp = 0;
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Address));
  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  if(status == HAL_OK){
    /* if the previous operation is completed, proceed to program the new first 
    half word */
    FLASH->CR |= CR_PG_Set;
    *(__IO uint16_t*)Address = (uint16_t)Data;
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
    FLASH->CR &= CR_PG_Reset;
  }         
  /* Return the Program Status */
  return status;
}
u8 init_frame_struct(u16 frame_number){
  u16* p_struct;
  u16* p_flash;
  p_struct = (u16*)&settings.vars.frame_number_saved;
  for (u16 i =0;i<476;i++){
    p_flash = (u16*)(WRITABLE_FLASH_PAGE+i*2);
    p_struct[i] = *p_flash;
  }
  if (!check_crc16((u8*)p_struct,952)){
    settings.vars.frame_number_saved = 0;
    settings.vars.down_time.hour=25;
    settings.vars.up_time.hour = 25;
    settings.vars.down_time.min = 0;
    settings.vars.up_time.min  = 0;

    for (u16 i=0;i<118;i++){
      if (i<frame_number){
        settings.vars.frame[i].option = 0;
        settings.vars.frame[i].time = 15;
        settings.vars.frame[i].reserve = 0;
      }else{
        settings.vars.frame[i].option = 0;
        settings.vars.frame[i].time = 15;
        settings.vars.frame[i].reserve = 0;
      }
    }
    rewrite_page();
  }
  if (settings.vars.frame_number_saved != frame_number){
    settings.vars.frame_number_saved = frame_number;
    rewrite_page();
  }
}
u8 rewrite_page(){
    u16* p_struct;
    p_struct = (u16*)&settings.vars.frame_number_saved;
    settings.vars.crc16 = crc16((u8*)p_struct,950);
    HAL_FLASH_OB_Unlock();
    HAL_FLASH_Unlock();
    FLASH_ErasePage(WRITABLE_FLASH_PAGE);
    for (u16 i =0;i<476;i++){
      flash_program_u16(WRITABLE_FLASH_PAGE+i*2, p_struct[i]);
    }
}
/*
       (settings.vars.down_time.hour>25)||
       (settings.vars.up_time.hour>25)||
       (settings.vars.down_time.min>59)||
       (settings.vars.up_time.min>59)||
*/
