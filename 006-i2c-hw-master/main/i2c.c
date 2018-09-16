/*

Copyright (c) 2018 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <driver/i2c.h>
#include <esp_log.h>

#include "i2c.h"

static const char* TAG = "i2c";

void i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    ESP_LOGD(TAG, "Starting I2C master at port %d.", i2c_master_port);

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_io_num = I2C_MASTER_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(
        i2c_master_port,
        conf.mode,
        I2C_MASTER_RX_BUF_LEN,
        I2C_MASTER_TX_BUF_LEN,
        0
    ));
}

esp_err_t i2c_master_probe(uint8_t address)
{
    esp_err_t result;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
    i2c_master_stop(cmd);

    result = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return result;
}

uint8_t i2c_read(uint8_t address, uint8_t *buffer, uint16_t length)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(
        i2c_master_write_byte(
            cmd,
            (address << 1) | I2C_MASTER_READ,
            ACK_CHECK_ENABLE
        )
    );
    if (length > 1) {
        ESP_ERROR_CHECK(
            i2c_master_read(cmd, buffer, length - 1, ACK_VAL)
        );
    }
    ESP_ERROR_CHECK(
        i2c_master_read_byte(cmd, buffer + length - 1, NACK_VAL)
    );
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS)
    );
    i2c_cmd_link_delete(cmd);

    return 0;
}

// uint8_t i2c_write(uint8_t address, uint8_t *buffer, uint16_t length)
// {

// }
