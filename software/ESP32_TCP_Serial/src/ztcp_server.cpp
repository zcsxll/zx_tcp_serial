#include "ztcp_server.h"

void task_tcp(void *args) {
    ZTcpServer *ztc_ = (ZTcpServer*)args;
    ztc_->run();
}

ZTcpServer::ZTcpServer() {
    memset(ssid_, 0, sizeof(ssid_));
    memset(password_, 0, sizeof(password_));
    server_port_ = 8899;
    state_ = 0;

    obj_uart_ = NULL;
    obj_i2c_ = NULL;

    tcp_buf_state_ = 0;
    tcp_buf_off_ = 0;
}

int ZTcpServer::read_ssid_and_password(At24c02 *at24c02) {
    uint8_t ssid_len = at24c02->read_byte(0x00);
    printf("read ssid len: %d\r\n", ssid_len);
    if (ssid_len < 1 || ssid_len > 16) { //oled 12864 一行最多16个字符，这里约束一下
        return 1;
    }
    at24c02->read(0x01, (uint8_t*)ssid_, ssid_len);
    ssid_[ssid_len] = '\0';

    uint8_t password_len = at24c02->read_byte(0x80);
    printf("read password len: %d\r\n", password_len);
    if (password_len < 1 || password_len > 16) { //oled 12864 一行最多16个字符，这里约束一下
        return 2;
    }
    at24c02->read(0x81, (uint8_t*)password_, password_len);
    password_[password_len] = '\0';

    printf("read ssid and password: [%s][%s]\r\n", ssid_, password_);
    return 0;
}

void ZTcpServer::begin(At24c02 *at24c02) {
    at24c02_ = at24c02;
    xTaskCreatePinnedToCore(task_tcp, "task_tcp", 10000, this, 0, NULL, 1);
    // xTaskCreate(task_tcp2uart, "task_tcp2uart", 10000, (void*)&ztcp_server, 0, NULL);
}

void ZTcpServer::run() {
    if (this->read_ssid_and_password(at24c02_) == 0) {
        state_ = 1;
        this->connect_ap();
        state_ = 2;
    }

    uint8_t pack[256];
    while (true) {
        delay(50);
        this->check_client();
        // if (!client_.connected()) {
        //     continue;
        // }
        this->read_tcp_pack();
        if (obj_uart_ != NULL) {
            uint8_t pack_len = obj_uart_->pack_to_tcp(pack); //悬空或者接入usb转串口但是不接在电脑上，复位后这里会收到一些数据，不过没有影像
            if (pack_len > 0) {
                printf("write pack(%d bytes) about uart to tcp\r\n", pack_len);
                client_.write(pack, pack_len);
            }
        }
        if (obj_i2c_ != NULL) {
            uint8_t pack_len = obj_i2c_->pack_to_tcp(pack);
            if (pack_len > 0) {
                printf("write pack(%d bytes) about i2c to tcp\r\n", pack_len);
                client_.write(pack, pack_len);
            }
        }
    }
}

void ZTcpServer::connect_ap() {
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true); //wifi蓝牙同时用，必须打开
    WiFi.begin(ssid_, password_);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(200);
    }
    Serial.printf("wifi connected\r\n");
    Serial.printf("IP:");
    Serial.print(WiFi.localIP());
    Serial.printf(" Port:%d\r\n", server_port_);
    server_.begin(server_port_);

    // oled_->print(0, 0, WiFi.localIP().toString().c_str(), true);
    // oled_->print(0, 2, server_port_, true);
    // oled_->show();
}

void ZTcpServer::check_client() {
    if (!client_.connected()) {
        // printf("checking client\r\n");
        client_ = server_.available();
        if (client_.connected()) {
            printf("client connected!\r\n");
            // printf("client connected!\r\n");
            // oled_->print(0, 4, "client connected", true);
            // oled_->show();
        }
    }
}

