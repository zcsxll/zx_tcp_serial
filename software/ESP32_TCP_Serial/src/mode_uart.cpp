#include "mode_uart.h"

// void task_tcp_uart(void *args) {
//     ModeUart *mode_uart = (ModeUart*)args;
//     // ztc_->connect_ap();
//     while (true) {
//         // ztc_->check_client();
//         // ztc_->uart2tcp(&Serial2, 1);
//         // ztc_->tcp2uart();
//         mode_uart->uart2tcp();
//         delay(1000);
//     }
// }

ModeUart::ModeUart() {
    sprintf(uart_conf_, "8 N 1");

    uart_buf_[0] = 'Z';
    uart_buf_[1] = 'X';
    uart_buf_len_ = 0;
    n_bytes_rx_ = 0;
    n_bytes_tx_ = 0;

    key_flag_ = 0;
}

void ModeUart::begin(HardwareSerial *uart, Oled096 *oled) {
    uart_ = uart;
    oled_ = oled;
    
    // xTaskCreate(task_tcp_uart, "task_tcp_uart", 10000, (void*)this, 0, NULL);
}

void ModeUart::start() {
    oled_->clear();
    oled_->zx_print(0, 0, "      Uart      ");
    oled_->zx_print(0, 6, "MODE");
    oled_->zx_print(48, 6, "ZERO");
    oled_->zx_print(96, 6, "SEND");
    char buf[24];
    sprintf(buf, "%d %s", uart_->baudRate(), uart_conf_);
    oled_->print(0, 2, buf, true);
    sprintf(buf, "R:%d", n_bytes_rx_);
    oled_->print(0, 4, buf);
    sprintf(buf, "T:%d", n_bytes_tx_);
    oled_->print(64, 4, buf);
    oled_->show();
    oled_->show(1);

    n_bytes_rx_ = 0;
    n_bytes_tx_ = 0;
}

uint8_t ModeUart::exec(uint8_t key) {
    if (key == 0) {
        key_flag_ = 0;
    }
    
    if (key == 0x01) {
        return 1;
    } else if (key == 0x02) {
        n_bytes_rx_ = 0;
        n_bytes_tx_ = 0;
    } else if (key == 0x04 && key_flag_ == 0) {
        key_flag_ = 1;
        uart_->printf("hello\r\n");
        n_bytes_tx_ += 7;
    }
    
    char buf[24];
    sprintf(buf, "%d %s", uart_->baudRate(), uart_conf_);
    oled_->print(0, 2, buf, true);
    sprintf(buf, "R:%d", n_bytes_rx_);
    oled_->print(0, 4, buf, true);
    sprintf(buf, "T:%d", n_bytes_tx_);
    oled_->print(64, 4, buf);
    oled_->show(2, 5);

    return 0;
}

void ModeUart::pack_from_tcp(uint8_t pack_id, const uint8_t *pack, uint8_t pack_len) {
    // for (int i = 0; i < pack_len; i ++) printf("%02x ", pack[i]); printf(" [%d]\r\n", pack_id);
    if (pack_id == 0x01) { //set uart config
        if (pack_len == 8) {
            uint32_t baund_rate = *(uint32_t*)(pack+0);
            uint32_t uart_conf_id = *(uint32_t*)(pack+4);
            printf("set uart config: [%d %x]\r\n", baund_rate, uart_conf_id);
            uart_->end();
            uart_->begin(baund_rate, uart_conf_id);
            while(!uart_->availableForWrite());

            uart_buf_[2] = pack_id;
            uart_buf_[3] = 2; //data len
            uart_buf_[4] = 'O';
            uart_buf_[5] = 'K';
            uart_buf_len_ = 6;
        }
    } else if (pack_id == 0x02) { //send data to uart
        uart_->write(pack, pack_len);
        n_bytes_tx_ += pack_len;
    }
}

uint8_t ModeUart::pack_to_tcp(uint8_t *pack) {
    uint8_t ret = uart_buf_len_;
    if (uart_buf_len_ > 0) { //设置串口之后有反馈
        memcpy(pack, uart_buf_, uart_buf_len_);
        uart_buf_len_ = 0;
    } else { //检查串口读取的数据
        if (uart_->available()) {
            size_t n_read = uart_->read(uart_buf_+U_HEADER_LEN, U_DATA_LEN_MAX);
            uart_buf_[n_read+U_HEADER_LEN] = '\0'; //printf("read %d bytes [%s]\r\n", n_read, uart_buf_+U_HEADER_LEN);
            uart_buf_[2] = 0x02; //uart data
            uart_buf_[3] = (uint8_t)n_read;
            ret = n_read + U_HEADER_LEN;
            memcpy(pack, uart_buf_, ret);
            n_bytes_rx_ += n_read;
        }
    }
    return ret;
}
