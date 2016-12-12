#include "frame_control.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "step.h"
#include "saver.h"
/*#define OPT_ONE GPIO_PIN_5
#define OPT_TWO GPIO_PIN_6
#define OPT_THREE GPIO_PIN_7
*/
static u8 opt_old;
  
u8 frame_init(void){
  settings.vars.state = INIT_STATE;
  settings.vars.init_state = SEARCH_START;
  settings.vars.frame_finded = 0;
  settings.vars.stop_time =0;
  opt_old = 0;
  
}
u8 frame_control_hadler(void){
  static u32 time_stoped;
  u8 opt;
  opt = get_opt_mask();
  if (!settings.vars.stop_time){
    switch(settings.vars.state){
        case INIT_STATE:
          if (settings.vars.init_state & SEARCH_START){
            if (settings.vars.init_state & STARTED){
              if (opt == OPT_FULL_CAPE){
                settings.vars.init_state &= ~SEARCH_START;
                settings.vars.init_state &= ~STARTED;
                settings.vars.init_state |= CHECK_FRAME;
                settings.vars.init_state |= START_POSITON;
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                stop_rotate(&motor_two);
                stop_rotate(&motor_one);
                settings.vars.frame_finded = 0;
              }else if ((opt&0x02)&&((opt_old&0x02)==0)){
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                stop_rotate(&motor_two);
                start_rotate(0,800,&motor_one);
              }
            }else{
              settings.vars.init_state |= STARTED;
              stop_rotate(&motor_two);
              start_rotate(0,800,&motor_one);
            }
          }else if (settings.vars.init_state & CHECK_FRAME){
            if (settings.vars.init_state & STARTED){
              if ((opt == OPT_FULL_CAPE)&&(opt_old != OPT_FULL_CAPE)){
                settings.vars.init_state &= ~CHECK_FRAME;
                settings.vars.init_state &= ~START_POSITON;
                settings.vars.init_state &= ~STARTED;
                settings.vars.init_state |= END_POSITON;
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                stop_rotate(&motor_two);
                stop_rotate(&motor_one);
                settings.vars.state = WORK_STATE;
              }else if ((opt&0x02)&&((opt_old&0x02)==0)){
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                stop_rotate(&motor_one);
                start_rotate(1,800,&motor_two);
                settings.vars.frame_finded++;
              }
            }else{
              settings.vars.init_state |= STARTED;
              stop_rotate(&motor_one);
              start_rotate(1,800,&motor_two);
            }
          }
          break;
        case WORK_STATE:
            if (settings.vars.init_state & STARTED){
              if ((opt == OPT_FULL_CAPE)&&(opt_old != OPT_FULL_CAPE)){
                if (settings.vars.init_state & END_POSITON){
                  settings.vars.init_state |= START_POSITON;
                  settings.vars.init_state &= ~END_POSITON;
                  stop_rotate(&motor_one);
                  start_rotate(1,800,&motor_two);
                }else{
                  settings.vars.init_state &= ~START_POSITON;
                  settings.vars.init_state |= END_POSITON;
                  stop_rotate(&motor_two);
                  start_rotate(0,800,&motor_one);

                }
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                settings.vars.state = WORK_STATE;
              }else {
                if (settings.vars.init_state & END_POSITON){
                  if ((opt&0x02)&&((opt_old&0x02)==0)){
                    stop_rotate(&motor_two);
                    start_rotate(0,800,&motor_one);
                    settings.vars.stop_time = 5;
                    time_stoped = uwTick;
                    settings.vars.frame_finded++;

                  }
                }else{
                  if ((opt&0x02)&&((opt_old&0x02)==0)){
                    stop_rotate(&motor_one);
                    start_rotate(1,800,&motor_two);
                    settings.vars.stop_time = 5;
                    time_stoped = uwTick;
                    settings.vars.frame_finded++;
                  }
                }
              }
            }else{
              settings.vars.init_state |= STARTED;
              if (settings.vars.init_state & END_POSITON){
                stop_rotate(&motor_two);
                start_rotate(0,800,&motor_one);
              }else{
                stop_rotate(&motor_one);
                start_rotate(1,800,&motor_two);
              }
            }
          break;
        default:
          settings.vars.state = INIT_STATE;
    }
  }else{
    suspend_rotate(&motor_one);
    suspend_rotate(&motor_two);
    if (uwTick>(time_stoped+settings.vars.stop_time*1000)){
      settings.vars.stop_time =0;
      awaik_rotate(&motor_one);
      awaik_rotate(&motor_two);
    }
  }
  opt_old = opt;
}
u8 get_opt_mask(){
  u8 opt_state;
  opt_state =0;
  if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)){
    opt_state |=0x01;
  }else{
    opt_state &=~0x01;
  }
  if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)){
    opt_state |=0x02;
  }else{
    opt_state &=~0x02;
  }
  if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7)){
    opt_state |=0x04;
  }else{
    opt_state &=~0x04;
  }
}
