import socket
import struct
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from zbuffer import ZBuffer
from zsemaphore import ZSemaphore
from pack_util import PackUtil

class NetUtil(QThread):
    def __init__(self) -> None:
        super().__init__()

        # socket.setdefaulttimeout(3)
        self.connected = False
        self.zbuf = ZBuffer()
        self.zsems = {} #ZSemaphore()
        self.consumers = {}

        self.pack_util = PackUtil(self.zbuf, self.recv_pack) #从zbuf读区bytes流，得到完整pack后，调用recv_pack
        self.pack_util.start()

    def run(self):
        print('controller: reading tcp data started')
        while True:
            try:
                data = self.sock.recv(1024)
                if not data:
                    break
                # print(data)
                self.zbuf.write(data)
                # print(f'recv {len(data)} bytes {data}')
            except Exception as e:
                print(e)
                break
        self.connected = False
        print('controller: reading tcp data stopped')

    def send(self, buf):
        if not self.connected:
            # QMessageBox.warning(None, 'net error', 'net notconnected')
            return False

        try:
            self.sock.send(buf)
        except Exception as e:
            # QMessageBox.warning(None, 'socket error', str(e))
            return False
        return True

    def connect(self, ip, port):
        if self.connected:
            QMessageBox.warning(None, 'net error', 'already connected')
            return

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((ip, port))
            self.connected = True
            self.start()
        except Exception as e:
            QMessageBox.warning(None, 'socket error', str(e))
            return
        QMessageBox.information(None, 'OK', 'connect OK')

    def recv_pack(self, pack_type, pack):
        print('recv pack:', pack_type, pack)
        if pack_type in self.consumers.keys():
            self.consumers[pack_type](pack)
        else:
            if pack_type not in self.zsems.keys():
                self.zsems[pack_type] = ZSemaphore()
            self.zsems[pack_type].release(pack)

    def reg_pack_consumer(self, pack_type, consumer):
        self.consumers[pack_type] = consumer

    def acquire_pack(self, pack_type):
        wait_ms = 1000
        while pack_type not in self.zsems.keys():
            self.usleep(100000)
            wait_ms -= 100
            if wait_ms <= 0:
                return None
        pack = self.zsems[pack_type].try_acquire(wait_ms)
        return pack