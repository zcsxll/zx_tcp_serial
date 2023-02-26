#include "oled096.h"
#include "font.h"

uint8_t CONF_DATA[] = {
    0xAE, 0x00, 0x10, 0x40, 0xB0, 0x81, 0xFF, 0xA0, 0xC0,
    0xA6, 0xA8, 0x3F, 0xD3, 0x00, 0xD5, 0x80, 0xD8, 0x05,
    0xD9, 0xF1, 0xDA, 0x12, 0xD8, 0x30, 0x8D, 0x14, 0xAF};

Oled096::Oled096(uint8_t addr, uint8_t wire_id) {
    addr_ = addr;
    if (wire_id == 0) {
        wire_ = &Wire;
    } else if (wire_id == 1) {
        wire_ = &Wire1;
    } else {
        printf("wrong wire_id");
    }

    need_paint_ = false;
}

int Oled096::write_cmd(uint8_t cmd) {
    wire_->beginTransmission(addr_);
    wire_->write(0x00);
    wire_->write(cmd);
    uint8_t ret = wire_->endTransmission();
    return ret;
}

int Oled096::write_data(uint8_t data) {
    wire_->beginTransmission(addr_);
    wire_->write(0x40);
    wire_->write(data);
    uint8_t ret = wire_->endTransmission();
    return ret;
}

int Oled096::begin(int sda, int scl, uint32_t frequency) {
    wire_->begin(sda, scl, frequency);
    for (uint8_t i = 0; i < 27; i++) {
		if (this->write_cmd(CONF_DATA[i]) != 0) {
			return -1;
		}
	}
    return 0;
}

void Oled096::show(uint8_t flag) {
    if (flag == 0) {
        need_paint_ = true;
        return;
    }
    if (!need_paint_) {
        return;
    }

    // printf("do paint\r\n");
    need_paint_ = false;
    // noInterrupts();
    uint16_t off = 0;
    for (uint8_t i = 0; i < 8; i++) {
        this->write_cmd(0xb0 + i);
        this->write_cmd(0x00);
        this->write_cmd(0x10);
        for (uint8_t n = 0; n < 128; n++) {
            this->write_data(mem_[off++]);
        }
    }
    // interrupts();
}

void Oled096::show(uint8_t ys, uint8_t ye) {
    uint16_t off = 128 * ys;
    for (int i = ys; i <= ye; i ++) {
        this->write_cmd(0xb0 + i);
        this->write_cmd(0x00);
        this->write_cmd(0x10);
        for (uint8_t n = 0; n < 128; n++) {
            this->write_data(mem_[off++]);
        }
    }
}

void Oled096::clear() {
    memset(mem_, 0, sizeof(mem_));
}

void Oled096::clear(uint8_t ys, uint8_t ye) {
    for (int i = ys; i <= ye; i ++) {
        this->write_cmd(0xb0 + i);
        this->write_cmd(0x00);
        this->write_cmd(0x10);
        for (uint8_t n = 0; n < 128; n++) {
            this->write_data(0x00);
        }
    }
}

void Oled096::print(uint8_t x, uint8_t y, const char* s, bool clear_to_end) {
    uint16_t off = y * 128 + x;
    for (const char *p = s; (*p) != '\0' && off < 128*(8-1); p ++) {
        char c = (*p) - ' ';
        memcpy(mem_+off, F8x16+c*16, 8);
        memcpy(mem_+off+128, F8x16+c*16+8, 8);
        off += 8;
    }
    if (clear_to_end) {
        for (; off % 128 != 0 && off < 128*(8-1); off ++) {
            mem_[off] = 0;
            mem_[off+128] = 0;
        }
    }
}

void Oled096::print(uint8_t x, uint8_t y, uint16_t u16, bool clear_to_end) {
    sprintf(buf_, "%d", u16);
    this->print(x, y, buf_, clear_to_end);
}

void Oled096::zx_print(uint8_t x, uint8_t y, const char* s) {
    uint16_t off = y * 128 + x;
    for (const char *p = s; (*p) != '\0' && off < 128*(8-1); p ++) {
        char c = (*p) - ' ';
        for (int i = 0; i < 8; i ++) {
            mem_[off+i] = F8x16[c*16+i] ^ 0xff;
            mem_[off+128+i] = F8x16[c*16+8+i] ^ 0xff;
        }
        off += 8;
    }
}

void Oled096::paint(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *data) {
    for (uint8_t yy = 0; yy < h; yy ++) {
        memcpy(mem_ + ((yy+y)*128+x), data + (yy*w), w);
    }
}