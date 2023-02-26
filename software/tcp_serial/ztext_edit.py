from logging import Filter
from signal import signal
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *

class Filter(QObject):
    def __init__(self, parent=None, signal=None):
        super().__init__(parent)
        self.signal = signal
        self.ignore_enter = True
        self.shift_pressed = False

    def eventFilter(self, obj, e):
        if isinstance(e, QKeyEvent):
            # print(e, e.type(), f'[{e.text()}] [{e.key()}]')
            if e.text() == '\r':
                # print("11111", QEvent.KeyPress, QEvent.KeyRelease)
                if e.type() == QEvent.KeyRelease:
                    # print("press enter and release")
                    if not self.shift_pressed and self.signal is not None:
                        self.signal.emit()
                if not self.shift_pressed and self.ignore_enter:
                    return True
            elif e.key() == 16777248:
                if e.type() == QEvent.KeyRelease:
                    self.shift_pressed = False
                elif e.type() == QEvent.KeyPress:
                    self.shift_pressed = True
        return super().eventFilter(obj, e)

class ZTextEdit(QTextEdit):
    signal_enter_pressed = pyqtSignal()

    def __init__(self):
        super().__init__()
        # self.signal_press_enter.connect(self.slot_press_enter)
        self.filter = Filter(self, self.signal_enter_pressed)
        self.installEventFilter(self.filter)

    def ignore_enter(self, yes_no):
        self.filter.ignore_enter = yes_no

    # def slot_press_enter(self):
    #     print('press enter done')