#ifndef pcf8574_H
#define pcf8574_H

#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include "esp_task_wdt.h"
#include "driver/i2c.h"


class PCF8574
{
    private:
        uint8_t _address;


    public:
        PCF8574(i2c_port_t port, uint8_t address, gpio_num_t gpio_sda, gpio_num_t gpio_scl);
        bool is_active();
        uint8_t get_address();
        int16_t read_all();
        int16_t read(uint8_t pin);

};



#endif