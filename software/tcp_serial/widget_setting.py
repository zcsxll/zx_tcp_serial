from tabnanny import check
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import struct

from uart_config import get_uart_config

class WidgetSetting(QWidget):
    def __init__(self, net_util, parent=None):
        super().__init__(parent)

        self.net_util = net_util
        gbox_net = self.init_group_net()
        gbox_uart = self.init_group_uart()
        # gbox_common = self.init_group_common()

        vbox = QVBoxLayout()
        vbox.addWidget(gbox_net)
        vbox.addWidget(gbox_uart)
        # vbox.addWidget(gbox_common)
        self.setLayout(vbox)

    def init_group_net(self):
        self.le_ip = QLineEdit('192.168.31.124')
        # self.le_ip = QLineEdit('127.0.0.1')
        self.le_port = QLineEdit('8899')
        self.le_port.setMaximumWidth(70)
        self.btn_connect = QPushButton('连接')
        self.btn_connect.setMaximumWidth(100)
        self.btn_connect.clicked.connect(lambda: self.btn_clicked('connect'))
        hbox_ip_port = QHBoxLayout()
        hbox_ip_port.addWidget(QLabel('IP:'))
        hbox_ip_port.addWidget(self.le_ip)
        hbox_ip_port.addWidget(QLabel('PORT:'))
        hbox_ip_port.addWidget(self.le_port)
        hbox_ip_port.addWidget(self.btn_connect)
        hbox_ip_port.addStretch()

        gbox = QGroupBox()
        # gbox.setTitle('网络')
        gbox.setLayout(hbox_ip_port)
        return gbox

    def init_group_uart(self):
        self.cb_baudrate1 = QComboBox()
        self.cb_baudrate1.setEditable(True)
        # self.combo_box_baudrate.setValidator(QIntValidator(0, 9))
        self.cb_baudrate1.addItems(('115200', '9600', '19200', '1500000'))
        self.cb_data_bits1 = QComboBox()
        self.cb_data_bits1.addItem('8')
        self.cb_check_bit1 = QComboBox()
        self.cb_check_bit1.addItem('N')
        self.cb_stop_bit1 = QComboBox()
        self.cb_stop_bit1.addItem('1')
        btn_set_uart1 = QPushButton('更新')
        btn_set_uart1.clicked.connect(lambda : self.btn_clicked('set_uart'))
        hbox_uart_setting1 = QHBoxLayout()
        hbox_uart_setting1.addWidget(QLabel('串口: 波特率'))
        hbox_uart_setting1.addWidget(self.cb_baudrate1)
        hbox_uart_setting1.addWidget(QLabel('数据位'))
        hbox_uart_setting1.addWidget(self.cb_data_bits1)
        hbox_uart_setting1.addWidget(QLabel('校验位'))
        hbox_uart_setting1.addWidget(self.cb_check_bit1)
        hbox_uart_setting1.addWidget(QLabel('停止位'))
        hbox_uart_setting1.addWidget(self.cb_stop_bit1)
        hbox_uart_setting1.addWidget(btn_set_uart1)
        hbox_uart_setting1.addStretch()

        # self.cb_baudrate2 = QComboBox()
        # self.cb_baudrate2.setEditable(True)
        # # self.combo_box_baudrate.setValidator(QIntValidator(0, 9))
        # self.cb_baudrate2.addItems(('115200', '9600', '19200', '1500000'))
        # self.cb_data_bits2 = QComboBox()
        # self.cb_data_bits2.addItem('8')
        # self.cb_check_bit2 = QComboBox()
        # self.cb_check_bit2.addItem('N')
        # self.cb_stop_bit2 = QComboBox()
        # self.cb_stop_bit2.addItem('1')
        # btn_set_uart2 = QPushButton('更新')
        # btn_set_uart2.clicked.connect(lambda : self.btn_clicked('set_uart2'))
        # hbox_uart_setting2 = QHBoxLayout()
        # hbox_uart_setting2.addWidget(QLabel('串口2: 波特率'))
        # hbox_uart_setting2.addWidget(self.cb_baudrate2)
        # hbox_uart_setting2.addWidget(QLabel('数据位'))
        # hbox_uart_setting2.addWidget(self.cb_data_bits2)
        # hbox_uart_setting2.addWidget(QLabel('校验位'))
        # hbox_uart_setting2.addWidget(self.cb_check_bit2)
        # hbox_uart_setting2.addWidget(QLabel('停止位'))
        # hbox_uart_setting2.addWidget(self.cb_stop_bit2)
        # hbox_uart_setting2.addWidget(btn_set_uart2)
        # hbox_uart_setting2.addStretch()

        vbox = QVBoxLayout()
        vbox.addLayout(hbox_uart_setting1)
        # vbox.addLayout(hbox_uart_setting2)

        gbox = QGroupBox()
        gbox.setLayout(vbox)
        return gbox

    # def init_group_common(self):
        # self.rb_send_by_enter = QRadioButton('回车键')
        # self.rb_send_by_btn = QRadioButton('按钮')
        # self.rb_send_by_enter.setChecked(True)
        # group_send = QButtonGroup(self)
        # group_send.addButton(self.rb_send_by_enter)
        # group_send.addButton(self.rb_send_by_btn)
        # hbox_send = QHBoxLayout()
        # hbox_send.addWidget(QLabel('发送方式:'))
        # hbox_send.addWidget(self.rb_send_by_enter)
        # hbox_send.addWidget(self.rb_send_by_btn)
        # hbox_send.addStretch()


    #     self.rb_append_none = QRadioButton('不追加')
    #     self.rb_append_n = QRadioButton('追加\\n')
    #     self.rb_append_rn = QRadioButton('追加\\r\\n')
    #     self.rb_append_none.setChecked(True)
    #     group_append = QButtonGroup(self)
    #     group_append.addButton(self.rb_append_none)
    #     group_append.addButton(self.rb_append_n)
    #     group_append.addButton(self.rb_append_rn)
    #     hbox_append = QHBoxLayout()
    #     hbox_append.addWidget(QLabel('发送末尾:'))
    #     hbox_append.addWidget(self.rb_append_none)
    #     hbox_append.addWidget(self.rb_append_n)
    #     hbox_append.addWidget(self.rb_append_rn)
    #     hbox_append.addStretch()

    #     self.rb_format_ascii = QRadioButton('ASCII')
    #     self.rb_format_hex = QRadioButton('HEX')
    #     self.rb_format_ascii.setChecked(True)
    #     group_format = QButtonGroup(self)
    #     group_format.addButton(self.rb_format_ascii)
    #     group_format.addButton(self.rb_format_hex)
    #     group_format.buttonClicked.connect(lambda: self.btn_clicked('set_format'))
    #     hbox_format = QHBoxLayout()
    #     hbox_format.addWidget(QLabel('显示格式:'))
    #     hbox_format.addWidget(self.rb_format_ascii)
    #     hbox_format.addWidget(self.rb_format_hex)
    #     hbox_format.addStretch()

    #     vbox = QVBoxLayout()
    #     vbox.addLayout(hbox_send)
    #     vbox.addLayout(hbox_append)
    #     vbox.addLayout(hbox_format)

    #     gbox = QGroupBox()
    #     gbox.setLayout(vbox)
    #     return gbox

    def set_uart_config(self, baudrate, data_bit, check_bit, stop_bit):
        print('set uart config:', baudrate, data_bit, check_bit, stop_bit)
        # if not self.connected:
        #     return False, 'not connected'
        uart_conf = get_uart_config(data_bit, check_bit, stop_bit)
        pack = b'XZ\x01\x08' #01 means set uart config, 08 is pack len
        pack += struct.pack('II', baudrate, uart_conf)
        if not self.net_util.send(pack):
            return

        ret = self.net_util.acquire_pack(0x01)
        if ret is None:
            QMessageBox.warning(self, 'Fail', 'net error, no pack(type=1) received')
            return
        QMessageBox.information(self, 'OK', 'result: ' + ret.decode('utf-8'))

    def btn_clicked(self, arg):
        if arg == 'connect':
            ip = self.le_ip.text()
            port = int(self.le_port.text())
            self.net_util.connect(ip, port)
        elif arg == 'set_uart':
            baudrate = int(self.cb_baudrate1.currentText())
            data_bit = int(self.cb_data_bits1.currentText())
            check_bit = self.cb_check_bit1.currentText()
            stop_bit = int(self.cb_stop_bit1.currentText())
            self.set_uart_config(baudrate, data_bit, check_bit, stop_bit)
        # elif arg == 'set_uart2':
        #     baudrate = int(self.cb_baudrate2.currentText())
        #     data_bit = int(self.cb_data_bits2.currentText())
        #     check_bit = self.cb_check_bit2.currentText()
        #     stop_bit = int(self.cb_stop_bit2.currentText())
        #     self.signal.emit(['set_uart', 1, baudrate, data_bit, check_bit, stop_bit])
        # elif arg == 'set_format':
        #     if self.rb_format_ascii.isChecked():
        #         self.signal.emit(['set_format', 'ascii'])
        #     else:
        #         self.signal.emit(['set_format', 'hex'])
