#include "main.h"
extern char init_send[];
extern char const init_receive[] ;
u8 receive_packet_hanling(u8* buff);
u16 add_crc16(u8* pck, u16 len);
u8 check_crc16(u8* pck, u16 len);
u16 crc16(u8* pck, u16 len);
