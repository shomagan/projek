#include "stm32f1xx_hal.h"
#include "main.h"
#define TO_RIGHT 0 
#define TO_LEFT  1 
#define STOP     2
#define DRIVER_SLEEP 0
#define DRIVER_AWAIKE 1
typedef struct {
  u8 awaik;
  u16 aqsel_time;
  u8 dir_state;
  u8 step_state;
  u32 step_number;
  GPIO_TypeDef* gpio;
  u8 pin_dir;
  u8 pin_step;
  u8 pin_sleep;//low is active
} motor_template;

u8 motor_init(GPIO_TypeDef* gpio,u8 pin_dir,u8 pin_step,u8 pin_sleep,motor_template* motor);
/*
*u8 rotate_vector 0 , 1 - reverse
*u8 motor_number 1 , 2 
*/
u8 suspend_rotate(motor_template* motor);
u8 awaik_rotate(motor_template* motor);

u8 start_rotate(u8 rotate_vector,u32 step_number,motor_template* motor);
u8 change_rotate(u32 step_number,motor_template* motor);

u8 stop_rotate(motor_template* motor);
u8 step_motor_control(motor_template* motor);


extern motor_template motor_one;
extern motor_template motor_two;
