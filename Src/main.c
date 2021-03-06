/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "step.h"
#include "saver.h"
#include "frame_control.h"
#include "data_transfer.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"


ADC_HandleTypeDef hadc2;

IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;

u32 time_ms;
u8 config;
RTC_TimeTypeDef Time;
RTC_DateTypeDef Date;
settings_t settings;
u8 buff_temp[256];
u32 lenta;
u32 speed_control;
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC2_Init(void);
static void MX_USART1_UART_Init(void);
static u8 work_time(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
extern  uint8_t Receive_Buffer[64];
extern  uint32_t Receive_length ;
extern  uint32_t length ;
uint8_t Send_Buffer[64];
uint32_t packet_sent=1;
uint32_t packet_receive=1;
u16 time_for_state_memory_left;
u16 time_for_state_memory_midle;


/* USER CODE END 0 */

int main(void){
  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_IWDG_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  USB_Interrupts_Config();
  USB_Init();

  MX_ADC2_Init();
  MX_USART1_UART_Init();
  init_frame_struct(0);

  HAL_GPIO_WritePin(GPIOA,BIT(5),GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA,BIT(4),GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB,BIT(0),GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB,BIT(1),GPIO_PIN_SET);

  time_ms = uwTick;
  config = 0;
  HAL_IWDG_Start(&hiwdg);
  motor_init(GPIOB,8,9,12,&motor_two);
  motor_init(GPIOB,10,11,13,&motor_one);
  frame_init();
  u32 timer;
  timer = uwTick;
  speed_control = uwTick;
  lenta = 1;
  while (1){
    HAL_IWDG_Refresh(&hiwdg);
    if (config & STEP_TIME){
      config &=~STEP_TIME;
      if (settings.vars.state!=STOPED_TIME){
        frame_control_hadler();
      }
      step_motor_control(&motor_one);
      step_motor_control(&motor_two);
      if(time_for_state_memory_left){
        time_for_state_memory_left--;
      }else{
        settings.vars.init_state &= ~DID_LEFT_OPT;
      }
      if(time_for_state_memory_midle){
        time_for_state_memory_midle--;
      }else{
        settings.vars.init_state &= ~DID_MIDLE_OPT;
      }

    }

    if(uwTick>(timer+1000)){
      timer = uwTick;
    }

    if (bDeviceState == CONFIGURED){
      CDC_Receive_DATA();
      /*Check to see if we have data yet */
      if (Receive_length  != 0){
        receive_packet_hanling(Receive_Buffer);
/*        if (packet_sent == 1){
          CDC_Send_DATA ((unsigned char*)Receive_Buffer,Receive_length);
        }*/
        Receive_length = 0;
      }
    }
/*    if (settings.vars.usb_tranceiver_state & USB_RECIVE_OR_TRANSMIT_PACKET){
      settings.vars.usb_tranceiver_state &= ~USB_RECIVE_OR_TRANSMIT_PACKET;
      receive_packet_hanling(UserRxBufferFS);
    }*/


    if (config & SECOND){
      config &= ~SECOND;
      if ((work_time()==0) && (settings.vars.state!=STOPED_TIME)){
        settings.vars.state=STOPED_TIME;
        stop_rotate(&motor_one);
        stop_rotate(&motor_two);
        disable_led();
      }else if(work_time()&&(settings.vars.state==STOPED_TIME)){
        break_to_init();
      }
    }
    HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BCD);
  }
}

/** System Clock Configuration
*/
void SystemClock_Config(void){

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
															|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK){
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC2 init function */
static void MX_ADC2_Init(void){

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK){
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK){
    Error_Handler();
  }

}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }

}

/* RTC init function */
static void MX_RTC_Init(void){
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK){
    Error_Handler();
  }
    /**Initialize RTC and set the Time and Date 
    */
  if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD)!= HAL_OK){
    sTime.Hours = 0x1;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK){
      Error_Handler();
    }
  }
  BKP->DR1 = sTime.Hours;
  BKP->DR2 = sTime.Minutes;
  BKP->DR3 = sTime.Seconds;
  

  if(HAL_RTC_GetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD)!=HAL_OK){
    DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
    DateToUpdate.Month = RTC_MONTH_JANUARY;
    DateToUpdate.Date = 0x1;
    DateToUpdate.Year = 0x0;
    if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler();
    }
  }
  BKP->DR4  = DateToUpdate.WeekDay ;
  BKP->DR5  = DateToUpdate.Month ;
  BKP->DR6  = DateToUpdate.Date ;
  BKP->DR7  = DateToUpdate.Year ;
  

}

/* TIM1 init function */
static void MX_TIM1_Init(void){

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK){
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK){
    Error_Handler();
  }

  if (HAL_TIM_OC_Init(&htim1) != HAL_OK){
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK){
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    Error_Handler();
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK){
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim1);

}

/* USART1 init function */
static void MX_USART1_UART_Init(void){

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK){
    Error_Handler();
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA5,PA6,PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /*Configure GPIO pins : PB8 dir motor 2
                          PB9 step motor 2
                          PB12 enable low is active
                          PB10 dir motor 1 
                          PB11 step motor 1
                          PB13 enable low is active
                          PB14 pin reset 
                          */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  /*Configure GPIO pins : Pa4 led enable */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, BIT(8)|BIT(9)|BIT(10)|BIT(11), GPIO_PIN_RESET);//dir step 
  HAL_GPIO_WritePin(GPIOB, BIT(12)|BIT(13), GPIO_PIN_RESET);//enable is active
  HAL_GPIO_WritePin(GPIOB, BIT(14), GPIO_PIN_SET);//pin reset disable
  HAL_GPIO_WritePin(GPIOA, BIT(4), GPIO_PIN_SET);//led disable
}

/* USER CODE BEGIN 4 */
u8 work_time(void){
  RTC_TimeTypeDef sTime;
  u16 min_up,min_down,min_current;
  min_up = settings.vars.up_time.hour*60+settings.vars.up_time.min;
  min_down = settings.vars.down_time.hour*60+settings.vars.down_time.min;
  if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN)== HAL_OK){
    min_current = sTime.Hours*60+sTime.Minutes;
  }else{
    min_current = 1442;
  }
  if ((min_up>1440)||(min_down>1440)||(min_current>1440)){
    return 1;
  }else{
    if (min_up>min_down){
      if ((min_current >= min_up)||(min_current < min_down)){
        return 1;
      }else{
        return 0;
      }
    }else if(min_up<min_down){
      if ((min_current >= min_up)&&(min_current < min_down)){
        return 1;
      }else{
        return 0;
      }
    }else{
      return 1;
    }
  }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  BKP->DR7++;
  while(1){
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  BKP->DR8++;
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
