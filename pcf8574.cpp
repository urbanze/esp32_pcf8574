#include "pcf8574.h"

static bool _initialized;
static i2c_port_t _port;
static SemaphoreHandle_t _smf;


PCF8574::PCF8574(i2c_port_t port, uint8_t address, gpio_num_t gpio_sda, gpio_num_t gpio_scl)
{
    if (!_initialized)
    {
        _smf = xSemaphoreCreateBinary();
        i2c_config_t cfg;
        cfg.mode = I2C_MODE_MASTER;
        cfg.sda_io_num = gpio_sda;
        cfg.sda_pullup_en = GPIO_PULLUP_DISABLE;
        cfg.scl_io_num = gpio_scl;
        cfg.scl_pullup_en = GPIO_PULLUP_DISABLE;
        cfg.master.clk_speed = 100000;

        ESP_ERROR_CHECK(i2c_param_config(port, &cfg));
        ESP_ERROR_CHECK(i2c_driver_install(port, cfg.mode, 0, 0, 0));

        _smf = xSemaphoreCreateBinary();
        xSemaphoreGive(_smf);
        _port = port;
        _address = address;
        _initialized = true;

        ESP_LOGI(__func__, "Initialized OK");
    }
    else
    {
        if (port != _port)
        {
            ESP_LOGE(__func__, "Error, cant init another i2c_port");
        }
        else
        {
            _port = port;
            _address = address;
        }
    }
}

bool PCF8574::is_active()
{
    if (_initialized)
    {
        int16_t value = read_all();
        if (value >= 0 && value <= 255)
        {
            return true;
        }
    }

    return false;
}

uint8_t PCF8574::get_address()
{
    return _address;
}

int16_t PCF8574::read_all()
{
    if (xSemaphoreTake(_smf, pdMS_TO_TICKS(100)))
    {
        uint8_t data = 0;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, _address|I2C_MASTER_READ, 1);
        i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
        i2c_master_stop(cmd);

        esp_err_t err = i2c_master_cmd_begin(_port, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(cmd);


        if (err == ESP_OK)
        {
            xSemaphoreGive(_smf);
            return data;
        }
        else
        {
            ESP_LOGE(__func__, "Addr: [%d] => I2C error: [0x%x]", _address, err);
        }

        xSemaphoreGive(_smf);
    }
    else
    {
        ESP_LOGE(__func__, "Addr: [%d] => Fail to take semaphore", _address);
    }

    return -1;
}

int16_t PCF8574::read(uint8_t pin)
{
    if (pin < 0 || pin > 7)
    {
        ESP_LOGE(__func__, "Addr: [%d] => Pin[%d] error, must be 0-7", _address, pin);
        return-1;
    }


    int16_t values = read_all();
    if (values == -1)
    {
        ESP_LOGE(__func__, "Addr: [%d] => Error to read pin: [%d]", _address, pin);
        return -1;
    }

    return ((values >> pin) & 1);
}