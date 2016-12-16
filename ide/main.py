import io
import six
import packaging
import packaging.version
import packaging.specifiers
import packaging.requirements
import serial




from kivy.app import App
from kivy.uix.widget import Widget
from kivy.uix.label import Label
from kivy.properties import NumericProperty,ReferenceListProperty,\
                            ObjectProperty
from kivy.vector import Vector
from kivy.clock import Clock
from kivy.graphics.fbo import Fbo
from random import randint
from kivy.properties import StringProperty
from kivy.core.image import Image as CoreImage
MAX_RECEIVE_BYTE = 256

receive_timer = 0
receive_byte_num = 0
receive_buff = [0 for x in range(MAX_RECEIVE_BYTE)]


def com_list(serial_port_list, device):
    global receive_timer
    global receive_byte_num
    receive_byte_num = 0
    packet_num = 0
    print("start_com_listing")
    receive_timer = time.time()
    while 1:                 
        if (time.time() > (receive_timer+0.03)) & (receive_byte_num!=0):
#            print([receive_buff[x] for x in range(receive_byte_num)])
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


class MapApp(App):
#  k_numm = NumericProperty(0)
  kp_array = []
  def build(self):
    game = MapGame()
    game.size = 640,480
    print(game.size)
    game.serve_ball()
 #   self.kp_numm +=1
    self.add_kp(game,game.width/4,game.top/3)
    self.add_kp(game,game.width*(3/4),game.top*(2/3))
    Clock.schedule_interval(game.update,1.0/60.0)
    return game


  def add_kp(self,game,x,y):
    kp = len(self.kp_array)
    self.kp_array.append(KP(center = (x,y),kp_number = kp))
    label = Label(text = str(kp+1),center = (x,y+30),font_size = 15,color=(.1,.5,.1))
    self.kp_array[kp].add_widget(label)
    game.add_widget(self.kp_array[kp])
    print('add kp number',kp)
    


#    game.add_widget(KP_Label(center = (game.width*(3/4),game.top*(2/3)),text = '2',font_size = 15))


      


if __name__== '__main__':
  MapApp().run()
