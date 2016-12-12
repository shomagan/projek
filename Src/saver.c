#include "saver.h"


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


