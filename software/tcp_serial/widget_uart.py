from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import struct

from ztext_edit import ZTextEdit

class WidgetUart(QWidget):
    signal = pyqtSignal(bytes)

    def __init__(self, net_util):
        super().__init__()
        self.init_ui()
        
        self.net_util = net_util
        self.signal.connect(self.slot)

        self.show_ascii = True
        self.send_ascii = True
        self.auto_move = True

    def init_ui(self):
        self.rb_show_ascii = QRadioButton('ASCII')
        self.rb_show_hex = QRadioButton('HEX')
        self.rb_show_ascii.setChecked(True)
        group_show = QButtonGroup(self)
        group_show.addButton(self.rb_show_ascii)
        group_show.addButton(self.rb_show_hex)
        group_show.buttonClicked.connect(lambda: self.btn_clicked('set_show_format'))
        hbox_show = QHBoxLayout()
        hbox_show.addWidget(QLabel('显示格式:'))
        hbox_show.addWidget(self.rb_show_ascii)
        hbox_show.addWidget(self.rb_show_hex)
        hbox_show.addStretch()
        self.te_in = ZTextEdit()
        self.te_in.setFixedHeight(60)

        self.te_out = QTextEdit()
        self.te_out.setReadOnly(True)
        self.te_out.verticalScrollBar().valueChanged.connect(self.slot_vsb)

        self.rb_send_ascii = QRadioButton('ASCII')
        self.rb_send_hex = QRadioButton('HEX')
        self.rb_send_ascii.setChecked(True)
        group_sending_format = QButtonGroup(self)
        group_sending_format.addButton(self.rb_send_ascii)
        group_sending_format.addButton(self.rb_send_hex)
        group_sending_format.buttonClicked.connect(lambda: self.btn_clicked('set_sending_format'))

        self.rb_send_by_enter = QRadioButton('回车')
        self.rb_send_by_btn = QRadioButton('按钮')
        self.rb_send_by_enter.setChecked(True)
        group_sending_way = QButtonGroup(self)
        group_sending_way.addButton(self.rb_send_by_enter)
        group_sending_way.addButton(self.rb_send_by_btn)
        group_sending_way.buttonClicked.connect(lambda: self.btn_clicked('set_sending_way'))

        self.btn_send = QPushButton('发送')
        self.btn_send.clicked.connect(lambda: self.btn_clicked('send'))
        self.btn_send.setVisible(False)
        
        hbox_send = QHBoxLayout()
        hbox_send.addWidget(QLabel('发送格式:'))
        hbox_send.addWidget(self.rb_send_ascii)
        hbox_send.addWidget(self.rb_send_hex)
        hbox_send.addStretch()
        hbox_send.addWidget(QLabel('发送方式:'))
        hbox_send.addWidget(self.rb_send_by_enter)
        hbox_send.addWidget(self.rb_send_by_btn)
        hbox_send.addStretch()
        hbox_send.addWidget(self.btn_send)
        hbox_send.addStretch()
        self.te_in = ZTextEdit()
        self.te_in.setFixedHeight(60)
        self.te_in.signal_enter_pressed.connect(lambda: self.btn_clicked('enter_pressed'))

        # self.te_out.append('1234')
        # self.te_out.append('1234')
        # self.te_out.append('11 22 33 44')
        # self.te_out.append('11 22 33 44 66 ff')

        vbox = QVBoxLayout()
        vbox.addLayout(hbox_show)
        vbox.addWidget(self.te_out)
        vbox.addLayout(hbox_send)
        vbox.addWidget(self.te_in)

        self.setLayout(vbox)

    def to_ascii(self, data):
        text_ascii = ''
        for d in data:
            try:
                text_ascii += chr(d)
            except Exception as e:
                text_ascii += '?'
        return text_ascii

    def on_data(self, data): #该函数位于非ui线程
        # self.te_out.append(data.decode('utf-8'))
        self.signal.emit(data)

    def slot(self, data):
        if self.show_ascii:
            text_ascii = self.to_ascii(data)
            self.te_out.insertPlainText(text_ascii)
        else:
            text_hex = ''
            for c in data:
                text_hex += (' %02X' % c)
            if self.te_out.toPlainText() == '':
                self.te_out.insertPlainText(text_hex[1:])
            else:
                self.te_out.insertPlainText(text_hex)

        if self.auto_move:
            self.te_out.verticalScrollBar().setValue(self.te_out.verticalScrollBar().maximum())

    def slot_vsb(self, value):
        if value == self.te_out.verticalScrollBar().maximum():
            self.auto_move = True
        else:
            self.auto_move = False

    def btn_clicked(self, cmd):
        if cmd == 'set_show_format':
            self.show_ascii = self.rb_show_ascii.isChecked()
        elif cmd == 'set_sending_format':
            self.set_sending_format()
        elif cmd == 'set_sending_way':
            if self.rb_send_by_btn.isChecked():
                self.btn_send.setVisible(True)
                self.te_in.ignore_enter(False)
            else:
                self.btn_send.setVisible(False)
                self.te_in.ignore_enter(True)
        elif cmd == 'send':
            # print('send by btn')
            self.send_data()
        elif cmd == 'enter_pressed':
            if not self.rb_send_by_enter.isChecked():
                return
            # print('send by enter')
            self.send_data()

    def set_sending_format(self):
        if self.rb_send_ascii.isChecked() and not self.send_ascii:
            # print('send ascii')
            hex = self.te_in.toPlainText()
            try:
                data = bytes.fromhex(hex)
                # print(data)
                ascii = ''
                for d in data:
                    if d < 0x20 or d > 0x7e:
                        raise Exception('存在不可显字符')
                    ascii += chr(d)
                self.te_in.setPlainText(ascii)
                self.send_ascii = True
            except Exception as e:
                self.rb_send_hex.setChecked(True)
                QMessageBox.warning(self, 'warning', str(e))

        elif self.rb_send_hex.isChecked() and self.send_ascii:
            self.send_ascii = False
            # print('send hex')
            ascii = self.te_in.toPlainText()
            hex = ''
            for a in ascii:
                hex += (' %02x' % ord(a))
            self.te_in.setPlainText(hex[1:])

    def send_data(self):
        if self.rb_send_ascii.isChecked():
            self.send_ascii_data()
        else:
            self.send_hex_data()

    def send_ascii_data(self):
        ascii = self.te_in.toPlainText()
        data = ascii.encode('utf-8')
        if data != b'':
            # print(f'send [{data}]')
            self.send_uart_data(data)

    def send_hex_data(self):
        hex = self.te_in.toPlainText()
        try:
            data = bytes.fromhex(hex)
            if data != b'':
                # print(f'send [{data}]')
                self.send_uart_data(data)
        except Exception as e:
            QMessageBox.warning(self, 'warning', str(e))

    def send_uart_data(self, data):
        if len(data) > 255:
            QMessageBox.warning(self, 'data error', f'data length({len(data)}) > 255')
            return
            
        pack = b'XZ\x02' #02 means send data to uart
        pack += struct.pack('B', len(data))
        pack += data
        self.net_util.send(pack)
        
if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    widget = WidgetUart(None)
    widget.show()
    sys.exit(app.exec())