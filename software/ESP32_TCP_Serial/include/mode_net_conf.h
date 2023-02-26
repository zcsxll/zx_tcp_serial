#pragma once

#include "mode_base.h"
#include "zble.h"
#include "ztcp_server.h"
#include "at24c02.h"

class ModeNetConf : public ModeBase {
private:
    ZTcpServer *ztcp_server_;
    ZBle zble_;
    At24c02 *at24c02_;

public:
    ModeNetConf();

    void begin(ZTcpServer *ztcp_server, Oled096 *oled, At24c02 *at24c02);
    void write_ssid_and_password(const char *ssid, const char *password);
    
    void start();
    uint8_t exec(uint8_t key);
    void pack_from_tcp(uint8_t pack_id, const uint8_t *pack, uint8_t pack_len) {}
    uint8_t pack_to_tcp(uint8_t *pack) { return 0; }
};