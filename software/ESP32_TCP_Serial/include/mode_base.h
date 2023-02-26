#pragma once

#include "oled096.h"

class ModeBase {
protected:
    Oled096 *oled_;

public:
    virtual void start() = 0;
    virtual uint8_t exec(uint8_t key) = 0;

    virtual void pack_from_tcp(uint8_t pack_id, const uint8_t *pack, uint8_t pack_len) = 0;
    virtual uint8_t pack_to_tcp(uint8_t *pack) = 0;
};