package com.zx.netelf;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import androidx.annotation.RequiresApi;

import java.util.List;
import java.util.UUID;

@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
public class ZBluetooth extends BluetoothGattCallback {
    private BluetoothGatt gatt_communicate_;
    private BluetoothGattCharacteristic write_haracteristic_;
    private BluetoothGattCharacteristic notify_characteristic_;

    private MySemaphore zsem_ = new MySemaphore(0);
    private Handler handler_;

    public ZBluetooth(Handler handler) {
        super();
        handler_ = handler;
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState){
        if (newState == BluetoothProfile.STATE_CONNECTED) {
            gatt.discoverServices();
            Log.d("zzz","蓝牙连接成功");
            Message m = new Message();
            m.what = 1;
            handler_.sendMessage(m);
        } else {
            Log.d("zzz","蓝牙连接断开");
            Message m = new Message();
            m.what = 2;
            handler_.sendMessage(m);
        }
    };

    //写入成功回调函数
    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status){
        Log.e("zzz", "蓝牙写入成功");
    };

    //接受数据回调
//        @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
//        @Override
//        public void onCharacteristicRead(BluetoothGatt gatt,
//                                         BluetoothGattCharacteristic characteristic,
//                                         int status) {
//            if (status == BluetoothGatt.GATT_SUCCESS) {
//                Log.e("zzz", "read: " + characteristic.getStringValue(0));
//            }
////            gatt_communicate_.readCharacteristic(notify_characteristic_);
////            gatt_communicate_.setCharacteristicNotification(characteristic, true);
////            gatt_communicate_.readCharacteristic(characteristic);
//        }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
        byte []data = characteristic.getValue();
//        String hex = "";
//        for (int i = 0; i < data.length; i ++) {
//            String s = Integer.toHexString(data[i] & 0xFF);
//            hex += (s.length() == 1) ? "0" + s : s;
//        }
        zsem_.zcs_release(data);
    }

    //UUID搜索成功回调
    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {
        if (status == BluetoothGatt.GATT_SUCCESS) {
            List<BluetoothGattService> supportedGattServices = gatt.getServices();
            for (int i = 0; i < supportedGattServices.size(); i++) {
                Log.i("zzz", "bluetooth_gatt_service UUID=:" + supportedGattServices.get(i).getUuid());
                BluetoothGattService bluetooth_gatt_service = gatt.getService(supportedGattServices.get(i).getUuid());
//                Log.e("zzz", "    " + bluetooth_gatt_service.toString());
                List<BluetoothGattCharacteristic> listGattCharacteristic = supportedGattServices.get(i).getCharacteristics();
                for (int j = 0; j < listGattCharacteristic.size(); j++) {
                    UUID uuid = listGattCharacteristic.get(j).getUuid();
                    Log.i("zzz", "    BluetoothGattCharacteristic UUID=:" + uuid);
                    if (uuid.toString().indexOf("6e400003") >= 0) {
                        gatt_communicate_ = gatt;
                        notify_characteristic_ = bluetooth_gatt_service.getCharacteristic(uuid);

                        for(BluetoothGattDescriptor dp: notify_characteristic_.getDescriptors()) { //傻逼华为手机，没有这些代码无法触发onCharacteristicChanged
                            dp.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            gatt.writeDescriptor(dp);
                        }

                        gatt.setCharacteristicNotification(notify_characteristic_, true);
//                        gatt.readCharacteristic(read_characteristic_); //read类型的characteristic只会触发一次onCharacteristicRead
//                        Log.d("zzz", "监听uuid连接成功");
//                        Message m = new Message();
//                        m.what = 1;
//                        handler_.sendMessage(m);
                    } else if (uuid.toString().indexOf("6e400002") >= 0) {
                        write_haracteristic_ = bluetooth_gatt_service.getCharacteristic(uuid);
                    }
                }
            }
        } else {
            Log.e("zzz", "onservicesdiscovered()收到: " + status);
        }
    }

    public byte[] read() {
        try {
            byte data[] = zsem_.zcs_acquire();
            return data;
        } catch (Exception e) {
            Log.e("zzz", e.getMessage());
        }
        return new byte[0];
    }

    public int write(byte []data) {
        try {
            write_haracteristic_.setValue(data);
            write_haracteristic_.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
            gatt_communicate_.writeCharacteristic(write_haracteristic_);
        } catch (Exception e) {
            Log.e("zzz", e.getMessage());
            return -1;
        }
        return data.length;
    }
}
