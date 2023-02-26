import sys
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *

from net_util import NetUtil
from widget_setting import WidgetSetting
from widget_uart import WidgetUart
from widget_i2c import WidgetI2C

class WidgetMain(QMainWindow):
    def __init__(self):
        super().__init__()
        self.net_util = NetUtil()

        self.init_ui()
        self.net_util.reg_pack_consumer(0x02, self.tab_uart.on_data) #注册需要主动上报的数据，如串口接受的数据
        self.show()

    def init_ui(self):
        self.tab_setting = WidgetSetting(self.net_util)
        self.tab_uart = WidgetUart(self.net_util)
        self.tab_i2c = WidgetI2C(self.net_util)
        self.tab_widget = QTabWidget()
        self.tab_widget.addTab(self.tab_setting, "设置")
        self.tab_widget.addTab(self.tab_uart, "串口")
        self.tab_widget.addTab(self.tab_i2c, "I2C")
        self.setCentralWidget(self.tab_widget)

        self.resize(600, 400)
        self.move(300, 100)
        self.setWindowTitle("TCP Serial")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    wm = WidgetMain()
    sys.exit(app.exec_())
