#include "frame_control.h"
#include "stm32f1xx_hal.h"
#include "step.h"
#include "saver.h"
/*#define OPT_ONE GPIO_PIN_5
#define OPT_TWO GPIO_PIN_6
#define OPT_THREE GPIO_PIN_7
*/
static u8 opt_old;
static u8 opt;  
static u16 shift_step;  
#define LEFT_PARA 0x01
#define MIDLE_PARA 0x02
#define RIGHT_PARA 0x04
#define STRETCH_STEP 25
#define MAX_MAIN_STEP 1200
u8 rising_only_opt(u8 para_number);
u8 rising_opt(u8 para_number);
u8 rising_midle_opt_after_left();
u8 rising_midle_opt_after_right();
u8 rising_left_after_two_right();
u8 rising_right_after_two_left();
u8 rising_full();
u8 move_to_left(u16 step);
u8 move_to_right(u16 step);
u8 stretch(u16 step);
u8 stop_move();




u8 frame_init(void){
  settings.vars.state = INIT_STATE;
  settings.vars.init_state = SEARCH_START;
  settings.vars.frame_finded = 0;
  settings.vars.stop_time =0;
  opt_old = 0;
}
u8 frame_control_hadler(void){
  static u32 time_stoped;
  opt = get_opt_mask();
  if (!settings.vars.stop_time){
    switch(settings.vars.state){
        case INIT_STATE:
          if (settings.vars.init_state & SEARCH_START){
            if (settings.vars.init_state & STARTED){
              if (rising_full()){
                settings.vars.init_state &= ~SEARCH_START;
                settings.vars.init_state &= ~STARTED;
                settings.vars.init_state |= CHECK_FRAME;
                settings.vars.init_state |= START_POSITON;
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                stop_move();
                settings.vars.frame_finded = 0;
              }else if (rising_only_opt(MIDLE_PARA)){
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                move_to_right(MAX_MAIN_STEP);
              }else if(motor_one.step_number==0){
                break_to_init();
              }
            }else{
              if (opt == OPT_FULL_CAPE){//not cool state
                settings.vars.state = NO_STATE;
              }else{
                settings.vars.init_state |= STARTED;
                move_to_right(MAX_MAIN_STEP);
              }
            }
          }else if (settings.vars.init_state & CHECK_FRAME){
            if (settings.vars.init_state & STARTED){
              if (rising_full()&&(motor_two.step_number<(MAX_MAIN_STEP-STRETCH_STEP-10))){
                settings.vars.init_state &= ~CHECK_FRAME;
                settings.vars.init_state &= ~START_POSITON;
                settings.vars.init_state &= ~STARTED;
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                stop_move();
                settings.vars.state = WORK_STATE;
                init_frame_struct(settings.vars.frame_finded);
              }else if (rising_only_opt(MIDLE_PARA)&&(motor_two.step_number<(MAX_MAIN_STEP-STRETCH_STEP-10))){
                settings.vars.stop_time = 1;
                time_stoped = uwTick;
                move_to_left(MAX_MAIN_STEP );
                settings.vars.frame_finded++;
              }else if(motor_two.step_number==0){
                break_to_init();
              }
            }else{
              settings.vars.init_state |= STARTED;
              move_to_left(MAX_MAIN_STEP );
            }
          }
          break;
        case WORK_STATE:
            if (settings.vars.init_state & STARTED){
              if (rising_full()){
                if ((settings.vars.move_state == MOVE_TO_RIGHT)&&
                  (motor_one.step_number<(MAX_MAIN_STEP-STRETCH_STEP))){
                  settings.vars.init_state |= START_POSITON;
                  settings.vars.frame_finded = 0;
                  move_to_left(MAX_MAIN_STEP);
                  settings.vars.stop_time = 1;
                  time_stoped = uwTick;
                  settings.vars.state = WORK_STATE;
                }else if(motor_two.step_number<(MAX_MAIN_STEP-STRETCH_STEP)){
                  if (settings.vars.frame_finded != settings.vars.frame_number_saved){
                    break_to_init();
                  }else{
                    settings.vars.init_state &= ~START_POSITON;
                    move_to_right(MAX_MAIN_STEP);
                  }
                  settings.vars.stop_time = 1;
                  time_stoped = uwTick;
                  settings.vars.state = WORK_STATE;
                }
              }else{
                if (settings.vars.move_state == MOVE_TO_RIGHT){
                  if (settings.vars.init_state & STRETCH){
                    if(motor_two.step_number == 0){
                      move_to_right(MAX_MAIN_STEP);
                      suspend_rotate(&motor_one);
                      suspend_rotate(&motor_two);
                      settings.vars.stop_time = settings.vars.frame[settings.vars.frame_finded-1].time;
                      if (settings.vars.stop_time){
                        enable_led();
                      }else{
                        settings.vars.stop_time = 1;
                      }
                      time_stoped = uwTick;
                      settings.vars.frame_finded--;
                    }
                  }else{
                    if (rising_only_opt(MIDLE_PARA)&&
                        (motor_one.step_number<(MAX_MAIN_STEP-STRETCH_STEP-10))){
                      stretch(STRETCH_STEP);
                    }else if(motor_one.step_number==0){
                      break_to_init();
                    }
                  }
                }else{
                  if (settings.vars.init_state & STRETCH){
                    if(motor_one.step_number == 0){
                      move_to_left(MAX_MAIN_STEP);
                      suspend_rotate(&motor_one);
                      suspend_rotate(&motor_two);
                      settings.vars.stop_time = settings.vars.frame[settings.vars.frame_finded].time;
                      if (settings.vars.stop_time){
                        enable_led();
                      }else{
                        settings.vars.stop_time = 1;
                      }
                      time_stoped = uwTick;
                      settings.vars.frame_finded++;
                    }
                  }else{
                    if (rising_only_opt(MIDLE_PARA) &&
                        (motor_two.step_number<(MAX_MAIN_STEP-STRETCH_STEP-10))){
                      stretch(STRETCH_STEP);
                    }else if(motor_two.step_number==0){
                      break_to_init();
                    }
                  }
                }
              }
            }else{
              settings.vars.init_state |= STARTED;
              move_to_right(MAX_MAIN_STEP);
            }
          break;
        case(NO_STATE):
          if (settings.vars.init_state & STARTED){
            if (rising_only_opt(MIDLE_PARA)){
              break_to_init();
              time_stoped = uwTick;
              stop_move();
            }else if (settings.vars.move_state == MOVE_TO_LEFT){
              if(motor_two.step_number==0){
                
                shift_step =(shift_step>=1800)?200:(shift_step + 400);
                move_to_right(shift_step);
              }
            }else if (settings.vars.move_state == MOVE_TO_RIGHT){
              if(motor_one.step_number==0){
                shift_step =(shift_step>=1800)?200:(shift_step + 400);
                move_to_left(shift_step);
              }
            }

          }else{
            settings.vars.init_state |= STARTED;
            shift_step = 200;
            move_to_left(shift_step);
          }
          break;
        default:
          settings.vars.state = INIT_STATE;
    }
  }else{
    suspend_rotate(&motor_one);
    suspend_rotate(&motor_two);
    if (uwTick>(time_stoped+settings.vars.stop_time*1000)){
      if (settings.vars.move_state == MOVE_TO_RIGHT){
        if (settings.vars.frame_finded>0){
          if(settings.vars.frame[settings.vars.frame_finded-1].option & ENABLE_LED){
          }else{
            disable_led();
          }
        }else{
          if(settings.vars.frame[settings.vars.frame_finded].option & ENABLE_LED){
          }else{
            disable_led();
          }
        }
      }else{
        if(settings.vars.frame[settings.vars.frame_finded].option & ENABLE_LED){
        }else{
          disable_led();
        }
      }
      settings.vars.stop_time =0;
      awaik_rotate(&motor_one);
      awaik_rotate(&motor_two);
    }
  }
  opt_old = opt;
}
u8 break_to_init(){
  settings.vars.state = INIT_STATE;
  settings.vars.init_state |= SEARCH_START;
  settings.vars.init_state &= ~STARTED;
  settings.vars.stop_time = 1;
}
u8 move_to_left(u16 step){
  settings.vars.init_state &= ~STRETCH;
  settings.vars.move_state = MOVE_TO_LEFT;
  stop_rotate(&motor_one);
  start_rotate(1,step,&motor_two);
}
u8 move_to_right(u16 step){
  settings.vars.init_state &= ~STRETCH;
  settings.vars.move_state = MOVE_TO_RIGHT;
  stop_rotate(&motor_two);
  start_rotate(0,step,&motor_one);
}
u8 stretch(u16 step){
  settings.vars.init_state |= STRETCH;
  if (settings.vars.move_state == MOVE_TO_RIGHT){
    stop_rotate(&motor_one);
    suspend_rotate(&motor_one);
    start_rotate(1,step,&motor_two);
  }else{
    stop_rotate(&motor_two);
    suspend_rotate(&motor_two);
    start_rotate(0,step,&motor_one);
  }
}
u8 stop_move(){
  settings.vars.init_state &= ~STRETCH;
  settings.vars.move_state = STOPED;
  stop_rotate(&motor_two);
  stop_rotate(&motor_one);
}


