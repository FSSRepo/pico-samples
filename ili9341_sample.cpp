#include <stdio.h>
#include "pico/stdlib.h"
#include "ili9341.h"

int main() {
    ILI9341* sc = new ILI9341(spi1, 11, 10, 8, 6, 9, 15);
    sc->start();
    sc->fillScreen(ILI9341_BLACK);
    sc->fillRectangle(10, 10, 40, 40, ILI9341_RED);
    sc->fillRectangle(50, 50, 40, 40, ILI9341_GREEN);
    sc->fillRectangle(90, 90, 40, 40, ILI9341_BLUE);
    sc->fillRectangle(130, 130, 40, 40, ILI9341_WHITE);

    sc->drawPixel(50, 170, ILI9341_CYAN);

    sc->text("Hello world!\nThis is a test", 20, 180, FONT_8x8, ILI9341_YELLOW);
    return 0;
}