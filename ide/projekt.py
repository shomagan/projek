import io

import six

from kivy.app import App
from kivy.lang import Builder
from kivy.uix.widget import Widget
from kivy.uix.button import Button
from kivy.uix.switch import Switch
from kivy.uix.textinput import TextInput

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.popup import Popup
from kivy.uix.settings import (Settings, SettingsWithSidebar,
                               SettingsWithSpinner,
                               SettingsWithTabbedPanel)
from kivy.properties import OptionProperty, ObjectProperty





class TestApp(App):
    display_type = OptionProperty('normal', options=['normal', 'popup'])
    layout = GridLayout()
    layout.cols = 2
    button_number = 0

    def build(self):
        textinput = TextInput(text='Hello world')
        sidebar_button = Button(text='Sidebar')
        sidebar_button.size_hint_x = None
        sidebar_button.bind(on_press=lambda j: self.set_settings_cls())
        self.layout.add_widget(sidebar_button)
        self.layout.add_widget(textinput)
        return self.layout
    def set_settings_cls(self):
        sibebar_button = Button(text='Sibebar'+str(self.button_number))
        sibebar_button.size_hint_x = None
        led_switch = Switch(active=False)
        self.layout.add_widget(sibebar_button)
        self.layout.add_widget(led_switch)
        sibebar_button.bind(on_press=lambda j: self.set_settings_cls())
        self.button_number+=1
        return self.layout
if __name__ == '__main__':
    TestApp().run()
