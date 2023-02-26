#include "zble.h"

ZBle::ZBle() {
    server_ = NULL;
    characteristic_ = NULL;
    connected_ = false;

    // oled_ = NULL;
    state_ = 0;
    ssid_password_ = "";
}

void ZBle::begin(/*Oled096 *oled*/) {
    // oled_ = oled;
    // at24c02_ = at24c02;

    BLEDevice::init("ESP32 BLE"); //初始化蓝牙设备
    server_ = BLEDevice::createServer(); //为蓝牙设备创建服务器
    server_->setCallbacks(this);
    BLEService *service = server_->createService(SERVICE_UUID); //基于SERVICE_UUID来创建一个服务
    characteristic_ = service->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    characteristic_->addDescriptor(new BLE2902());
    BLECharacteristic *characteristic = service->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    characteristic->setCallbacks(this);
    service->start(); //开启服务
    // server_->getAdvertising()->start(); //开启通知
    // printf("waiting a client connection to notify...\r\n");
}

void ZBle::start_ble() {
    server_->getAdvertising()->start(); //开启通知
}

void ZBle::stop_ble() {
    server_->getAdvertising()->stop(); //关闭通知
}

int ZBle::send(uint8_t* data, size_t len) {
    characteristic_->setValue(data, len);
    characteristic_->notify();
    return 0;
}

// void ZBle::write_ssid_and_password(const char *ssid, const char *password) {
//     at24c02_->write_byte(0x00, (uint8_t)strlen(ssid));
//     if (strlen(ssid) < 1 || strlen(ssid) > 16) { //oled 12864 一行最多16个字符，这里约束一下
//         return;
//     }
//     at24c02_->write(0x01, (uint8_t*)ssid, strlen(ssid));

//     at24c02_->write_byte(0x80, (uint8_t)strlen(password));
//     if (strlen(password) < 1 || strlen(password) > 16) { //oled 12864 一行最多16个字符，这里约束一下
//         return;
//     }
//     at24c02_->write(0x81, (uint8_t*)password, strlen(password));
// }
