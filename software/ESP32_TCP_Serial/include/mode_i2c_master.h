#pragma once

#include <Wire.h>

#include "mode_base.h"

#define N_SLAVE_ADDR    8
#define I2C_HEADER_LEN    4
#define I2C_DATA_LEN_MAX  (256-4)

class ModeI2CMaster : public ModeBase {
private:
    TwoWire *wire_;
    uint8_t n_slave_;
    uint8_t slave_id_;
    uint8_t slave_addr_[N_SLAVE_ADDR];

    uint8_t key_flag_; //todo use this
    uint8_t i2c_buf_[I2C_HEADER_LEN + I2C_DATA_LEN_MAX];
    uint32_t i2c_buf_len_;

public:
    ModeI2CMaster();

    void begin(TwoWire *wire, Oled096 *oled);

    void start();
    uint8_t exec(uint8_t key);
    void pack_from_tcp(uint8_t pack_id, const uint8_t *pack, uint8_t pack_len);
    uint8_t pack_to_tcp(uint8_t *pack);

    uint8_t scan();
};