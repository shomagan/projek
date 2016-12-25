import io

import six
import serial_port
import serial
import platform
import re
from time import localtime, sleep

from kivy.app import App
from kivy.logger import Logger
from kivy.lang import Builder
from kivy.uix.widget import Widget
from kivy.uix.button import Button
from kivy.uix.switch import Switch
from kivy.uix.checkbox import CheckBox
from kivy.uix.textinput import TextInput
from kivy.uix.label import Label

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
MAX_WRITE_SETTINGS = 30
SETTINGS_PREFIX_SIZE = 13
settings_write = "settings_rite"
settings_read = "settings_read"
settings_save = "settings_save"
settings_write_b = b'settings_rite'
settings_read_b = b'settings_read'
settings_save_b = b'settings_save'


def crc16(pck, lenght):
    """CRC16 for modbus"""
    CRC = 0xFFFF
    i = 0
    while i < lenght:
        CRC ^= pck[i]
        i += 1
        j = 0
        while j < 8:
            j += 1
            if (CRC & 0x0001) == 1:
                CRC = ((CRC >> 1) & 0xFFFF) ^ 0xA001
            else:
                CRC >>= 1
    return CRC & 0xFFFF


class FloatInput(TextInput):
    pat = re.compile('[^0-9]')

    def insert_text(self, substring, from_undo=False):
        pat = self.pat
        if '.' in self.text:
            s = re.sub(pat, '', substring)
        else:
            s = '.'.join([re.sub(pat, '', s) for s in substring.split('.', 1)])
        return super(FloatInput, self).insert_text(s, from_undo=from_undo)


