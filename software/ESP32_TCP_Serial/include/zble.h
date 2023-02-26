#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "oled096.h"
#include "at24c02.h"

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // 定义收发服务的UUID（唯一标识）
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // RX串口标识
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // TX串口标识

class ZBle: public BLEServerCallbacks, BLECharacteristicCallbacks {
private:
    BLEServer *server_;
    BLECharacteristic *characteristic_;
    bool connected_;

    // WS2812B *leds_;
    Oled096 *oled_;
    // At24c02 *at24c02_;

    uint8_t state_;
    std::string ssid_password_;
    std::string ssid_;
    std::string password_;

public:
    ZBle();
    ~ZBle() {}

    void begin(/*Oled096 *oled, At24c02 *at24c02*/);
    void start_ble();
    void stop_ble();
    bool connected() { return connected_; }
    int send(uint8_t* data, size_t len);
    // void write_ssid_and_password(const char *ssid, const char *password);
    uint8_t state() { return state_; }
    std::string ssid() { return ssid_; }
    std::string password() { return password_; }

    void onConnect(BLEServer* server) { //from BLEServerCallbacks
        connected_ = true;
        printf("ble client connected!\r\n");
    };

    void onDisconnect(BLEServer* server) { //from BLEServerCallbacks
        connected_ = false;
        // server->getAdvertising()->start(); //开启通知
        printf("ble disconnected, don't start here, pls enter net conf mode again\r\n");
    }

    void onWrite(BLECharacteristic *pCharacteristic) { //from BLECharacteristicCallbacks
        std::string data = pCharacteristic->getValue();
        if (data.length() > 0) { //goon here, 可以传输不可显字符
            printf("recv %d bytes\r\n", data.length());
            for (int i = 0; i < data.length(); i++) {
                if (state_ == 0) {
                    if (data[i] == '[') {
                        ssid_password_ = "";
                        state_ = 1;
                    }
                } else if (state_ == 1) {
                    if (data[i] == ']') {
                        state_ = 0;
                        if (ssid_password_.length() >= 6) {
                            uint8_t pos = ssid_password_.find(" ");
                            if (pos > 0 && pos < ssid_password_.length() - 1) {
                                ssid_ = ssid_password_.substr(0, pos);
                                password_ = ssid_password_.substr(pos+1, ssid_password_.length()-pos-1);
                                printf("set (%s)(%s)\r\n", ssid_.c_str(), password_.c_str());
                                state_ = 2;
                            }
                        }
                    } else {
                        ssid_password_ += data[i];
                        if (ssid_password_.length() >= 40) {
                            state_ = 0;
                        }
                    }
                }
            }
        }
    }
};