import io

import six

import packaging

import packaging.version

import packaging.specifiers

import packaging.requirements


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

class KP_Label(Widget):
#  center_x = Widget.parent.center_x
#  text = ObjectProperty(None)
#  def __init__(kp_number)
#    self.kp_number = kp_number
  pass

class KP(Widget):
#  center_x = Widget.parent.center_x
  kp_number = NumericProperty(0)
#  def __init__(kp_number)
#    self.kp_number = kp_number
  pass




class MapImage(Widget):
  timer = NumericProperty(0)


class MapPaddle(Widget):

  score = NumericProperty(0)

  def bounce_ball(self, ball):
    if self.collide_widget(ball):
      ball.velocity_x *= -1


class MapBall(Widget):

  # velocity of the ball on x and y axis
  velocity_x = NumericProperty(0)
  velocity_y = NumericProperty(0)
  velocity_volume=NumericProperty(0)
  # referencelist property so we can use ball.velocity as
  # a shorthand, just like e.g. w.pos for w.x and w.y

  velocity = ReferenceListProperty(velocity_x, velocity_y)

  # ``move`` function will move the ball one step. This
  # will be called in equal intervals to animate the ball
  point_to=[]
  point_number = 0
  current_point = 0
  
    
  def move(self,game):
    if self.current_point < self.point_number:
      if Vector(self.point_to[self.current_point]).distance(self.center) < Vector(self.velocity).length():
        self.current_point +=1
      else:
        self.velocity_volume = game.get_velocity(self)
        print(self.velocity_volume)
        vecto = (self.point_to[self.current_point][0] - self.center_x),(self.point_to[self.current_point][1] - self.center_y)
        self.velocity = Vector(vecto).normalize()*self.velocity_volume
        print(Vector(self.velocity).length())
        print(Vector(self.point_to[self.current_point]).distance(self.center))
        self.pos = Vector(*self.velocity) + self.pos                                                                               #
    
#    self.size[0] += self.velocity_x
#    self.size[1] += self.velocity_y
  def bounce_ball(self, ball):
    if self.collide_widget(ball):
      offset = Vector(ball.center_x-self.center_x, ball.center_y-self.center_y)
      self.velocity =Vector(*self.velocity).rotate(Vector(*self.velocity).angle(offset)/2)
      


class MapGame(Widget):
  source = StringProperty(None)
  runner = ObjectProperty(None)
  o_map = ObjectProperty(None)
  data = io.BytesIO(open("images/map640_480.png", "rb").read())
  im = CoreImage(data, ext="png", filename="images/map640_480.png")

  def serve_ball(self):
    self.runner.velocity = Vector(0, 0)

  def update(self,dt):
    self.runner.move(self)
    if (self.runner.y < 0) or (self.runner.top > self.height):
      self.runner.velocity_y*= -1
    # bounce off left and right
    if (self.runner.x < 0) or (self.runner.right > self.width):
      self.runner.velocity_x*= -1
    self.o_map.timer+=(1/60)

  def on_touch_move(self, touch):
    print("touch x,y ",touch.x,touch.y)
    self.runner.point_to.append((touch.x,touch.y))
    self.runner.point_number+=1

  def on_touch_down(self,touch):
    print("touch x,y ",touch.x,touch.y)
    self.runner.point_to.append((touch.x,touch.y))
    self.runner.point_number+=1
  def get_velocity(self,widget):
    pixel_color = self.im.read_pixel(widget.pos[0],widget.pos[1])
    print(pixel_color)
    velosity = pixel_color[1] # green sost
    print(velosity)
    return  velosity


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
