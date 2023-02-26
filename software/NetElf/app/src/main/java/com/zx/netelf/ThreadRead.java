package com.zx.netelf;

import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.util.Log;

import androidx.annotation.RequiresApi;

public class ThreadRead extends Thread {
    private Handler handler_;
    private Context context_;
    private ZBluetooth zble_;

    public ThreadRead(Handler handler, Context context, ZBluetooth zble) {
        handler_ = handler;
        context_ = context;
        zble_ = zble;
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    public void run() {
        while(true) {
            byte []data = zble_.read();
            String s = new String(data);
            Log.e("zzz", "read from ble: " + s);
        }
    }
}
