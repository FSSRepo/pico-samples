#include <stdio.h>
#include "pico/stdlib.h"
#include "ili9341.h"

int main() {
    ILI9341* sc = new ILI9341(spi1, 11, 10, 8, 6, 9, 15);
    sc->start();
    sc->fillScreen(ILI9341_BLACK);
    sc->drawPixel(8, 8, ILI9341_BLUE);
    return 0;
}