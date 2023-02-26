from threading import Thread
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import struct
import time

from zvscroll_area import ZVScrollArea

class ZWidgetBase(QWidget):
    def __init__(self, id, signal, suffix):
        super().__init__()

        self.id = id
        self.signal = signal
        self.suffix = suffix

        self.color = QColor(0x99aa00) if id % 2 == 0 else QColor(0x0099aa)
        pe = QPalette()
        pe.setColor(QPalette.ColorRole.WindowText, QColor(0x660099))
        # pe.setColor(QPalette.ColorRole.Base, QColor(0x334400))
        
        self.label_id = QLabel(f'%03d {self.suffix}' % id)
        self.label_id.setPalette(pe)
        # self.le_data = QLineEdit()
        btn_delete = QPushButton('删除')
        btn_delete.setMaximumWidth(50)
        btn_delete.clicked.connect(lambda: self.btn_clicked('delete'))

        hbox = QHBoxLayout()
        hbox.addWidget(self.label_id)
        # hbox.addWidget(QLabel('DATA:'))
        # hbox.addWidget(self.le_data)
        hbox.addWidget(btn_delete)
        # hbox.addStretch()

        self.setLayout(hbox)
        self.setFixedHeight(36)

    def paintEvent(self, event):
        rect = event.region().rects()[0]
        painter = QPainter(self)
        painter.setPen(QPen(self.color, 1))
        painter.setBrush(QBrush(self.color))
        painter.drawRect(rect)

    def btn_clicked(self, cmd):
        if cmd == 'delete':
            self.signal.emit(cmd, self.id)

    def set_id(self, id):
        self.id = id
        self.color = QColor(0x99aa00) if id % 2 == 0 else QColor(0x0099aa)
        self.label_id.setText(f'%03d {self.suffix}' % id)

class WidgetWriteBuf(ZWidgetBase):
    def __init__(self, id, signal, fp=None):
        super().__init__(id, signal, 'W')
        
        self.le_data = QLineEdit('11 22 33')

        hbox = self.layout()
        hbox.insertWidget(1, QLabel('DATA:'))
        hbox.insertWidget(2, self.le_data)

        if fp is not None:
            data_len = fp.read(4)
            data_len = struct.unpack('i', data_len)[0]
            data = fp.read(data_len)
            s = ''
            for d in data:
                s += '%02x ' % d
            self.le_data.setText(s)

    def dump(self):
        ret = b'\x11' #0x11 means write data
        data = bytes.fromhex(self.le_data.text())
        if len(data) <= 0:
            raise Exception('Empty data')
        ret += struct.pack('i', len(data))
        ret += data
        return ret

class WidgetWriteAddrBuf(ZWidgetBase):
    def __init__(self, id, signal, fp=None):
        super().__init__(id, signal, 'W')

        self.le_addr = QLineEdit('34')
        self.le_addr.setMaximumWidth(30)
        self.le_data = QLineEdit('66 77 88')

        hbox = self.layout()
        hbox.insertWidget(1, QLabel('ADDR:'))
        hbox.insertWidget(2, self.le_addr)
        hbox.insertWidget(3, QLabel('DATA:'))
        hbox.insertWidget(4, self.le_data)

        if fp is not None:
            addr = fp.read(1)
            data_len = fp.read(4)
            data_len = struct.unpack('i', data_len)[0]
            data = fp.read(data_len)
            self.le_addr.setText('%02x' % addr[0])
            s = ''
            for d in data:
                s += '%02x ' % d
            self.le_data.setText(s)

    def dump(self):
        ret = b'\x12' #0x12 means write data with addr
        addr = bytes.fromhex(self.le_addr.text())
        data = bytes.fromhex(self.le_data.text())
        if len(addr) != 1:
            raise Exception('Wrong addr')
        if len(data) <= 0:
            raise Exception('Wrong data')
        ret += addr
        ret += struct.pack('i', len(data))
        ret += data
        return ret

class WidgetRead(ZWidgetBase):
    def __init__(self, id, signal, fp=None):
        super().__init__(id, signal, 'R')

        self.le_addr = QLineEdit('99')
        self.le_addr.setMaximumWidth(30)
        self.le_len = QLineEdit('66')
        self.le_len.setMaximumWidth(30)
        self.le_ret = QLineEdit('')
        self.le_ret.setReadOnly(True)

        hbox = self.layout()
        hbox.insertWidget(1, QLabel('ADDR:'))
        hbox.insertWidget(2, self.le_addr)
        hbox.insertWidget(3, QLabel('LEN:'))
        hbox.insertWidget(4, self.le_len)
        hbox.insertWidget(5, QLabel('RET:'))
        hbox.insertWidget(6, self.le_ret)
        # hbox.insertStretch(7)

        if fp is not None:
            addr = fp.read(1)
            length = fp.read(4)
            length = struct.unpack('i', length)[0]
            self.le_addr.setText('%02x' % addr[0])
            self.le_len.setText('%d' % length)

    def dump(self):
        ret = b'\x13' #0x13 means read data with addr
        addr = bytes.fromhex(self.le_addr.text())
        length = int(self.le_len.text(), 10)
        if len(addr) != 1:
            raise Exception('Wrong addr')
        if length <= 0:
            raise Exception('Wrong length')
        ret += addr
        ret += struct.pack('i', length)
        return ret

    def set_data(self, data):
        s = ''
        for d in data:
            s += '%02x ' % d
        self.le_ret.setText(s)

