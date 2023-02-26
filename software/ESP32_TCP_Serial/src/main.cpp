#include <Arduino.h>

#include "oled096.h"
#include "at24c02.h"
#include "ztcp_server.h"
#include "mode_i2c_master.h"
#include "mode_uart.h"
#include "mode_net_conf.h"
#include "mode_for_fun.h"
#include "test_gy302.h"

Oled096 oled(0x3c, 1); //板载器件使用I2C1
At24c02 at24c02(0x50, 1); //板载器件使用I2C1
ZTcpServer ztcp_server;
ModeI2CMaster mode_i2c_master;
ModeUart mode_uart;
ModeNetConf mode_net_conf;
ModeForFun mode_for_fun;

ModeBase *modes[4] = {
    &mode_i2c_master, &mode_uart, &mode_net_conf, &mode_for_fun
};
uint8_t mode_id = 0;

int read_key() {
    int ret = 0;
    ret |= (digitalRead(15) == 0 ? 1 : 0);
    ret |= (digitalRead(2) == 0 ? 2 : 0);
    ret |= (digitalRead(4) == 0 ? 4 : 0);
    return ret;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial2.begin(115200);
    while (!Serial2);

    Wire.begin(); //I2C 0，默认IO 21 22
    oled.begin(14, 13, 400000);
    at24c02.begin();
    delay(100); //without this, may failed to read at24c02
    ztcp_server.begin(&at24c02);
    mode_i2c_master.begin(&Wire, &oled);
    mode_uart.begin(&Serial2, &oled);
    mode_net_conf.begin(&ztcp_server, &oled, &at24c02);
    mode_for_fun.begin(&oled);

    ztcp_server.set_uart_callback(&mode_uart);
    ztcp_server.set_i2c_callback(&mode_i2c_master);

    pinMode(15, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);

    // oled.clear();
    // oled.print(0, 0, "Hello");
    // oled.show();
    // oled.show(1);
    modes[mode_id]->start();

    // for (int i = 0; i < 20; i ++) {
    //     uint16_t light = gy302_read(Wire);
    //     printf("%d\r\n", light);
    //     delay(200);
    // }
}

void loop() {
    // scan_i2c_device(&Wire);
    int key = read_key();
    if (modes[mode_id]->exec(key) == 1) {
        mode_id = (mode_id + 1) % (sizeof(modes) / sizeof(modes[0]));
        printf("mode %d\r\n", mode_id);
        modes[mode_id]->start();
    }
    // printf("%d\r\n", ret);
    delay(100);
}