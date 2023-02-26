package com.zx.netelf;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.util.Arrays;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private BluetoothAdapter bluetooth_adapter_;
    private BroadcastReceiver receiver_;
    private ZBluetooth gatt_callback_;
    private ThreadRead thread_read_;

    private Button btn_connect_;
    private Button btn_set_;
    private EditText et_ssid_;
    private EditText et_password_;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH}, 0);
        }

        if (ContextCompat.checkSelfPermission(this,
                "android.permission.BLUETOOTH_CONNECT") != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{"android.permission.BLUETOOTH_CONNECT"}, 0);
        }

        if (ContextCompat.checkSelfPermission(this,
                "android.permission.BLUETOOTH_SCAN") != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{"android.permission.BLUETOOTH_SCAN"}, 0);
        }

        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_ADMIN}, 0);
        }

        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, 0);
        }

        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 0);
        }

        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
        }

        btn_connect_ = findViewById(R.id.btn_connect);
        btn_set_ = findViewById(R.id.btn_set);
        et_ssid_ = findViewById(R.id.et_ssid);
        et_password_ = findViewById(R.id.et_password);

        et_ssid_.setText("Redmi_EBB4");
        et_password_.setText("zcsxll1314");

        btn_connect_.setOnClickListener(this);
        btn_set_.setOnClickListener(this);

        Handler handler = new Handler() {
            @Override
            public void handleMessage(@NonNull Message msg) {
                switch (msg.what) {
                    case 1:
                        Log.e("zzz", "蓝牙已连接");
                        btn_connect_.setText("已连接");
                        break;
                    case 2:
                        Log.e("zzz", "蓝牙已断开");
                        btn_connect_.setText("连接蓝牙");
                        break;
                }
            }
        };

        gatt_callback_ = new ZBluetooth(handler);
        receiver_ = new BroadcastReceiver() {
            @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                /*
                  Discovery has found a device. Get the BluetoothDevice object and its info from the Intent.
                  这个BluetoothDevice是远程设备的Device
                */
                    try {
                        BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                        String device_name = device.getName();
                        String device_hardware_address = device.getAddress(); // MAC address
                        if (device_name != null && device_name.indexOf("ESP32") >= 0) {
                            Log.e("zzz", device_name + " " + device_hardware_address);
                            bluetooth_adapter_.cancelDiscovery();
                            Thread.sleep(100);
//                        thread_ = new ThreadBle(device);
//                        thread_.start();
                            device.connectGatt(context, false, gatt_callback_);
//                        socket_ = (BluetoothSocket) device.getClass().getDeclaredMethod("createRfcommSocket", new Class[]{int.class}).invoke(device, 1);
//                        Thread.sleep(500);
//                        socket_.connect();
//                        istream_ = socket_.getInputStream();
//                        ostream_ = socket_.getOutputStream();
//                        Log.e("zzz", "蓝牙已连接");
////                        sem_.zcs_release(device);
//                        tv_state_.setText("蓝牙已连接，请返回主页面");
                        } else {
                            Log.i("zzz", "[" + device_name + "]");
                        }
                    } catch (Exception e) { //有时候device_name是null，Log.i不能打印null，也不能调用null的equsls函数
                        Log.e("zzz", e.getMessage());
                    }
                }
            }
        };

        bluetooth_adapter_ = BluetoothAdapter.getDefaultAdapter();
        IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        this.registerReceiver(receiver_, filter);

        thread_read_ = new ThreadRead(handler, this, gatt_callback_);
        thread_read_.start();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_connect:
                if (btn_connect_.getText().equals("连接蓝牙")) {
                    if (!bluetooth_adapter_.isDiscovering()) {
                        Log.e("zzz", "" + bluetooth_adapter_);
                        boolean ret = bluetooth_adapter_.startDiscovery();
                        if (!ret) {
                            Toast.makeText(this, "无法开启蓝牙扫描", Toast.LENGTH_SHORT).show();
                            return;
                        }
                    }
                    btn_connect_.setText("搜索中");
                } else {
                    Log.e("zzz", "搜索中...");
                }
                break;
            case R.id.btn_set:
                String ssid_s = et_ssid_.getText().toString();
                String password_s = et_password_.getText().toString();
                Log.e("zzz", "[" + ssid_s + "] [" + password_s + "]");
                byte []ssid = ssid_s.getBytes();
                byte []password = password_s.getBytes();
                byte []pack = new byte[ssid.length+password.length + 3];
                System.arraycopy(ssid, 0, pack, 1, ssid.length);
                System.arraycopy(password, 0, pack, 1 + ssid.length + 1, password.length);
                pack[0] = '[';
                pack[ssid.length+1] = ' ';
                pack[pack.length-1] = ']';
                Log.e("zzz", new String(pack) + " " + pack.length);
                for (int i = 0; i < pack.length; i += 10) {
                    gatt_callback_.write(Arrays.copyOfRange(pack, i, Math.min(i + 10, pack.length)));
                    //Log.e("zzz", new String(Arrays.copyOfRange(pack, i, Math.min(i + 10, pack.length))) + " " + i);
                    try {
                        Thread.sleep(100);
                    } catch (Exception e) {
                        Log.e("zzz", e.getMessage());
                    }
                }
                break;
        }
    }
}