class ThreadExec(QThread):
    def __init__(self, signal, net_util, slave_addr, delay_ms, widgets):
        super().__init__()
        self.signal = signal
        self.net_util = net_util
        self.slave_addr = slave_addr
        self.delay_ms = delay_ms
        self.widgets = widgets
        self.running = False

    def run(self):
        self.running = True
        self.exec()
        self.running = False
        self.signal.emit('exec done')

    def exec(self):
        for i in range(self.widgets.count()):
            item = self.widgets.at(i)
            # print(f'exec {i}')
            self.signal.emit(f'executing {i}')
            data = item.widget().dump()
            type = data[0:1]
            data = data[1:]
            data = self.slave_addr + data
            # print(data)
            
            pack = b'XZ' + type
            pack += struct.pack('B', len(data))
            pack += data
            if not self.net_util.send(pack):
                self.signal.emit('net error')
                return
            ret = self.net_util.acquire_pack(type[0])
            if ret is None:
                # QMessageBox.warning(None, 'Fail', 'net error, no pack(type=0x%02x) received' % type[0])
                self.signal.emit('net error, no pack(type=0x%02x) received' % type[0])
                return
            if type[0] in [0x11, 0x12]:
                if ret[0] != 0:
                    # QMessageBox.warning(None, 'Fail', f'Failed to write I2C: ({ret[0]})')
                    self.signal.emit(f'Failed to write I2C: ({ret[0]})')
                    return
            elif type[0] == 0x13:
                item.widget().set_data(ret)
                if len(ret) == 0:
                    # QMessageBox.warning(None, 'Fail', 'Failed to read I2C no data read')
                    self.signal.emit('Failed to read I2C no data read')
                    return
            else:
                # QMessageBox.warning(None, 'Fail', f'Failed to write I2C: {ret}')
                self.signal.emit(f'Failed to write I2C: {ret}')
                return
            time.sleep(self.delay_ms)

