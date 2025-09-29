#include "shift_register.h"

void ShiftRegister::init(uint8_t data,uint8_t latch,uint8_t clock) {
    gpio_init(data);
    gpio_set_dir(data, GPIO_OUT);
    gpio_init(latch);
    gpio_set_dir(latch, GPIO_OUT);
    gpio_init(clock);
    gpio_set_dir(clock, GPIO_OUT);
    this->dataPin = data;
    this->clockPin = clock;
    this->latchPin = latch;
}

void ShiftRegister::update(uint32_t data_packed) {
    for(int o = 0;o < 8; o ++) {
        gpio_put(this->dataPin, (data_packed >> o) & 1);
        gpio_put(this->clockPin, 1);
        sleep_us(1);
        gpio_put(this->clockPin, 0);
    }
    gpio_put(this->latchPin, 1);
    sleep_us(1);
    gpio_put(this->latchPin, 0);
}