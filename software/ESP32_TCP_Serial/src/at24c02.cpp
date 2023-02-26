#include "at24c02.h"

At24c02::At24c02(uint8_t addr, uint8_t wire_id) {
    addr_ = addr;
    if (wire_id == 0) {
        wire_ = &Wire;
    } else if (wire_id == 1) {
        wire_ = &Wire1;
    } else {
        printf("wrong wire_id");
    }
}

void At24c02::begin(int sda, int scl, uint32_t frequency, bool init_wire) {
    if (init_wire) {
        wire_->begin(sda, scl, frequency);
    }
}

void At24c02::wait() {
    int resault = 0;
    int cnt = 0;
    do {
        wire_->beginTransmission(addr_);
        resault = wire_->endTransmission();
        delay(1);
    } while (resault != 0 && cnt++ < 1000);
}

uint8_t At24c02::read_byte(uint8_t addr) {
    this->wait();

    wire_->beginTransmission(addr_);
    wire_->write(addr);
    wire_->endTransmission();
    wire_->requestFrom(addr_, (uint8_t)1);
    uint8_t ret = wire_->read();
    return ret;
}

int At24c02::read(uint8_t addr, uint8_t *buf, uint8_t n_total) {
    int n_left = n_total;
    while(n_left) {
        uint8_t  n_read = n_left > 32 ? 32 : n_left;
        this->wait();
        wire_->beginTransmission(addr_);
        wire_->write(addr);  /*发送读地址*/
        wire_->endTransmission();
        wire_->requestFrom(addr_, n_read);
        for(uint8_t i = 0; i < n_read; i ++) {
            buf[i] = wire_->read();
        }
        n_left -= n_read;
        buf += n_read;
        addr += n_read;
    }
    return n_total;
}

void At24c02::write_byte(uint8_t addr, uint8_t data) {
    this->wait();
    
    wire_->beginTransmission(addr_);
    wire_->write(addr);
    wire_->write(data);
    /*uint8_t ret = */wire_->endTransmission();
}

int At24c02::write(int addr, const uint8_t *buf, int n_total) {
    int n_left = n_total;
    int n_send = PAGE_SIZE - (addr % PAGE_SIZE);
    n_send = (n_left > n_send) ? n_send : n_left;
    while(n_left) {
        this->wait();
        wire_->beginTransmission(addr_);
        wire_->write(addr);  /*发送写地址*/
        wire_->write(buf, n_send);  /*写数据*/
        wire_->endTransmission();
        n_left -= n_send;
        addr += n_send;
        buf += n_send;
        n_send = (n_left > PAGE_SIZE) ? PAGE_SIZE : n_left;
    }
    return n_total;
}