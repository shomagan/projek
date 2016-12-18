import sys
import glob
import serial

MAX_RECEIVE_BYTE = 255
serial_is_open = 0
receive_timer = 0
receive_byte_num = 0
receive_buff = [0 for x in range(MAX_RECEIVE_BYTE)]



def com_start_list(serial_port, device):
    thread.start_new_thread(com_list, (serial_port, device))


def com_list(serial_port_list, device):
    global receive_timer
    global receive_byte_num
    receive_byte_num = 0
    packet_num = 0
    print("start_com_listing")
    receive_timer = time.time()
    while 1:                 
        if (time.time() > (receive_timer+0.01)) & (receive_byte_num!=0):
#            print([receive_buff[x] for x in range(receive_byte_num)])
            if device.receive_rtu_packet(receive_buff, receive_byte_num):
                send_packet(serial_port_list, device.answer_packet, device.answer_packet_size)
            else:
                print('error_packet',receive_buff[0:receive_byte_num])
            receive_byte_num = 0
            packet_num += 1
        receive_char = serial_port_list.read(1)
        if receive_char:
            receive_timer = time.time()
            if receive_byte_num >=MAX_RECEIVE_BYTE:
                print('error max_packet_size')
                receive_byte_num =0
            receive_buff[receive_byte_num] = ord(receive_char)
            receive_byte_num += 1

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')
    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

if __name__ == '__main__':
    print(serial_ports())