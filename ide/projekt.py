import io

import six

import packaging

import packaging.version

import packaging.specifiers

import packaging.requirements

from kivy.app import App
from kivy.lang import Builder
from kivy.uix.widget import Widget
from kivy.uix.button import Button
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.popup import Popup
from kivy.uix.settings import (Settings, SettingsWithSidebar,
                               SettingsWithSpinner,
                               SettingsWithTabbedPanel)
from kivy.properties import OptionProperty, ObjectProperty





class TestApp(App):
    display_type = OptionProperty('normal', options=['normal', 'popup'])
    layout = BoxLayout(orientation='vertical')
    buttons = BoxLayout(orientation='horizontal')

    def build(self):
        paneltype = Label(text='What kind of settings panel to use?')
        sidebar_button = Button(text='Sidebar')
        sidebar_button.bind(on_press=lambda j: self.set_settings_cls())
        self.buttons.add_widget(sidebar_button)
        self.layout.add_widget(paneltype)
        self.layout.add_widget(self.buttons)
        return self.layout
    def set_settings_cls(self):
        sibebar_button = Button(text='Sibebar1')
        self.layout.add_widget(sibebar_button)
        return self.layout
if __name__ == '__main__':
    TestApp().run()
