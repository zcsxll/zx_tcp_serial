#pragma once

#include <HardwareSerial.h>

#include "mode_base.h"

#define U_HEADER_LEN    4
#define U_DATA_LEN_MAX  (128-4)

class ModeUart : public ModeBase {
private:
    HardwareSerial *uart_;
    char uart_conf_[6];

    uint8_t uart_buf_[U_HEADER_LEN + U_DATA_LEN_MAX];
    uint8_t uart_buf_len_;
    uint32_t n_bytes_rx_;
    uint32_t n_bytes_tx_;
    uint8_t key_flag_; //todo use this

public:
    ModeUart();

    void begin(HardwareSerial *uart, Oled096 *oled);

    void start();
    uint8_t exec(uint8_t key);
    void pack_from_tcp(uint8_t pack_id, const uint8_t *pack, uint8_t pack_len);
    uint8_t pack_to_tcp(uint8_t *pack);

    // void begin();
    // void uart2tcp();
};