u8 rising_only_opt(u8 para_number){
  u8 para_inverse;
  para_inverse = ~para_number;
  para_inverse &= 0x07;
  if ((opt&para_number)&&((opt_old&para_number)==0)&&((opt&para_inverse))==0){
    return 1;
  }else{
    return 0;
  }
}  
u8 rising_midle_opt_after_left(){
  if ((opt&0x02)&&((opt_old&0x02)==0)&&(opt&0x01)){
    return 1;
  }else{
    return 0;
  }
}  
u8 rising_midle_opt_after_right(){
  if ((opt&0x02)&&((opt_old&0x02)==0)&&(opt&0x04)){
    return 1;
  }else{
    return 0;
  }
}  
u8 rising_left_after_two_right(){
  if ((opt&0x01)&&((opt_old&0x01)==0)&&(opt&0x02)&&(opt&0x04)){
    return 1;
  }else{
    return 0;
  }
}  
u8 rising_right_after_two_left(){
  if ((opt&0x04)&&((opt_old&0x04)==0)&&(opt&0x02)&&(opt&0x01)){
    return 1;
  }else{
    return 0;
  }
}  
u8 rising_full(){
  if(rising_right_after_two_left()||rising_left_after_two_right()){
    return 1;
  }else{
    return 0;
  }
}
u8 enable_led(){
  HAL_GPIO_WritePin(GPIOA, BIT(4), GPIO_PIN_RESET);//led enable
}
u8 disable_led(){
  HAL_GPIO_WritePin(GPIOA, BIT(4), GPIO_PIN_SET);//led disable
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