class TestApp(App):
    display_type = OptionProperty('normal', options=['normal', 'popup'])
    connect_status = CheckBox(active=False, group="money")
    rtc = TextInput(text='0:0:0')
    layout = GridLayout()
    have_serial = 0
    wind10 = False

    def build(self):
        self.settings_cls = MySettingsWithTabbedPanel
        self.have_serial = 0
        platform_type = platform.platform()
        Logger.info("platform - {0}".format(platform_type))
        if 'Windows-10' in platform_type:
            self.wind10 = True
        else:
            self.wind10 = False

        self.layout.cols = 2
        find_button = Button(text='Find device')
        find_button.size_hint_x = None
        find_button.bind(on_press=lambda j: self.find_device())
        self.connect_status = CheckBox(active=False, group="money")
        self.rtc = TextInput(text='current time 0:0:0 \n')
        read_time_button = Button(text='read time')
        read_time_button.size_hint_x = None
        read_time_button.bind(on_press=lambda j: self.read_time(self.ser))
        sync_time_button = Button(text='sync time')
        sync_time_button.size_hint_x = None
        sync_time_button.bind(on_press=lambda j: self.sync_time(self.ser))
        change_settings_button = Button(text='change_settings')
        change_settings_button.size_hint_x = None
        change_settings_button.bind(on_release=lambda j: App.open_settings(self))
        write_settings_button = Button(text='write_settings')
        write_settings_button.size_hint_x = None
        write_settings_button.bind(on_press=lambda j: self.write_settings())
        read_settings_button = Button(text='read_settings')
        read_settings_button.size_hint_x = None
        read_settings_button.bind(on_press=lambda j: self.read_settings())
        self.layout.add_widget(find_button)
        self.layout.add_widget(read_time_button)
        self.layout.add_widget(sync_time_button)
        self.layout.add_widget(change_settings_button)
        self.layout.add_widget(write_settings_button)
        self.layout.add_widget(read_settings_button)
        self.layout.add_widget(self.rtc)
        self.layout.add_widget(self.connect_status)
        return self.layout

    def build_config(self, config):
        """
        Set the default values for the configs sections.
        """
        config.setdefaults('time_up', {'time_hour': 0, 'time_min': 0})
        config.setdefaults('time_down', {'time_hour': 0, 'time_min': 0})
        for i in range(1, 119):
            config.setdefaults('Frame'+str(i), {'name': 'FRAME'+str(i), 'time_sec': 20, 'led': False})

    def build_settings(self, settings):
        """
        Add our custom section to the default configuration object.
        """
        # We use the string defined above for our JSON, but it could also be
        # loaded from a file as follows:
        #     settings.add_json_panel('My Label', self.config, 'settings.json')
        with open("sets.json", "r") as sets_json:
            settings.add_json_panel('Time settings', self.config, data=sets_json.read())

    def find_device(self):
        com_available = serial_port.serial_ports()
        for i in range(len(com_available)):
            self.have_serial = 1
            try:
                self.ser = serial.Serial(com_available[i])
                self.ser.baudrate = 115200
                if self.wind10:
                    self.ser.timeout = 0.3

            except serial.SerialException as e:
                self.have_serial = 0
                break
            if self.have_serial:
                self.have_serial = 0
                for j in range(5):
                    answer = self.send_request(self.ser)
                    answer_str = answer
                    Logger.info("otvet {0}".format(answer_str))
                    if answer_str == receive_answer_init:
                        Logger.info("find device on {0}".format(com_available[i]))
                        self.have_serial = 1
                        break
                if self.have_serial:
                    self.layout.remove_widget(self.connect_status)
                    self.connect_status.active = True
                    self.layout.add_widget(self.connect_status)
                    break
                else:
                    self.ser.close()

    def write_time(self, ser):
        Logger.info("send_request_init {0}".format(send_request_init))
        ser.reset_input_buffer()
        ser.write(send_request_init.encode('ascii'))
        if not self.wind10:
            ser.timeout = 0.3
        receive = ser.read(11)
        self.ser.reset_input_buffer()
        Logger.info("write time receive {0}".format(receive))
        return receive

    def send_request(self, ser):
        Logger.info("send_request_init {0}".format(send_request_init))
        ser.reset_input_buffer()
        ser.write(send_request_init.encode('ascii'))
        if not self.wind10:
            ser.timeout = 0.3
        receive = ser.read(11)
        self.ser.reset_input_buffer()
        Logger.info("recv_request_init {0}".format(receive))
        return receive

    def write_settings_c(self, index, number):
        '''return 1 if ok write'''
        send_buff = [i for i in range(SETTINGS_PREFIX_SIZE+number*4+1+2)]
        for i in range(SETTINGS_PREFIX_SIZE):
            send_buff[i] = settings_write_b[i]
        send_len = SETTINGS_PREFIX_SIZE
        send_buff[send_len] = number
        send_len += 1
        for i in range(number):
            send_buff[send_len] = index+i
            send_len += 1
            if index+i == 118:
                hour = int(self.config.get('time_up', 'time_hour'))
                min = int(self.config.get('time_up', 'time_min'))
                time_sec = (hour | (min << 8)) & 0xffff
                led = 0
            elif index+i == 119:
                hour = int(self.config.get('time_down', 'time_hour'))
                min = int(self.config.get('time_down', 'time_min'))
                time_sec = (hour | (min << 8)) & 0xffff
                led = 0
            else:
                time_sec = int(self.config.get('Frame'+str(index+i+1), 'time_sec'))
                led = int(self.config.get('Frame'+str(index+i+1), 'led'))
            send_buff[send_len] = led & 0xff
            send_len += 1
            send_buff[send_len] = time_sec & 0xff
            send_len += 1
            send_buff[send_len] = (time_sec >> 8) & 0xff
            send_len += 1

        self.ser.reset_input_buffer()
        crc = crc16(send_buff, SETTINGS_PREFIX_SIZE+number*4+1)
        send_buff[SETTINGS_PREFIX_SIZE+number*4+1] = crc & 0xff
        send_buff[SETTINGS_PREFIX_SIZE+number*4+1+1] = (crc >> 8) & 0xff
        self.ser.write(send_buff)
        if not self.wind10:
            self.ser.timeout = 0.3
        receive_buff = self.ser.read(SETTINGS_PREFIX_SIZE+number+2)
        if len(receive_buff) == SETTINGS_PREFIX_SIZE+number+2:
            crc = crc16(receive_buff, SETTINGS_PREFIX_SIZE+number)
            crc_r = receive_buff[SETTINGS_PREFIX_SIZE+number] | (receive_buff[SETTINGS_PREFIX_SIZE+number+1] << 8)
            if crc == crc_r:
                for i in range(number):
                    if receive_buff[SETTINGS_PREFIX_SIZE+i] != index + i:
                        Logger.info("mismatch index {0}, {1}".format(receive_buff[SETTINGS_PREFIX_SIZE+i], index + i))
                        return 0
                return 1
            else:
                Logger.info("crc mismatch: {0}, {1}".format(crc, crc_r))
        else:
            Logger.info("len mismatch: {0}".format(len(receive_buff)))

            return 0

    def write_settings(self):
        '''write all settings '''
        if self.have_serial:
            if self.write_settings_c(0, 10):
                Logger.info("good write settings: {0}, {1}".format(0, 10))
            if self.write_settings_c(10, 10):
                Logger.info("good write settings: {0}, {1}".format(10, 10))
            if self.write_settings_c(20, 10):
                Logger.info("good write settings: {0}, {1}".format(20, 10))
            if self.write_settings_c(30, 10):
                Logger.info("good write settings: {0}, {1}".format(30, 10))
            if self.write_settings_c(40, 10):
                Logger.info("good write settings: {0}, {1}".format(40, 10))
            if self.write_settings_c(50, 10):
                Logger.info("good write settings: {0}, {1}".format(50, 10))
            if self.write_settings_c(60, 10):
                Logger.info("good write settings: {0}, {1}".format(60, 10))
            if self.write_settings_c(70, 10):
                Logger.info("good write settings: {0}, {1}".format(70, 10))
            if self.write_settings_c(80, 10):
                Logger.info("good write settings: {0}, {1}".format(80, 10))
            if self.write_settings_c(90, 10):
                Logger.info("good write settings: {0}, {1}".format(90, 10))
            if self.write_settings_c(100, 10):
                Logger.info("good write settings: {0}, {1}".format(100, 10))
            if self.write_settings_c(110, 10):
                Logger.info("good write settings: {0}, {1}".format(110, 10))
            self.save_settings_c()
        return 0

    def read_settings_c(self, index, number):
        send_buff = [i for i in range(SETTINGS_PREFIX_SIZE+2)]
        for i in range(SETTINGS_PREFIX_SIZE):
            send_buff[i] = settings_read_b[i]
        send_len = SETTINGS_PREFIX_SIZE
        send_buff[send_len] = number
        send_len += 1
        send_buff[send_len] = index
        send_len += 1
        self.ser.reset_input_buffer()
        Logger.info("read settings {0}".format(send_buff))
        self.ser.write(send_buff)
        if not self.wind10:
            self.ser.timeout = 0.4
        receive_buff = self.ser.read(SETTINGS_PREFIX_SIZE+number*4+2)
        self.ser.reset_input_buffer()
        self.ser.write(send_buff)
        if not self.wind10:
            self.ser.timeout = 0.4
        receive_buff = self.ser.read(SETTINGS_PREFIX_SIZE+number*4+2)
        Logger.info("read settings {0}".format(receive_buff))
        if len(receive_buff) == SETTINGS_PREFIX_SIZE+number*4+2:
            crc = crc16(receive_buff, SETTINGS_PREFIX_SIZE+number*4)
            crc_r = receive_buff[SETTINGS_PREFIX_SIZE+number*4] | (receive_buff[SETTINGS_PREFIX_SIZE+number*4+1] << 8)
            if crc == crc_r:
                Logger.info("crc mtch: {0}, {1}".format(crc, crc_r))
                j = SETTINGS_PREFIX_SIZE
                for i in range(number):
                    index_recv = receive_buff[j]
                    j += 1
                    if index_recv == 118:
                        led = receive_buff[j]
                        j += 1
                        hour = receive_buff[j]
                        j += 1
                        min = receive_buff[j]
                        j += 1
                        Logger.info("read time_up: {0}, {1}".format(hour, min))
                        self.config.set('time_up', 'time_hour', str(hour))
                        self.config.write()
                        self.config.set('time_up', 'time_min', str(min))
                        self.config.write()
                    elif index_recv == 119:
                        led = receive_buff[j]
                        j += 1
                        hour = receive_buff[j]
                        j += 1
                        min = receive_buff[j]
                        j += 1
                        Logger.info("read time_down: {0}, {1}".format(hour, min))
                        self.config.set('time_down', 'time_hour', str(hour))
                        self.config.write()
                        self.config.set('time_down', 'time_min', str(min))
                        self.config.write()
                    else:
                        led = receive_buff[j]
                        j += 1
                        time_sec = receive_buff[j]
                        j += 1
                        time_sec |= ((receive_buff[j] << 8) & 0xff00)
                        j += 1
                        Logger.info("read frame {0},{1}".format((index_recv+1), time_sec))
                        self.config.set('Frame'+str(index_recv+1), 'time_sec', str(time_sec))
                        self.config.write()
                        self.config.set('Frame'+str(index_recv+1), 'led', str(led))
                        self.config.write()
            else:
                Logger.info("crc mismatch: {0}, {1}".format(crc, crc_r))
                return 0
            return 1
        else:
            return 0

    def read_settings(self):
        ''' read all settings '''
        if self.have_serial:
            if self.read_settings_c(0, 30):
                Logger.info("good read settings: {0}, {1}".format(0, 30))
            if self.read_settings_c(30, 30):
                Logger.info("good read settings: {0}, {1}".format(30, 30))
            if self.read_settings_c(60, 30):
                Logger.info("good read settings: {0}, {1}".format(60, 30))
            if self.read_settings_c(90, 30):
                Logger.info("good read settings: {0}, {1}".format(90, 30))
        return 0


    def save_settings_c(self):
        '''return 1 if ok write'''
        send_buff = [i for i in range(SETTINGS_PREFIX_SIZE)]
        for i in range(SETTINGS_PREFIX_SIZE):
            send_buff[i] = settings_save_b[i]
        self.ser.reset_input_buffer()
        self.ser.write(send_buff)
        if not self.wind10:
            self.ser.timeout = 0.3
        receive_buff = self.ser.read(SETTINGS_PREFIX_SIZE+2)
        crc = crc16(receive_buff, SETTINGS_PREFIX_SIZE)
        crc_r = receive_buff[SETTINGS_PREFIX_SIZE] | (receive_buff[SETTINGS_PREFIX_SIZE+1] << 8)
        if crc == crc_r and len(receive_buff) == SETTINGS_PREFIX_SIZE+2:
            Logger.info("crc mtch: {0}, {1}".format(crc, crc_r))
            return 1
        else:
            Logger.info("crc mismatch: {0}, {1}".format(crc, crc_r))
            return 0

    def read_time(self, ser):
        Logger.info("send_request_time {0}".format(send_request_time))
        ser.reset_input_buffer()
        ser.write(send_request_time.encode('ascii'))
        if not self.wind10:
            ser.timeout = 0.3
        receive = ser.read(18)
        self.ser.reset_input_buffer()
        Logger.info("send_request_time receive {0}".format(receive))
        receive_ord = list(receive)
        if len(receive_ord) == 18:
            crc = crc16(receive_ord, 16)
            crc_r = receive_ord[16] | (receive_ord[17] << 8)
            if crc == crc_r:
                hours = receive_ord[0]
                minutes = receive_ord[2]
                seconds = receive_ord[4]
                weekday = receive_ord[6]
                month = receive_ord[8]
                date = receive_ord[10]
                year = receive_ord[12]
                frame_number = receive_ord[13] | (receive_ord[14] << 8)
                self.layout.remove_widget(self.rtc)
                self.rtc = TextInput(text='current time '+str(hours)+':'+str(minutes)+':'+str(seconds)+'\n'+
                                            'frame number - '+str(frame_number)+'\n')
                self.layout.add_widget(self.rtc)

            else:
                Logger.info("crc mismatch {0},{1}".format(hex(crc), hex(crc_r)))
        return receive

    def sync_time(self, ser):
        send_time = [i for i in range(20)]
        tm = localtime()
        send_time[0] = ord('t')
        send_time[1] = ord('i')
        send_time[2] = ord('m')
        send_time[3] = ord('e')
        send_time[4] = tm.tm_hour & 0xff
        send_time[6] = tm.tm_min & 0xff
        send_time[8] = tm.tm_sec & 0xff
        send_time[10] = tm.tm_wday & 0xff
        send_time[12] = tm.tm_mon & 0xff
        send_time[14] = tm.tm_mday & 0xff
        send_time[16] = (tm.tm_year - 2000) & 0xff
        ser.reset_input_buffer()
        crc = crc16(send_time, 18)
        send_time[18] = crc & 0xff
        send_time[19] = (crc >> 8) & 0xff
        ser.write(send_time)
        if not self.wind10:
            ser.timeout = 0.3
        receive = ser.read(20)
        #        receive_ord = [ord(receive[i]) for i in range(len(receive))]
        receive_ord = list(receive)
        if len(receive_ord) == 16:
            if send_time == receive_ord:
                self.status_sync.active = True
            else:
                Logger.info("time mismatch")
        return receive

    def on_config_change(self, config, section, key, value):
        """
        Respond to changes in the configuration.
        """
        Logger.info("main.py: App.on_config_change: {0}, {1}, {2}, {3}".format(
            config, section, key, value))

    def close_settings(self, settings):
        """
        The settings panel has been closed.
        """
        Logger.info("main.py: App.close_settings: {0}".format(settings))
        super(TestApp, self).close_settings(settings)


class MySettingsWithTabbedPanel(SettingsWithTabbedPanel):
    """
    It is not usually necessary to create subclass of a settings panel. There
    are many built-in types that you can use out of the box
    (SettingsWithSidebar, SettingsWithSpinner etc.).

    You would only want to create a Settings subclass like this if you want to
    change the behavior or appearance of an existing Settings class.
    """
    def on_close(self):
        Logger.info("main.py: MySettingsWithTabbedPanel.on_close")

    def on_config_change(self, config, section, key, value):
        Logger.info(
            "main.py: MySettingsWithTabbedPanel.on_config_change: "
            "{0}, {1}, {2}, {3}".format(config, section, key, value))
    def add_kivy_panel(self):
        pass


if __name__ == '__main__':
    TestApp().run()


