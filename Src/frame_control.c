#include "frame_control.h"
#include "stm32f1xx_hal.h"
#include "step.h"
#include "saver.h"
/*
#define OPT_ONE GPIO_PIN_5
#define OPT_TWO GPIO_PIN_6
#define OPT_THREE GPIO_PIN_7
*/
static u8 opt_old;
static u8 opt;  
static u16 shift_step;  
static u32 time_stoped;
#define LEFT_PARA 0x01
#define MIDLE_PARA 0x02
#define RIGHT_PARA 0x04
#define STRETCH_STEP 30
#define MAX_MAIN_STEP 1600
#define STEP_FOR_MEMORY_STATE 80

u8 rising_only_opt(u8 para_number);
u8 rising_opt(u8 para_number);
u8 rising_full();
u8 stretch(u16 step);
u8 stop_move();
static u8 frame_control_search_start();
static u8 frame_control_work();
static u8 frame_control_no_state();
static u8 move_to_left_both(u16 step);
static u8 move_to_right_both(u16 step);
u8 frame_init(void){
  settings.vars.state = INIT_STATE;
  settings.vars.frame_number = 0;
  settings.vars.stop_time =0;
  opt_old = 0;
  return 1;
}

static u8 frame_control_search_start(){
    if (settings.vars.init_state & STARTED){
      if (rising_full()){
        settings.vars.init_state &= ~STARTED;
        settings.vars.init_state |= START_POSITON;
        settings.vars.stop_time = 1;
        time_stoped = uwTick;
        stop_move();
        settings.vars.state = WORK_STATE;
        settings.vars.frame_number = 0;
      }else if (rising_only_opt(LEFT_PARA)){
        settings.vars.stop_time = 1;
        time_stoped = uwTick;
        move_to_left(MAX_MAIN_STEP,1);
      }else if(motor_two.step_number==0){
        settings.vars.state = NO_STATE;
        settings.vars.init_state &= ~STARTED;
      }
    }else{
      settings.vars.init_state |= STARTED;
      move_to_left(MAX_MAIN_STEP,1);
    }
  return 1;
}
static u8 frame_control_work(){
  static u8 centrofication;
  if (settings.vars.init_state & STARTED){
    if (rising_full()){
      if (settings.vars.move_state == MOVE_TO_LEFT){
        settings.vars.init_state |= START_POSITON;
        settings.vars.frame_number = 0;
        move_to_right(MAX_MAIN_STEP,1);
        settings.vars.stop_time = 1;
        time_stoped = uwTick;
        settings.vars.state = WORK_STATE;
      }else{
        settings.vars.init_state &= ~START_POSITON;
        move_to_left(MAX_MAIN_STEP,1);
        settings.vars.stop_time = 1;
        time_stoped = uwTick;
        settings.vars.state = WORK_STATE;
        if(settings.vars.frame_number){
          settings.vars.frame_number--;
        }
        settings.vars.frame_number_saved = settings.vars.frame_number;
      }
    }else{
      if (settings.vars.move_state == MOVE_TO_RIGHT){
        if (settings.vars.init_state & STRETCH){
          if(rising_only_opt(MIDLE_PARA)){
            centrofication =0;
            move_to_right(MAX_MAIN_STEP,0);
            suspend_rotate(&motor_one);
            suspend_rotate(&motor_two);
            settings.vars.stop_time = settings.vars.frame[settings.vars.frame_number].time;
            if (settings.vars.stop_time){
              enable_led();
            }else{
              settings.vars.stop_time = 1;
            }
            time_stoped = uwTick;
            settings.vars.frame_number++;
          }else if(motor_one.step_number == 0){
            if(centrofication==0){
              centrofication=1;
              move_to_left_both(STRETCH_STEP*3);
            }else{
              centrofication=0;
              settings.vars.state = NO_STATE;
              settings.vars.init_state &= ~STARTED;
              settings.vars.stop_time = 1;
            }
          }
        }else{
          if (rising_only_opt(LEFT_PARA)&&
              (motor_one.step_number<(MAX_MAIN_STEP-STRETCH_STEP-60))){
            centrofication =0;
            stretch(STRETCH_STEP);
          }else if(motor_one.step_number==0){
            settings.vars.state = NO_STATE;
            settings.vars.init_state &= ~STARTED;
            settings.vars.stop_time = 1;
          }
        }
      }else{
        if (settings.vars.init_state & STRETCH){
          if(rising_only_opt(MIDLE_PARA)){
            move_to_left(MAX_MAIN_STEP,0);
            suspend_rotate(&motor_one);
            suspend_rotate(&motor_two);
            settings.vars.frame_number--;
            settings.vars.stop_time = settings.vars.frame[settings.vars.frame_number].time;
            if (settings.vars.stop_time){
              enable_led();
            }else{
              settings.vars.stop_time = 1;
            }
            time_stoped = uwTick;
          }else if(motor_one.step_number == 0){
            if(centrofication==0){
              centrofication=1;
              move_to_left_both(STRETCH_STEP*3);
            }else{
              centrofication=0;
              settings.vars.state = NO_STATE;
              settings.vars.init_state &= ~STARTED;
              settings.vars.stop_time = 1;
            }
          }
        }else{
          if (rising_only_opt(LEFT_PARA) &&
              (motor_two.step_number<(MAX_MAIN_STEP-STRETCH_STEP-60))){
            centrofication =0;
            stretch(STRETCH_STEP);
          }else if(motor_two.step_number==0){
            settings.vars.state = NO_STATE;
            settings.vars.init_state &= ~STARTED;
            settings.vars.stop_time = 1;
          }
        }
      }
    }
  }else{
    settings.vars.init_state |= STARTED;
    move_to_right(MAX_MAIN_STEP,1);
  }
  return 1;
}
static u8 frame_control_no_state(){
  if (settings.vars.init_state & STARTED){
    if (settings.vars.move_state == MOVE_TO_LEFT){
      if (rising_only_opt(LEFT_PARA)){
        break_to_init();
        time_stoped = uwTick;
        stop_move();
      }else if(motor_two.step_number==0){
        shift_step =(shift_step>=3600)?200:(shift_step + 400);
        move_to_right(shift_step,1);
      }
    }else if (settings.vars.move_state == MOVE_TO_RIGHT){
      if (rising_only_opt(MIDLE_PARA)){
        break_to_init();
        time_stoped = uwTick;
        stop_move();
      }else if(motor_one.step_number==0){
        shift_step =(shift_step>=3600)?200:(shift_step + 400);
        move_to_left(shift_step,1);
      }
    }
  }else{
    settings.vars.init_state |= STARTED;
    shift_step = 200;
    move_to_left(shift_step,1);
  }
  return 1;
}
u8 frame_control_hadler(void){
  opt = get_opt_mask();
  if (!settings.vars.stop_time){
    switch(settings.vars.state){
    case INIT_STATE:
      frame_control_search_start();
      break;
    case WORK_STATE:
      frame_control_work();
      break;
    case(NO_STATE):
      frame_control_no_state();
      break;
    default:
      settings.vars.state = NO_STATE;
      settings.vars.init_state &= ~STARTED;
      settings.vars.stop_time = 1;
    }
  }else{
    suspend_rotate(&motor_one);
    suspend_rotate(&motor_two);
    if (uwTick>(time_stoped+settings.vars.stop_time*1000)){
      if (settings.vars.move_state == MOVE_TO_RIGHT){
        if (settings.vars.frame_number>0){
          if(settings.vars.frame[settings.vars.frame_number-1].option & ENABLE_LED){
          }else{
            disable_led();
          }
        }else{
          if(settings.vars.frame[settings.vars.frame_number].option & ENABLE_LED){
          }else{
            disable_led();
          }
        }
      }else{
        if(settings.vars.frame[settings.vars.frame_number].option & ENABLE_LED){
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
  return 1;
}
u8 break_to_init(){
  settings.vars.state = INIT_STATE;
  settings.vars.init_state &= ~STARTED;
  settings.vars.stop_time = 1;
  return 1;
}
u8 move_to_left(u16 step,u8 with_stop){
  settings.vars.init_state &= ~STRETCH;
  settings.vars.move_state = MOVE_TO_LEFT;
  if(with_stop){
    stop_rotate(&motor_one);
  }
  motor_one.step_number =0;
  start_rotate(1,step,&motor_two);
  return 1;
}
u8 move_to_right(u16 step,u8 with_stop){
  settings.vars.init_state &= ~STRETCH;
  settings.vars.move_state = MOVE_TO_RIGHT;
  if(with_stop){
    stop_rotate(&motor_two);
  }
  motor_two.step_number =0;
  start_rotate(0,step,&motor_one);
  return 1;
}
u8 move_to_left_both(u16 step){
  start_rotate(0,step,&motor_two);
  start_rotate(0,step,&motor_one);
  return 1;
}
u8 move_to_right_both(u16 step){
  start_rotate(1,step,&motor_one);
  start_rotate(1,step,&motor_two);
  return 1;
}

u8 stretch(u16 step){
  settings.vars.init_state |= STRETCH;
  stop_rotate(&motor_two);
  motor_two.step_number = 0;
  suspend_rotate(&motor_two);
  start_rotate(0,step,&motor_one);
  return 1;
}
u8 stop_move(){
  settings.vars.init_state &= ~STRETCH;
  settings.vars.move_state = STOPED;
  stop_rotate(&motor_two);
  stop_rotate(&motor_one);
  return 1;
}


u8 rising_only_opt(u8 para_number){
  u8 para_inverse;
  para_inverse = ~para_number;
  para_inverse &= 0x03;
  if ((opt&para_number)&&((opt_old&para_number)==0)&&((opt&para_inverse))==0){
    return 1;
  }else{
    return 0;
  }
}  
u8 rising_full(){
  if((opt!=opt_old)&&(opt==0x03)){
    return 1;
  }else{
    return 0;
  }
}
u8 enable_led(){
  HAL_GPIO_WritePin(GPIOA, BIT(4), GPIO_PIN_RESET);//led enable
  return 1;
}
u8 disable_led(){
  HAL_GPIO_WritePin(GPIOA, BIT(4), GPIO_PIN_SET);//led disable
  return 1;
}

u8 get_opt_mask(){
  u8 opt_state;
  opt_state =0;
  if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)){
    settings.vars.init_state |= DID_LEFT_OPT;
    time_for_state_memory_left = STEP_FOR_MEMORY_STATE;
    opt_state |=0x01;
  }else{
    opt_state &=~0x01;
  }
  if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)){
    settings.vars.init_state |= DID_MIDLE_OPT;
    time_for_state_memory_midle =STEP_FOR_MEMORY_STATE;    
    opt_state |=0x02;
  }else{
    opt_state &=~0x02;
  }

  return opt_state;
}
