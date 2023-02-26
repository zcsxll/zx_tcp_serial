#include "mode_i2c_master.h"

ModeI2CMaster::ModeI2CMaster() {
    n_slave_ = 0;
    i2c_buf_[0] = 'Z';
    i2c_buf_[1] = 'X';
    i2c_buf_len_ = 0;
}

void ModeI2CMaster::begin(TwoWire *wire, Oled096 *oled) {
    wire_ = wire;
    oled_ = oled;
}

void ModeI2CMaster::start() {
    oled_->clear();
    oled_->zx_print(0, 0, "   I2C Master   ");
    oled_->zx_print(0, 6, "MODE");
    oled_->zx_print(48, 6, "SCAN");
    oled_->zx_print(96, 6, "    ");
    oled_->show();
    oled_->show(1);
}

uint8_t ModeI2CMaster::exec(uint8_t key) {
    if (key == 0x01) {
        return 1;
    } else if (key == 0x02) {
        this->scan();
    } else if (key == 0x04) {
        if (n_slave_ > 0) {
            for (int i = 0; i < n_slave_; i ++) {
                printf("addr %d: 0x%02x\r\n", i, slave_addr_[i]);
            }

            char buf[24];
            sprintf(buf, "device %d: 0x%02x", slave_id_, slave_addr_[slave_id_]);
            oled_->print(0, 2, buf, true);
            oled_->show(2, 3);
            slave_id_ = (slave_id_ + 1) % n_slave_;
            delay(200);
        }
    }
    return 0;
}

void ModeI2CMaster::pack_from_tcp(uint8_t pack_id, const uint8_t *pack, uint8_t pack_len) {
    for (int i = 0; i < pack_len; i ++) printf("%02x ", pack[i]); printf(" [0x%02x]\r\n", pack_id);
    if (pack_id == 0x11) { //write data to i2c
        uint8_t i2c_addr = pack[0];
        uint32_t data_len = *(uint32_t*)(pack+1);
        printf("write data(%d bytes) to i2c(0x%02x)\r\n", data_len, i2c_addr);
        wire_->beginTransmission(i2c_addr);
        for (int i = 0; i < data_len; i ++) {
            wire_->write(pack[5+i]);
        }
        uint8_t ret = wire_->endTransmission();
        i2c_buf_[2] = pack_id;
        i2c_buf_[3] = 1; //data len
        i2c_buf_[4] = ret;
        i2c_buf_len_ = 5;
    } else if (pack_id == 0x12) {
        uint8_t i2c_addr = pack[0];
        uint8_t mem_addr = pack[1];
        int32_t data_len = *(uint32_t*)(pack+2);
        printf("write data(%d bytes) to i2c(0x%02x), mem_addr=0x%02x\r\n", data_len, i2c_addr, mem_addr);
        wire_->beginTransmission(i2c_addr);
        wire_->write(mem_addr);
        for (int i = 0; i < data_len; i ++) {
            wire_->write(pack[6+i]);
        }
        uint8_t ret = wire_->endTransmission();
        i2c_buf_[2] = pack_id;
        i2c_buf_[3] = 1; //data len
        i2c_buf_[4] = ret;
        i2c_buf_len_ = 5;
    } else if (pack_id == 0x13) { //read data from i2c
        uint8_t i2c_addr = pack[0];
        uint8_t mem_addr = pack[1];
        int32_t read_len = *(uint32_t*)(pack+2);
        printf("read from 0x%02x, mem_addr=0x%02x, read_len=%d\r\n", i2c_addr, mem_addr, read_len);
        wire_->beginTransmission(i2c_addr);
        wire_->write(mem_addr);
        wire_->endTransmission();
        i2c_buf_[2] = pack_id;
        if (wire_->requestFrom(i2c_addr, (uint8_t)(read_len&0xff)) != read_len) {
            printf("read failed\r\n");
            i2c_buf_[3] = 0; //data len
            i2c_buf_len_ = 4;
            return;
        }

        i2c_buf_[3] = read_len; //data len
        i2c_buf_len_ = 4 + read_len;
        for (int i = 0; i < read_len; i ++) {
            i2c_buf_[4+i] = wire_->read();
        }

        printf("ret: %d\r\n", (int32_t)((i2c_buf_[4] << 8) + i2c_buf_[5]));
    }
}

uint8_t ModeI2CMaster::pack_to_tcp(uint8_t *pack) {
    uint8_t ret = i2c_buf_len_;
    if (i2c_buf_len_ > 0) { //设置串口之后有反馈
        memcpy(pack, i2c_buf_, i2c_buf_len_);
        i2c_buf_len_ = 0;
    }
    return ret;
}

uint8_t ModeI2CMaster::scan() {
    n_slave_ = 0;
    slave_id_ = 0;
    oled_->clear(2, 5);
    char buf[32];
    for (uint8_t addr = 1; addr <= 127; addr ++) {
        sprintf(buf, "scanning 0x%02x", addr);
        oled_->print(0, 2, buf, true);
        oled_->show(2, 3);
        wire_->beginTransmission(addr);
        uint8_t ret = wire_->endTransmission();
        if (ret == 0) {
            printf("found I2C device at addr 0x%02x\r\n", addr);
            slave_addr_[n_slave_++] = addr;
        }
    }
    oled_->print(0, 2, "scan finish", true);
    if (n_slave_ == 0) {
        oled_->print(0, 4, "no device found");
        oled_->zx_print(96, 6, "    ");
    } else {
        sprintf(buf, "%d device%sfound", n_slave_, n_slave_ == 1 ? " " : "s ");
        oled_->print(0, 4, buf);
        oled_->zx_print(96, 6, "ADDR");
    }
    oled_->show(2, 7);
    return n_slave_;
}
