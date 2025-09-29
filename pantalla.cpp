#include "ssd1306.h"
#include "pico/stdlib.h"
#include <stdio.h>

using namespace std;

int main() {
    SSD1306Driver* display = new SSD1306Driver(0, 1, 0, 0x3C, Size::SIZE_128x64);
    display->orientation(0);
    display->rect(20, 5, 50, 50, false);
    display->circle(64, 32, 20, true);
    display->sendBuffer();
}