package com.zx.netelf;

import java.util.LinkedList;
import java.util.Queue;
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class MySemaphore extends Semaphore {
    public MySemaphore(int permits) {
        super(permits);

        lock_ = new ReentrantLock();
        datas_ = new LinkedList<byte[]>();
        datas_.clear();
    }

    public byte[] zcs_acquire() throws InterruptedException {
        super.acquire();

        lock_.lock();
        byte []data = datas_.poll();
//        Log.e("zcs", String.valueOf(zdata));
        lock_.unlock();

        return data;
    }

    public void zcs_release(byte []data) {
        lock_.lock();
        datas_.add(data);
        lock_.unlock();

        super.release();
    }

    public void zcs_clear() {
        lock_.lock();
        datas_.clear();
        while(this.tryAcquire());
        lock_.unlock();
    }

    private Lock lock_;
    private Queue<byte[]> datas_;
}