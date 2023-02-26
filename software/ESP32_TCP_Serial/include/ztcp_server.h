#pragma once

#include <WiFi.h>

#include "at24c02.h"
#include "mode_base.h"

#define T_LEN_MAX       128

class ZTcpServer {
private:
    At24c02 *at24c02_;
    char ssid_[24];
    char password_[24];
    uint16_t server_port_;
    uint8_t state_;

    WiFiServer server_;
    WiFiClient client_;

    ModeBase *obj_uart_;
    ModeBase *obj_i2c_;

    // uint8_t uart_buf_[U_HEADER_LEN + U_DATA_LEN_MAX];
    uint8_t tcp_buf_[T_LEN_MAX];
    uint8_t tcp_buf_state_;
    uint8_t tcp_buf_off_;
    uint8_t tcp_buf_len_;
    uint8_t pack_id_;

    int read_ssid_and_password(At24c02 *at24c02);

public:
    ZTcpServer();
    ~ZTcpServer() {}

    void begin(At24c02 *at24c02);
    void run();
    void connect_ap();
    uint8_t state() { return state_; }
    void check_client();
    void read_tcp_pack();
    // void uart2tcp(HardwareSerial *serial, uint8_t serial_id);
    // void tcp2uart();
    // WiFiClient *accept_client();

    void set_uart_callback(ModeBase *obj_uart);
    void set_i2c_callback(ModeBase *obj_i2c);
};