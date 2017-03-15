#include "step.h"


motor_template motor_one;
motor_template motor_two;


u8 motor_init(GPIO_TypeDef* gpio,u8 pin_dir,u8 pin_step,u8 pin_sleep,motor_template* motor){
  
  motor->awaik  =1;
  motor->gpio = gpio;
  motor->pin_dir = pin_dir;
  motor->pin_step = pin_step;
  motor->pin_sleep = pin_sleep;
  motor->tick_from_start =0;
  return 0x00;
}
/*
*u8 rotate_vector 0 , 1 - reverse
*u8 motor_number 1 , 2 
*/
u8 start_rotate(u8 rotate_vector,u32 step_number,motor_template* motor){
  motor->aqsel_time = 511;
  motor->step_number = step_number;
  HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_sleep),GPIO_PIN_SET);
  motor->tick_from_start =0;
  if (rotate_vector <= 2){
    motor->dir_state = rotate_vector;
    if (rotate_vector){
      HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_dir),GPIO_PIN_RESET);
    }else{
      HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_dir),GPIO_PIN_SET);
    }
    return 1;
  }else{
    motor->dir_state = 2;
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_sleep),GPIO_PIN_RESET);
    return 0;
  }

}
u8 change_rotate(u32 step_number,motor_template* motor){
  motor->aqsel_time = 511;
  motor->step_number = step_number;
  HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_sleep),GPIO_PIN_SET);
  motor->tick_from_start =0;
  if (motor->dir_state == 0){
    motor->dir_state = 1;
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_dir),GPIO_PIN_RESET);
  }else if (motor->dir_state == 1){
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_dir),GPIO_PIN_SET);
    motor->dir_state = 0;
  }else{
    motor->dir_state = 2;
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_sleep),GPIO_PIN_RESET);
  }
  return 1;
}

u8 stop_rotate(motor_template* motor){
  motor->dir_state= 2;//stop
  motor->step_number =0;
  HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_sleep),GPIO_PIN_RESET);//hh
  return 1;
}
u8 suspend_rotate(motor_template* motor){
  HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_sleep),GPIO_PIN_SET);//hh
  motor->awaik =0;
  return 1;
}
u8 awaik_rotate(motor_template* motor){
  if(motor->step_number ==0){
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_sleep),GPIO_PIN_RESET);//hh
  }
  motor->awaik =1;
  return 1;
}

u8 step_motor_control(motor_template* motor){
  HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_step),GPIO_PIN_RESET);
  if((motor->step_number)&&(motor->awaik)){
    if (motor->step_state ==0){
      motor->step_number--;
      HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_step),GPIO_PIN_SET);
      motor->step_state =1;
      motor->tick_from_start++;
    }else{
      motor->step_state =0;
    }
  }else{
    HAL_GPIO_WritePin(motor->gpio,BIT(motor->pin_step),GPIO_PIN_RESET);
  }
  return 1;

}

