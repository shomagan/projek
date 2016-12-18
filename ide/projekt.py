import io

import six
import serial_port
import serial
from time import gmtime, mktime

from kivy.app import App
from kivy.lang import Builder
from kivy.uix.widget import Widget
from kivy.uix.button import Button
from kivy.uix.switch import Switch
from kivy.uix.checkbox import CheckBox
from kivy.uix.textinput import TextInput

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.popup import Popup
from kivy.uix.settings import (Settings, SettingsWithSidebar,
                               SettingsWithSpinner,
                               SettingsWithTabbedPanel)
from kivy.properties import OptionProperty, ObjectProperty


send_request_init = 'Jalisco'
receive_answer_init = b'Guadalajara'
send_request_time = 'how_mach_time'

def crc16(pck,lenght):
  """CRC16 for modbus"""
  CRC = 0xFFFF
  i = 0
  while ( i < lenght):
    CRC ^= pck[i]
    i+=1
    j = 0 
    while ( j < 8):
       j+=1
       if ( (CRC & 0x0001) == 1 ): CRC = ((CRC >> 1)&0xFFFF) ^ 0xA001;
       else: CRC >>= 1;
  return (CRC&0xFFFF)



class TestApp(App):
    display_type = OptionProperty('normal', options=['normal', 'popup'])
    layout = GridLayout()
    def build(self):
        self.com_available = serial_port.serial_ports()
        self.com_port_number = 257
        self.layout.cols = 2
        self.button_number = 0
#        textinput = TextInput(text='Hello world')
        self.init_button = Button(text='Find device')
        self.init_button.size_hint_x = None
        self.init_button.bind(on_press=lambda j: self.find_device())
        self.layout.add_widget(self.init_button)
#        self.layout.add_widget(textinput)
        return self.layout
    def find_device(self):
        for i in range(len(self.com_available)):
            have_serial = 1
            try:
                ser = serial.Serial(self.com_available[i])
                ser.baudrate = 115200
                print (ser.name)          # check which port was really used
            except serial.SerialException as e:
                have_serial = 0
                print("could not open port \n",self.com_available[i])
                break
            count = 0
            if have_serial:
                have_serial = 0
                for j in range(5):
                    answer = self.send_request(ser)
                    answer_str = answer
                    print("otvet  ",answer_str)
                    if answer_str ==receive_answer_init:
                      print ("find_packet")
                      have_serial=1
                      self.com_port_number =i
                      break
                if have_serial:
                    break  
        if have_serial:
            self.layout.clear_widgets()
            self.layout.cols = 2
            self.time_input = []
            self.led_rise_switch = []
            for i in range(10):
              self.time_input.append(TextInput(text='time frame'+str(i+1),
                                     width= 20))
              self.led_rise_switch.append(Switch(active = False))

            self.init_button = Button(text='read time')
            self.init_button.size_hint_x = None
            self.init_button.bind(on_press=lambda j: self.read_time(ser))
            self.sync_time_button = Button(text='sync time')
            self.sync_time_button.size_hint_x = None
            self.sync_time_button.bind(on_press=lambda j: self.sync_time(ser))
            self.status_sync = CheckBox(active = False,group = "money")
            for i in range(5):
              self.layout.add_widget(self.time_input[i])
              self.layout.add_widget(self.time_input[5+i])
              self.layout.add_widget(self.led_rise_switch[i])
              self.layout.add_widget(self.led_rise_switch[5+i])
            self.rtc = TextInput(text=  "hours -  " + str(0)+
                                           " minutes - " + str(0) +
                                           " seconds - " + str(0) +
                                           " weekday - " + str(0) +
                                           " month - " + str(0) +
                                           " date - " + str(0) +
                                           " year - " + str(0) )
            self.layout.add_widget(self.init_button)
            self.layout.add_widget(self.rtc)
            self.layout.add_widget(self.sync_time_button)
            self.layout.add_widget(self.status_sync)

        else:
            self.init_button = Button(text='Find device')
            self.init_button.size_hint_x = None

            
          
                  
    def send_request(self,ser):
        print(send_request_init)
        ser.reset_input_buffer()
        ser.write(send_request_init.encode('ascii'))
        ser.timeout = 0.2
        receive = ser.read(11)
        print(receive)
        return receive

    def read_time(self,ser):
        print(send_request_time)
        ser.reset_input_buffer()
        ser.write(send_request_time.encode('ascii'))
        ser.timeout = 0.2
        receive = ser.read(16)
        print(receive)
#        receive_ord = [ord(receive[i]) for i in range(len(receive))]
        receive_ord  = list(receive)
        print(receive_ord)
        if(len(receive_ord)==16):
          crc = crc16(receive_ord,14)
          crc_r = receive_ord[14]|(receive_ord[15]<<8)
          if crc==crc_r:
              self.hours = receive_ord[0]
              self.minutes = receive_ord[2]
              self.seconds = receive_ord[4]
              self.weekday = receive_ord[6]
              self.month = receive_ord[8]
              self.date = receive_ord[10]
              self.year = receive_ord[12]
              self.layout.remove_widget(self.rtc)
              self.rtc = TextInput(text=  "hours -  " + str(self.hours)+
                                           "minutes - " + str(self.minutes) +
                                           "seconds - " + str(self.seconds) +
                                           "weekday - " + str(self.weekday) +
                                           "month - " + str(self.month) +
                                           "date - " + str(self.date) +
                                           "year - " + str(self.year) )
              self.layout.add_widget(self.rtc)

          else:
              print("crc mismatch",hex(crc),hex(crc_r))
        return receive
    def sync_time(self,ser):
        send_time = [0 for i in range(20)] 
        tm = gmtime()
        print(tm)
        send_time[0] = ord('t')
        send_time[1] = ord('i')
        send_time[2] = ord('m')
        send_time[3] = ord('e')
        send_time[4] = (tm.tm_hour)&0xff
        send_time[6] = (tm.tm_min)&0xff
        send_time[8] = tm.tm_sec&0xff
        send_time[10] = tm.tm_wday&0xff
        send_time[12] = tm.tm_mon&0xff
        send_time[14] = tm.tm_mday&0xff
        send_time[16] = (tm.tm_year - 2000)&0xff
        ser.reset_input_buffer()
        crc = crc16(send_time,18)
        send_time[18] = crc&0xff
        send_time[19] = (crc>>8)&0xff
        print(send_time)
        ser.write(send_time)
        ser.timeout = 0.2
        receive = ser.read(20)
        print(receive)
#        receive_ord = [ord(receive[i]) for i in range(len(receive))]
        receive_ord  = list(receive)
        print(receive_ord)
        if(len(receive_ord)==16):
          if send_time==receive_ord:
              self.status_sync.active = True

          else:
              print("time mismatch")
        return receive




if __name__ == '__main__':
    TestApp().run()


