import struct
from PyQt5.QtCore import *

class PackUtil(QThread):
    def __init__(self, zbuf, recv_pack) -> None:
        super().__init__()
        self.zbuf = zbuf
        self.recv_pack = recv_pack

    def run(self):
        state = 0
        while True:
            if state == 0:
                c = self.zbuf.read(1)
                if c == b'Z':
                    state = 1
            elif state == 1:
                c = self.zbuf.read(1)
                if c == b'X':
                    state = 2
            elif state == 2:
                data = self.zbuf.read(2)
                pack_type, pack_len = struct.unpack('BB', data)
                pack = self.zbuf.read(pack_len)
                self.recv_pack(pack_type, pack)
                state = 0