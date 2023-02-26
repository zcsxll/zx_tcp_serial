#pragma once

#include <Arduino.h>
#include <Wire.h>

#define AT24C02

#ifdef AT24C02
#define PAGE_SIZE   (8)
#endif
#if defined (AT24C04) || defined (AT24C08) || defined (AT24C16)
#define PAGE_SIZE   (16)
#endif
#if defined(AT24C32) || defined(AT24C64)
#define PAGE_SIZE   (32)
#endif

class At24c02 {
private:
    uint8_t addr_;
    TwoWire *wire_;

public:
    At24c02(uint8_t addr, uint8_t wire_id=0);
    ~At24c02() {}

    void begin(int sda=-1, int scl=-1, uint32_t frequency=0U, bool init_wire=false);
    void wait();
    uint8_t read_byte(uint8_t addr);
    int read(uint8_t addr, uint8_t *buf, uint8_t n_total);

    void write_byte(uint8_t addr, uint8_t data);
    int write(int addr, const uint8_t *buf, int n_total);
};