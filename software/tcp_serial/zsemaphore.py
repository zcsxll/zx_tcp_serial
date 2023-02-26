from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *

class ZSemaphore(QSemaphore):
    def __init__(self):
        super().__init__(0)
        self.data = []

    def acquire(self):
        super().acquire()
        return self.data.pop(0)

    def try_acquire(self, timeout):
        ret = super().tryAcquire(1, timeout)
        return self.data.pop(0) if ret else None

    def release(self, data, pos=-1):
        if pos == -1:
            self.data.append(data)
        else:
            self.data.insert(pos, data)
        super().release()

    def clear(self):
        while self.try_acquire(1) is not None:
            pass

if __name__ == '__main__':
    import time
    class ZcsThread(QThread):
        def __init__(self, zsem):
            super(ZcsThread, self).__init__()
            self.zsem = zsem

        def run(self):
            while True:
                print('wait')
                data = self.zsem.try_acquire(1000)
                print('got:', data)
                # break

    zsem = ZSemaphore()

    zthread = ZcsThread(zsem)
    zthread.start()

    time.sleep(5)
    zsem.release('sdsd444')
    # zsem.release('sdsd1')
    # time.sleep(1)
    # zsem.release('sdsd2')
    time.sleep(0.5)
    print('end')