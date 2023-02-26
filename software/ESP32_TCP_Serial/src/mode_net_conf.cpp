#include "mode_net_conf.h"

ModeNetConf::ModeNetConf() {
    // memset(ssid_, 0, sizeof(ssid_));
    // memset(password_, 0, sizeof(password_));
}

void ModeNetConf::begin(ZTcpServer *ztcp_server, Oled096 *oled, At24c02 *at24c02) {
    ztcp_server_ = ztcp_server;
    oled_ = oled;
    at24c02_ = at24c02;
    zble_.begin();
}

void ModeNetConf::write_ssid_and_password(const char *ssid, const char *password) {
    if (strlen(ssid) < 1 || strlen(ssid) > 16) { //oled 12864 一行最多16个字符，这里约束一下
        return;
    }
    at24c02_->write_byte(0x00, (uint8_t)strlen(ssid));
    at24c02_->write(0x01, (uint8_t*)ssid, strlen(ssid));

    if (strlen(password) < 1 || strlen(password) > 16) { //oled 12864 一行最多16个字符，这里约束一下
        return;
    }
    at24c02_->write_byte(0x80, (uint8_t)strlen(password));
    at24c02_->write(0x81, (uint8_t*)password, strlen(password));
}

void ModeNetConf::start() {
    oled_->clear();
    oled_->zx_print(0, 0, "    Net Conf    ");
    oled_->zx_print(0, 6, "MODE");
    oled_->zx_print(48, 6, "    ");
    oled_->zx_print(96, 6, "    ");
    oled_->show();
    oled_->show(1);

    zble_.start_ble();
}

uint8_t ModeNetConf::exec(uint8_t key) {
    if (key == 0x01) {
        zble_.stop_ble();
        return 1;
    } else if (key == 0x02) {
    } else if (key == 0x04) {
    } else { //刷新oled
        if (zble_.state() == 0x02) {
            printf("writing [%s][%s]\r\n", zble_.ssid().c_str(), zble_.password().c_str());
            this->write_ssid_and_password(zble_.ssid().c_str(), zble_.password().c_str());
            for (int s = 3; s >= 1; s --) {
                char buf[32];
                sprintf(buf, "reboot in %d s", s);
                oled_->print(0, 2, buf);
                oled_->show();
                oled_->show(1);
                delay(1000);
            }
            ESP.restart();
        }
        if (ztcp_server_->state() == 2) {
            oled_->print(0, 2, WiFi.localIP().toString().c_str(), true);
            oled_->show();
            oled_->show(1);
        } else if (ztcp_server_->state() == 1) {
            oled_->print(0, 2, "connecting");
            oled_->show();
            oled_->show(1);
        } else if (ztcp_server_->state() == 0) {
            oled_->print(0, 2, "not connected");
            oled_->show();
            oled_->show(1);
        }
    }

    return 0;
}
