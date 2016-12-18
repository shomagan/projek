#include "data_transfer.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "step.h"
#include "saver.h"
#include "frame_control.h"
#include "usbd_cdc_if.h"
char init_send[] = "Guadalajara";
char const init_receive[] = "Jalisco";
char how_time[]  = "how_mach_time";
char set_time[]  = "time";
static u8 usb_send_packet(u8* buff,u16 len);
static void time_answer(void);
static void sync_time(u16* buff);
extern RTC_HandleTypeDef hrtc;

u8 receive_packet_hanling(u8* buff){
  u8 nruter;
  nruter =0;
  if (strncmp((char*)init_receive, (char*)buff,sizeof(init_receive)-1)==0){
    usb_send_packet((u8*)init_send, sizeof(init_send)-1);
  }else if(strncmp((char*)how_time, (char*)buff,sizeof(how_time)-1)==0){
    time_answer();
  }else if(strncmp((char*)set_time, (char*)buff,sizeof(set_time)-1)==0){
    sync_time((u16*)buff);
  }
  return nruter;
}
u8 usb_send_packet(u8* buff,u16 len){
  for(u16 i =0;i<len;i++){
    UserTxBufferFS[i] = buff[i];
  }
  settings.vars.usb_tranceiver_state |= USB_TRANSMIT_PACKET;
  CDC_Transmit_FS((u8*)UserTxBufferFS, len);
  return 0x00;
}
void time_answer(void){
  u16 buff[8];
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;

  if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN)!= HAL_OK){
    sTime.Hours = 0x1;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  }
  
  buff[0] = BKP->DR1 = sTime.Hours;
  buff[1] = BKP->DR2 = sTime.Minutes;
  buff[2] = BKP->DR3 = sTime.Seconds;
  if(HAL_RTC_GetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN)!=HAL_OK){
    DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
    DateToUpdate.Month = RTC_MONTH_JANUARY;
    DateToUpdate.Date = 0x1;
    DateToUpdate.Year = 0x0;
    HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN);
  }
  if (DateToUpdate.WeekDay == 0){
    buff[3] = BKP->DR4  = 6;
  }else{
    buff[3] = BKP->DR4  = DateToUpdate.WeekDay-1 ;
  }

  buff[3] = BKP->DR4  = DateToUpdate.WeekDay ;
  buff[4] = BKP->DR5  = DateToUpdate.Month ;
  buff[5] = BKP->DR6  = DateToUpdate.Date ;
  buff[6] = BKP->DR7  = DateToUpdate.Year ;
  add_crc16((u8*)buff,14);
  usb_send_packet((u8*)buff,16);
}
void sync_time(u16* buff){
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;
  if (check_crc16((u8*)buff, 20)){
    BKP->DR1 = sTime.Hours = buff[2];
    BKP->DR2 = sTime.Minutes = buff[3];
    BKP->DR3 = sTime.Seconds = buff[4];
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    if (buff[5] == 6){
      BKP->DR4  = DateToUpdate.WeekDay = 0;
    }else{
      BKP->DR4  = DateToUpdate.WeekDay = buff[5]+1;
    }
 
    BKP->DR5  = DateToUpdate.Month = buff[6];
    BKP->DR6  = DateToUpdate.Date = buff[7];
    BKP->DR7  = DateToUpdate.Year = buff[8];
    HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN);
    usb_send_packet((u8*)buff,20);
  }else{
    u8 temp_buff[20] = {0};
    usb_send_packet((u8*)temp_buff,20);
  }

}



u16 add_crc16(u8* pck, u16 len){
  u16 crc;
  crc = crc16(pck,len);
  pck[len] = crc;
  pck[len+1] = crc>>8;
  return crc;
}
u8 check_crc16(u8* pck, u16 len){
  u16 crc1,crc2;
  crc1 = pck[len-2]|((u16)pck[len-1]<<8);
  crc2 = crc16(pck,(len-2));
  if (crc1 == crc2){
    return 1;
  }else{
    return 0;
  }
}
u16 crc16(u8* pck, u16 len){
	u16  result;
	u16 i, j;

	result = 0xFFFF;
	for (i = 0; i < len; i++)  {
		result ^= pck[i];
		for (j = 0; j < 8; j++) {
			if ((result & 0x01) == 1)
				result = (result >> 1) ^ 0xA001;
			else
				result >>= 1;
		}
	}
	return result;
}

