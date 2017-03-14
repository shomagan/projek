#include "data_transfer.h"
#include "stm32f1xx_hal.h"
#include "step.h"
#include "saver.h"
#include "frame_control.h"
#include "hw_config.h"
char init_send[] = "Guadalajara";
char const init_receive[] = "Jalisco";
char how_time[]  = "how_mach_time";
char set_time[]  = "time";
#define SETTINGS_PREFIX_SIZE 13
char settings_write[]  = "settings_rite";
char settings_read[]  = "settings_read";
char settings_save[]  = "settings_save";
static u8 usb_send_packet(u8* buff,u16 len);
static void time_answer(void);
static void sync_time(u16* buff);
static void settings_save_data(u8* buff);
static void settings_read_data(u8* buff);
static void settings_write_data(u8* buff);
extern RTC_HandleTypeDef hrtc;

extern __IO uint32_t packet_sent;
extern __IO uint32_t packet_receive;
extern __IO uint8_t Receive_Buffer[64];


u8 receive_packet_hanling(u8* buff){
  u8 nruter,i;
  u8 temp_buff[13];
  nruter =0;
  if (strncmp((char*)init_receive, (char*)buff,sizeof(init_receive)-1)==0){
    for (i=0;i<11;i++){
      temp_buff[i] = init_send[i];
    }
    add_crc16((u8*)temp_buff,11);
    usb_send_packet((u8*)temp_buff,13);
  }else if(strncmp((char*)how_time, (char*)buff,sizeof(how_time)-1)==0){
    time_answer();
  }else if(strncmp((char*)set_time, (char*)buff,sizeof(set_time)-1)==0){
    sync_time((u16*)buff);
  }else if(strncmp((char*)settings_write, (char*)buff,sizeof(settings_write)-1)==0){
    settings_write_data((u8*)buff);
  }else if(strncmp((char*)settings_read, (char*)buff,sizeof(settings_read)-1)==0){
    settings_read_data((u8*)buff);
  }else if(strncmp((char*)settings_save, (char*)buff,sizeof(settings_save)-1)==0){
    settings_save_data((u8*)buff);
  }
  return nruter;
}
u8 usb_send_packet(u8* buff,u16 len){
  if (packet_sent == 1){
    CDC_Send_DATA ((unsigned char*)buff,len);
  }
  settings.vars.usb_tranceiver_state |= USB_TRANSMIT_PACKET;
  return 0x00;
}
void time_answer(void){
  u16 buff[9];
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
  buff[7] = settings.vars.frame_number_saved;
  add_crc16((u8*)buff,16);
  usb_send_packet((u8*)buff,18);
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
void settings_write_data(u8* buff){
  u16 j;
  u8 settings_number;
  u8 buff_answer[256],len_answer;
  for (u8 i=0;i<SETTINGS_PREFIX_SIZE;i++){
    buff_answer[i] = buff[i];
  }
  j = SETTINGS_PREFIX_SIZE;
  settings_number = buff[j];
  j++;
  len_answer = SETTINGS_PREFIX_SIZE;
  if(check_crc16(buff,SETTINGS_PREFIX_SIZE+settings_number*4+1+2)){
    for (u16 i=0;i<settings_number;i++){
      u8 index,param_led_transit;
      u16 param_time;
      index = buff[j];
      j++;
      param_led_transit = buff[j];
      j++;
      param_time = ((u16)(buff[j])&0x00ff)|(((u16)(buff[j+1])<<8)&0xff00);
      j+=2;
      if(index==UP_TIME_INDEX){
        settings.vars.up_time.hour = param_time&0x00ff;
        settings.vars.up_time.min = (param_time>>8)&0x00ff;
      }else if(index==DOWN_TIME_INDEX){
        settings.vars.down_time.hour = param_time&0x00ff;
        settings.vars.down_time.min = (param_time>>8)&0x00ff;
      }else if(index < UP_TIME_INDEX){
        settings.vars.frame[index].time = param_time;
        if(param_led_transit==1){
          settings.vars.frame[index].option |= ENABLE_LED;
        }else{
          settings.vars.frame[index].option &= ~ENABLE_LED;
        }
      }
      buff_answer[len_answer] = index;
      len_answer++;
    }
  }else{
    len_answer=0;
    for (u8 i=0;i<SETTINGS_PREFIX_SIZE+settings_number*4+1+2;i++){
      buff_answer[i] = buff[i];
      len_answer++;
    }
    
  }
  add_crc16((u8*)buff_answer,len_answer);
  usb_send_packet((u8*)buff_answer,len_answer+2);
}
void settings_read_data(u8* buff){
  u16 j;
  u8 settings_number;
  u8 buff_answer[256],len_answer;
  u8 index;
  for (u8 i=0;i<SETTINGS_PREFIX_SIZE;i++){
    buff_answer[i] = buff[i];
  }
  j = SETTINGS_PREFIX_SIZE;
  settings_number = buff[j];
  j++;
  index = buff[j];
  j++;
  len_answer = SETTINGS_PREFIX_SIZE;
  for (u16 i=0;i<settings_number;i++){
    u8 param_led_transit;
    u16 param_time;
    buff_answer[len_answer] = index+i;
    len_answer++;
    if((index+i)==UP_TIME_INDEX){
      param_led_transit = 0;
      param_time = ((u16)(settings.vars.up_time.hour)&0x00ff)|(((u16)(settings.vars.up_time.min)<<8)&0xff00);
    }else if((index+i)==DOWN_TIME_INDEX){
      param_led_transit = 0;
      param_time = ((u16)(settings.vars.down_time.hour)&0x00ff)|(((u16)(settings.vars.down_time.min)<<8)&0xff00);
    }else if((index+i) < UP_TIME_INDEX){
      param_time = settings.vars.frame[index+i].time;
      if(settings.vars.frame[index+i].option & ENABLE_LED){
        param_led_transit =1;
      }else{
        param_led_transit =0;
      }
    }
    buff_answer[len_answer] = param_led_transit ;
    len_answer++;
    buff_answer[len_answer] = param_time;
    len_answer++;
    buff_answer[len_answer] = param_time>>8;
    len_answer++;
  }
  add_crc16((u8*)buff_answer,len_answer);
  usb_send_packet((u8*)buff_answer,len_answer+2);
}

void settings_save_data(u8* buff){
  u8 buff_answer[256],len_answer;
  for (u8 i=0;i<SETTINGS_PREFIX_SIZE;i++){
    buff_answer[i] = buff[i];
  }
  len_answer = SETTINGS_PREFIX_SIZE;
  add_crc16((u8*)buff_answer,len_answer);
  rewrite_page();
  usb_send_packet((u8*)buff_answer,len_answer+2);
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

