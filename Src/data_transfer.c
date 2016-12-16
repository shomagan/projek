#include "data_transfer.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "step.h"
#include "saver.h"
#include "frame_control.h"

u16 add_crc16(u8* pck, u8 len){
  u16 crc;
  crc = crc16(pck,len);
  pck[len] = crc;
  pck[len+1] = crc>>8;
  return crc;
}
u8 check_crc16(u8* pck, u8 len){
  u16 crc1,crc2;
  crc1 = pck[len-2]|((u16)pck[len-1]<<8);
  crc2 = crc16(pck,(len-2));
  if (crc1 == crc2){
    return 1;
  }else{
    return 0;
  }
}
u16 crc16(u8* pck, u8 len){
	u16  result;
	u8 i, j;

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

