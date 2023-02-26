#pragma once

#include <Arduino.h>
#include <Wire.h>

class Oled096 {
private:
    uint8_t addr_;
    TwoWire *wire_;
    uint8_t mem_[8*128]; //128 * 64 / 8 = 128 * 8
    char buf_[64];
    bool need_paint_;

    int write_cmd(uint8_t cmd);
    int write_data(uint8_t cmd);

public:
    Oled096(uint8_t addr, uint8_t wire_id=0);
    ~Oled096() {}

    int begin(int sda=-1, int scl=-1, uint32_t frequency=0U);
    void show(uint8_t flag=0);
    void show(uint8_t ys, uint8_t ye);
    void clear();
    void clear(uint8_t ys, uint8_t ye);
    void print(uint8_t x, uint8_t y, const char* s, bool clear_to_end=false);
    void print(uint8_t x, uint8_t y, uint16_t u16, bool clear_to_end=false);
    void zx_print(uint8_t x, uint8_t y, const char* s);
    void paint(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *data);
};