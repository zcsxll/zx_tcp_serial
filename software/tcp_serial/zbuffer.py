import threading

class ZBuffer:
    def __init__(self, max_n_packs=1024):
        self.max_n_packs = max_n_packs
        self.packs = []
        
        self.sem = threading.Semaphore(0)
        self.lock = threading.Lock()

    def read(self, n_bytes, pre_data=b''):
        if n_bytes <= 0:
            return b''
            
        self.sem.acquire()
        self.lock.acquire()

        if n_bytes < len(self.packs[0]):
            data = self.packs[0][:n_bytes]
            self.packs[0] = self.packs[0][n_bytes:]
            self.sem.release()
            self.lock.release()
            return pre_data + data
        
        if n_bytes == len(self.packs[0]):
            data = self.packs.pop(0)
            self.lock.release()
            return pre_data + data

        if n_bytes > len(self.packs[0]):
            data = self.packs.pop(0)
            self.lock.release()
            return self.read(n_bytes - len(data), pre_data + data)

    # def peek(self):
    #     self.sem.acquire()
    #     self.lock.acquire()
    #     peek = self.packs[0]
    #     self.sem.release()
    #     self.lock.release()

    #     return peek

    def write(self, data):
        self.lock.acquire()
        self.packs.append(data)
        self.sem.release(1)
        self.lock.release()

if __name__ == '__main__':
    import time

    class MyThread(threading.Thread):
        def __init__(self, zbuffer):
            super().__init__()

            self.zbuffer = zbuffer

        def run(self):
            i = 0
            while True:
                self.zbuffer.write(i.to_bytes(4, byteorder='little', signed=False))
                i += 1
                time.sleep(0.1)

    zbuffer = ZBuffer()
    thread = MyThread(zbuffer)
    thread.start()
    # thread.join()

    # print('end')
    total_data = b''
    while True:
        data = zbuffer.read(11)
        total_data += data
        print(data, len(total_data))
        if len(total_data) >= 100:
            break

    for i in range(len(total_data) // 4):
        data = total_data[i * 4: i * 4 + 4]
        i = int.from_bytes(data, byteorder='little', signed=False)
        print(i)
