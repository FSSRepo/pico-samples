#include "display7segments.h"

int digits[] = {
    0b00000010, // 0
    0b10011111, // 1
    0b00100100, // 2
    0b00001100, // 3
    0b10011000, // 4
    0b01001000, // 5
    0b01000000, // 6
    0b00011110, // 7
    0b00000000, // 8
    0b00001000, // 9
    0b11111111, // off
    0b00010000, // A
    0b11000000, // b
    0b11100100, // c
    0b10000100, // d
    0b01100000, // E
    0b01110000, // f
    0b11000100, // o
    0b11110101, // r
    0b11111101 // -
};

Display7Segments::Display7Segments() {
    gpio_init(DIGIT_1);
    gpio_set_dir(DIGIT_1, GPIO_OUT);
    gpio_init(DIGIT_2);
    gpio_set_dir(DIGIT_2, GPIO_OUT);
    reg = new ShiftRegister();
    reg->init();
}

void Display7Segments::setCounter(uint8_t counter) {
    if(counter < 0) {
        return;
    }
    this->counter = counter;
}

void Display7Segments::update() {
    uint8_t unit = 0, deci = 0;
    if (counter >= 0 && counter <= 99)
    {
        unit = counter % 10;
        deci = (int)(counter / 10);
    }
    else if (counter == 200)
    {
        unit = 19;
        deci = 19;
    }
    else
    {
        unit = (counter - 100) + 12;
        deci = (counter - 100) + 11;
    }
    reg->update(digits[deci]);
    gpio_put(DIGIT_1, 0);
    gpio_put(DIGIT_2, 1);
    sleep_ms(2);
    reg->update(digits[unit]);
    gpio_put(DIGIT_2, 0);
    gpio_put(DIGIT_1, 1);
}
