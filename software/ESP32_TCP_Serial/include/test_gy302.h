#pragma once

#include <Arduino.h>
#include <Wire.h>

int32_t gy302_read(TwoWire &wire) {
    wire.beginTransmission(0x23);
    wire.write((uint8_t)0x01);
    wire.endTransmission();

    wire.beginTransmission(0x23);
    // wire.write((uint8_t)0x01);
    wire.write((uint8_t)0x10);
    wire.endTransmission();

    if (wire.requestFrom(0x23, 2) != 2) {
        return -1;
    }
    uint8_t c1 = wire.read();
    uint8_t c2 = wire.read();
    return (int32_t)((c1 << 8) + c2);
}

// class GY271 {
// private:
//     TwoWire *wire_;

// public:
//     GY271(TwoWire *wire) {
//         wire_ = wire;
//     }

//     void begin() {
//         wire_->beginTransmission(0x0d);
//         wire_->write(0x02);
//         wire_->write(0x00);
//         uint8_t ret = wire_->endTransmission();
//         printf("%d\r\n", ret);
//     }

//     void read() {
//         wire_->beginTransmission(0x0d);
//         wire_->write(0x03);
//         uint8_t ret = wire_->endTransmission();
//         printf("write 0x03: %d\r\n", ret);

//         uint8_t buf[6];
//         ret = wire_->requestFrom(0x0d+1, 6);
//         printf("requestFrom 0xd1: %d\r\n", ret);
//         for(uint8_t i = 0; i < 6; i ++) {
//             buf[i] = wire_->read();
//         }
//         for(uint8_t i = 0; i < 6; i ++) {
//             printf("%02x ", buf[i]);
//         }
//         printf("\r\n");
//     }
// };