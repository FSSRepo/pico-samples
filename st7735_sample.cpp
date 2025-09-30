#include <stdio.h>
#include "pico/stdlib.h"
#include "st7735.h"

int main() {
    ST7735* sc = new ST7735(spi0, 3, 2, 7, 6, 5);
    sc->start();
    sc->fillScreen(ST7735_WHITE);

    sc->fillRectangle(30, 30, 50, 50, ST7735_BLUE);

    sc->text("Hello world!\nThis is a test", 2, 2, FONT_8x8, ST7735_BLACK);
    return 0;
}