/*
pack format
0 1 2   3  4...
X Z id len pack

X: fixed
Z: fixed
id: pack id
len: pack len without X Z id and len
*/
void ZTcpServer::read_tcp_pack() {
    while (client_.available()) {
        int c = client_.read();
        if (tcp_buf_state_ == 0 && c == 'X') {
            tcp_buf_state_ = 1;
        } else if (tcp_buf_state_ == 1 && c == 'Z') {
            tcp_buf_state_ = 2;
        } else if (tcp_buf_state_ == 2) {
            tcp_buf_state_ = 3;
            pack_id_ = c;
        } else if (tcp_buf_state_ == 3) {
            tcp_buf_state_ = 4;
            tcp_buf_len_ = c;
            tcp_buf_off_ = 0;
        } else if (tcp_buf_state_ == 4) {
            tcp_buf_[tcp_buf_off_++] = c;
            if (tcp_buf_off_ >= tcp_buf_len_) {
                tcp_buf_state_ = 0;
                if (pack_id_ == 0x01 || pack_id_ == 0x02) { //data to serial
                    if (obj_uart_ != NULL) {
                        obj_uart_->pack_from_tcp(pack_id_, tcp_buf_, tcp_buf_len_);
                    }
                } else if (pack_id_ == 0x11 || pack_id_ == 0x12 || pack_id_ == 0x13) {
                    /* 0x11 write buf to i2c slave
                     * 0x12 write buf to i2c slave, with addr
                     * 0x13 read data from i2c, with addr
                     */
                    if (obj_i2c_ != NULL) {
                        obj_i2c_->pack_from_tcp(pack_id_, tcp_buf_, tcp_buf_len_);
                    }
                }
            }
        } else {
            tcp_buf_state_ = 0;
        }
    }
}

// void ZTcpServer::uart2tcp(HardwareSerial *serial, uint8_t serial_id) {
//     if (serial->available()) {
//         size_t n_read = serial->read(uart_buf_+U_HEADER_LEN, U_DATA_LEN_MAX);
//         // printf("[%d] read %d bytes %02x\r\n", serial_id, n_read, uart_buf_[4]);
//         uart_buf_[2] = serial_id;
//         uart_buf_[3] = (uint8_t)n_read;
//         if (client_.connected()) {
//             client_.write(uart_buf_, U_HEADER_LEN + n_read);
//         }
//     }
// }

// void ZTcpServer::tcp2uart() {
//     if (client_.available()) {
//         int c = client_.read();
//         if (tcp_buf_state_ == 0 && c == 'X') {
//             tcp_buf_state_ = 1;
//         } else if (tcp_buf_state_ == 1 && c == 'Z') {
//             tcp_buf_state_ = 2;
//         } else if (tcp_buf_state_ == 2) {
//             tcp_buf_state_ = 3;
//             pack_id_ = c;
//         } else if (tcp_buf_state_ == 3) {
//             tcp_buf_state_ = 4;
//             tcp_buf_len_ = c;
//             tcp_buf_off_ = 0;
//         } else if (tcp_buf_state_ == 4) {
//             tcp_buf_[tcp_buf_off_++] = c;
//             if (tcp_buf_off_ >= tcp_buf_len_) {
//                 tcp_buf_state_ = 0;
//                 if (pack_id_ == 0) { //data to serial
//                     Serial.write(tcp_buf_, tcp_buf_len_);
//                 } else if (pack_id_ == 1) { //data to serial2
//                     Serial2.write(tcp_buf_, tcp_buf_len_);
//                 } else if (pack_id_ == 0x11) { //set config
//                     if (tcp_buf_len_ == 9) {
//                         uint8_t serial_id = tcp_buf_[0];
//                         uint32_t baund_rate = *(uint32_t*)(tcp_buf_+1);
//                         uint32_t uart_conf_id = *(uint32_t*)(tcp_buf_+5);
//                         // printf("[%d %d %d]\r\n", serial_id, baund_rate, uart_conf_id);
//                         if (serial_id == 0) {
//                             Serial.end();
//                             Serial.begin(baund_rate, uart_conf_id);
//                             while(!Serial.availableForWrite());
//                         } else if (serial_id == 1) {
//                             Serial2.end();
//                             Serial2.begin(baund_rate, uart_conf_id);
//                             while(!Serial2.availableForWrite());
//                         }
//                         uart_buf_[2] = 0x11;
//                         uart_buf_[3] = 2; //data len
//                         uart_buf_[4] = 'O';
//                         uart_buf_[5] = 'K';
//                         if (client_.connected()) {
//                             client_.write(uart_buf_, U_HEADER_LEN + 2);
//                         }
//                     }
//                 }
//             }
//         } else {
//             tcp_buf_len_ = 0;
//         }
//         // printf("read %02x, state = %d\r\n", c, tcp_buf_state_);
//     }
// }

// WiFiClient *ZTcpServer::accept_client() {
//     client_ = server_.available();
//     if (client_) {
//         return &client_;
//     }
//     return NULL;
// }

void ZTcpServer::set_uart_callback(ModeBase *obj_uart) {
    obj_uart_ = obj_uart;
}

void ZTcpServer::set_i2c_callback(ModeBase *obj_i2c) {
    obj_i2c_ = obj_i2c;
}