class WidgetI2C(QWidget):
    signal_cmd = pyqtSignal(str, int)
    signal_status = pyqtSignal(str)

    def __init__(self, net_util):
        super().__init__()
        self.net_util = net_util
        self.init_ui()
        self.signal_cmd.connect(self.slot_cmd)
        self.signal_status.connect(self.slot_status)
        self.thread_exec = None

    def init_ui(self):
        self.widgets = ZVScrollArea(self)
        self.widgets.add_widget(WidgetWriteBuf(0, self.signal_cmd))

        self.le_slave_addr = QLineEdit('')
        self.le_slave_addr.setMaximumWidth(40)
        self.le_delay = QLineEdit('200')
        self.le_delay.setMaximumWidth(60)
        hbox_addr = QHBoxLayout()
        hbox_addr.addWidget(QLabel('I2C子设备地址'))
        hbox_addr.addWidget(self.le_slave_addr)
        hbox_addr.addStretch()
        hbox_addr.addWidget(QLabel('延时(毫秒)'))
        hbox_addr.addWidget(self.le_delay)
        hbox_addr.addStretch()

        btn_add_w_buf = QPushButton('添加[写数据]')
        btn_add_w_buf.clicked.connect(lambda: self.btn_clicked('add_w_buf'))
        btn_add_w_addr_buf = QPushButton('添加[写地址和数据]')
        btn_add_w_addr_buf.clicked.connect(lambda: self.btn_clicked('add_w_addr_buf'))
        btn_add_r_buf = QPushButton('添加[读数据]')
        btn_add_r_buf.clicked.connect(lambda: self.btn_clicked('add_r'))
        btn_save = QPushButton('保存')
        btn_save.clicked.connect(lambda: self.btn_clicked('save'))
        btn_load = QPushButton('加载')
        btn_load.clicked.connect(lambda: self.btn_clicked('load'))
        btn_exec = QPushButton('执行')
        btn_exec.clicked.connect(lambda: self.btn_clicked('exec'))
        hbox_btn = QHBoxLayout()
        hbox_btn.addWidget(btn_add_w_buf)
        hbox_btn.addWidget(btn_add_w_addr_buf)
        hbox_btn.addWidget(btn_add_r_buf)
        hbox_btn.addWidget(btn_save)
        hbox_btn.addWidget(btn_load)
        hbox_btn.addWidget(btn_exec)

        self.label_status = QLabel('Ready')
        
        vbox = QVBoxLayout()
        vbox.addLayout(hbox_addr)
        vbox.addLayout(hbox_btn)
        vbox.addWidget(self.widgets)
        vbox.addWidget(self.label_status)
        # vbox.addStretch()

        self.setLayout(vbox)

    def slot_cmd(self, cmd, id):
        if cmd == 'delete':
            self.widgets.delete(id)

    def slot_status(self, status):
        self.label_status.setText(status)

    def check(self):
        try:
            slave_addr = bytes.fromhex(self.le_slave_addr.text())
            if len(slave_addr) != 1:
                raise Exception('Wrong slave addr')
        except Exception as e:
            QMessageBox.warning(self, '错误', '[Slave Addr] ' + str(e))
            return False

        for i in range(self.widgets.count()):
            item = self.widgets.at(i)
            # print(item.widget()
            try:
                data = item.widget().dump()
            except Exception as e:
                QMessageBox.warning(self, '错误', f'[Operation {i}] ' + str(e))
                return False
        return True

    def save(self):
        if not self.check():
            return
        path = QDir.currentPath() + '/i2c_confs'
        file_path, filter = QFileDialog.getSaveFileName(self, '保存配置', path, 'I2C配置文件(*.i2c_conf)')
        if not file_path.endswith('i2c_conf'):
            return
        # file_path = './i2c_confs/test.i2c_conf'
        with open(file_path, 'wb') as fp:
            slave_addr = bytes.fromhex(self.le_slave_addr.text())
            fp.write(slave_addr)
            for i in range(self.widgets.count()):
                item = self.widgets.at(i)
                # print(item.widget()
                data = item.widget().dump()
                fp.write(data)

    def load(self):
        path = QDir.currentPath() + '/i2c_confs'
        file_path, filter = QFileDialog.getOpenFileName(self, '加载配置', path, 'I2C配置文件(*.i2c_conf)')
        if not file_path.endswith('i2c_conf'):
            return
        # file_path = './i2c_confs/test.i2c_conf'
        while self.widgets.count() > 0:
            self.widgets.delete(0)

        with open(file_path, 'rb') as fp:
            slave_addr = fp.read(1)
            self.le_slave_addr.setText('%02x' % slave_addr[0])
            while True:
                type = fp.read(1)
                if type == b'':
                    break
                print(f'read type: %02x' % type[0])
                if type == b'\x11':
                    self.widgets.add_widget(WidgetWriteBuf(self.widgets.count(), self.signal_cmd, fp))
                elif type == b'\x12':
                    self.widgets.add_widget(WidgetWriteAddrBuf(self.widgets.count(), self.signal_cmd, fp))
                elif type == b'\x13':
                    self.widgets.add_widget(WidgetRead(self.widgets.count(), self.signal_cmd, fp))

    def exec(self):
        if not self.check():
            return
        if self.thread_exec is not None and self.thread_exec.running:
            print('already running')
            return
        slave_addr = bytes.fromhex(self.le_slave_addr.text())
        delay_ms = float(self.le_delay.text()) / 1000
        self.thread_exec = ThreadExec(self.signal_status, self.net_util, slave_addr, delay_ms, self.widgets)
        self.thread_exec.start()
        # print(self.net_util)
        # for i in range(self.widgets.count()):
        #     item = self.widgets.at(i)
        #     # print(f'exec {i}')
        #     self.status.setText(f'executing {i}')
        #     data = item.widget().dump()
        #     type = data[0:1]
        #     data = data[1:]
        #     data = slave_addr + data
        #     # print(data)
            
        #     pack = b'XZ' + type
        #     pack += struct.pack('B', len(data))
        #     pack += data
        #     if not self.net_util.send(pack):
        #         break
        #     ret = self.net_util.acquire_pack(type[0])
        #     if ret is None:
        #         QMessageBox.warning(self, 'Fail', 'net error, no pack(type=0x%02x) received' % type[0])
        #         return
        #     if type[0] in [0x11, 0x12]:
        #         if ret[0] != 0:
        #             QMessageBox.warning(self, 'Fail', f'Failed to write I2C: ({ret[0]})')
        #             return
        #     elif type[0] == 0x13:
        #         item.widget().set_data(ret)
        #         if len(ret) == 0:
        #             QMessageBox.warning(self, 'Fail', 'Failed to read I2C no data read')
        #             return
        #     else:
        #         QMessageBox.warning(self, 'Fail', f'Failed to write I2C: {ret}')
        #         return
        #     time.sleep(float(self.le_delay.text()) / 1000)

    def btn_clicked(self, cmd):
        if cmd == 'add_w_buf':
            self.widgets.add_widget(WidgetWriteBuf(self.widgets.count(), self.signal_cmd))
        elif cmd == 'add_w_addr_buf':
            self.widgets.add_widget(WidgetWriteAddrBuf(self.widgets.count(), self.signal_cmd))
        elif cmd == 'add_r':
            self.widgets.add_widget(WidgetRead(self.widgets.count(), self.signal_cmd))
        elif cmd == 'save':
            self.save()
        elif cmd == 'load':
            self.load()
        elif cmd =='exec':
            self.exec()

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    widget = WidgetI2C(None)
    widget.show()
    sys.exit(app.exec())