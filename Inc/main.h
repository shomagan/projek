/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
#define TIME_MS (1<<0)
#define BIT(x)  (1<<x)
#define OPT_ONE GPIO_PIN_5
#define OPT_TWO GPIO_PIN_6
#define OPT_THREE GPIO_PIN_7

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef unsigned const char uc8;
extern u32 time_ms;
typedef struct __attribute__((packed)){
  u8 hour;
  u8 min;
}time_hm;

typedef struct __attribute__((packed)){
  u16 time;
  u16  option;
  u32 reserve;
}frame_settings;
#define ENABLE_LED BIT(0)
typedef union{
	struct __attribute__((packed)){
    u16 state;
#define INIT_STATE 1    
#define WORK_STATE 2    
#define NO_STATE   3    
    u16 init_state;
#define SEARCH_START  BIT(0)
#define CHECK_FRAME  BIT(1)
#define START_POSITON  BIT(2)
#define STARTED  BIT(3)
#define END_POSITON  BIT(4)
#define STRETCH      BIT(5)
    u16 frame_finded;
    u16 stop_time;  //current time in sec
    u8 usb_tranceiver_state;
#define USB_RECIVE_OR_TRANSMIT_PACKET BIT(0)  //set in irq when receive or transmith data
#define USB_TRANSMIT_PACKET BIT(1)  //set in irq when receive or transmith data
    u8 move_state;
#define MOVE_TO_RIGHT 0
#define MOVE_TO_LEFT  1
#define STOPED        2
    u16 frame_number_saved;
    time_hm up_time;  //index 118
#define UP_TIME_INDEX 118
    time_hm down_time;  //index 119
#define DOWN_TIME_INDEX 119
    frame_settings frame[118];//118*8 = 944
    u16 crc16;
  } vars;
	u8 Bytes[1024];
  u16 Words[512];
}  settings_t;
extern settings_t settings;
extern u8 buff_temp[256];
extern u32 lenta;
extern u8 config;
/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
