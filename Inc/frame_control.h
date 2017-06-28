#include "main.h"
#define OPT_FULL_CAPE 0x03
u8 frame_init(void);
u8 frame_control_hadler(void);
u8 move_to_left(u16 step,u8 with_stop);
u8 move_to_right(u16 step,u8 with_stop);

u8 get_opt_mask(void);
u8 break_to_init(void);
u8 enable_led(void);
u8 disable_led(void);
