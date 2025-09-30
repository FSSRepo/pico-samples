#ifndef ST7735_h
#define ST7735_h
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdint.h>
#include <stdlib.h>

#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT 0x11
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5
#define ST7735_INVOFF 0x20
#define ST7735_MADCTL 0x36
#define ST7735_COLMOD 0x3A

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1
#define ST7735_NORON 0x13
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E

#define FONT_8x8 0
#define FONT_12x16 1

#define   ST7735_BLACK   0x0000
#define   ST7735_BLUE    0x001F
#define   ST7735_RED     0xF800
#define   ST7735_GREEN   0x07E0
#define   ST7735_CYAN    0x07FF
#define   ST7735_MAGENTA 0xF81F
#define   ST7735_YELLOW  0xFFE0
#define   ST7735_WHITE   0xFFFF

class ST7735
{
private:
    int cs, dc, rst;
    spi_inst_t *spi_port;
    void first_cmd();
    void enable();
    void final_cmd();
    void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

public:
    ST7735(spi_inst_t *spi, int mosi, int sck, int cs, int dc, int rst);

    inline void tft_cs_low() { gpio_put(cs, 0); }
    inline void tft_cs_high() { gpio_put(cs, 1); }
    inline void tft_dc_low() { gpio_put(dc, 0); }
    inline void tft_dc_high() { gpio_put(dc, 1); }

    void reset()
    {
        gpio_put(rst, 0);
        sleep_ms(50);
        gpio_put(rst, 1);
        sleep_ms(150);
    }

    void spiwrite(uint8_t b)
    {
        spi_write_blocking(spi_port, &b, 1);
    }

    void write_command(uint8_t cmd)
    {
        tft_dc_low();
        tft_cs_low();
        spiwrite(cmd);
        tft_cs_high();
    }

    void write_data(uint8_t data)
    {
        tft_dc_high();
        tft_cs_low();
        spiwrite(data);
        tft_cs_high();
    }

    void start();

    void fillRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);

    void fillScreen(uint16_t color);

    void drawPixel(uint8_t x, uint8_t y, uint16_t color);
    
    void text(const char* text, uint8_t x, uint8_t y, uint8_t font, uint16_t color);
};
#endif
