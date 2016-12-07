#include "step.h"


motor_template motor_one;
motor_template motor_two;

u8 motor_init(GPIO_TypeDef* gpio,u8 pin_dir,u8 pin_step,motor_template* motor){
  motor->gpio = gpio;
  motor->pin_dir = pin_dir;
  motor->pin_step = pin_step;
}
/*
*u8 rotate_vector 0 , 1 - reverse
*u8 motor_number 1 , 2 
*/
u8 start_rotate(u8 rotate_vector,u32 step_number,motor_template* motor){
  motor->aqsel_time = 511;
  motor->step_number = step_number;
  if (rotate_vector <= 2){
    motor->dir_state = rotate_vector;
    if (rotate_vector){
      HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_dir),GPIO_PIN_RESET);
    }else{
      HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_dir),GPIO_PIN_SET);
    }
  }else{
    motor->dir_state = 2;
  }

}
u8 change_rotate(u32 step_number,motor_template* motor){
  motor->aqsel_time = 511;
  motor->step_number = step_number;
  if (motor->dir_state == 0){
    motor->dir_state = 1;
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_dir),GPIO_PIN_RESET);
  }else if (motor->dir_state == 1){
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_dir),GPIO_PIN_SET);
    motor->dir_state = 0;
  }else{
    motor->dir_state = 2;
  }

}

u8 stop_rotate(u32 step_number, motor_template* motor){
  motor->dir_state= 2;//stop
  motor->step_number = step_number;


}
u8 step_motor_control(motor_template* motor){
/*  u16 aqsel_time;
  u8 dir_state;
  u8 step_state;*/
  u32 mask;
  if(motor->step_number){
    if(motor->aqsel_time){
      motor->aqsel_time--;
    }
    mask = 0x00000001;
    if(uwTick & mask){
      if (motor->step_state ==0){
        HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_step),GPIO_PIN_SET);
        motor->step_state =1;
      }else{
        HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_step),GPIO_PIN_RESET);
        motor->step_state =0;
      }
    }
    motor->step_number--;
  }else{
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_step),GPIO_PIN_RESET);
  }

}

