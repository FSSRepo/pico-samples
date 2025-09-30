#include "st7735.h"

#include "fonts/8x8_font.h"
#include "fonts/12x16_font.h"

int _width = 128, _height = 160;

ST7735::ST7735(spi_inst_t *spi, int mosi, int sck, int cs, int dc, int rst) : cs(cs), dc(dc), rst(rst), spi_port(spi)
{
    spi_init(spi_port, 8000000);
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    gpio_set_function(sck, GPIO_FUNC_SPI);

    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 1);

    gpio_init(dc);
    gpio_set_dir(dc, GPIO_OUT);
    gpio_put(dc, 1);

    gpio_init(rst);
    gpio_set_dir(rst, GPIO_OUT);
    gpio_put(rst, 1);
}

void ST7735::first_cmd()
{
    write_command(ST7735_SWRESET);
    sleep_ms(150);
    write_command(ST7735_SLPOUT);
    sleep_ms(250);

    write_command(ST7735_FRMCTR1);
    write_data(0x01);
    write_data(0x2C);
    write_data(0x2D);

    write_command(ST7735_FRMCTR2);
    write_data(0x01);
    write_data(0x2C);
    write_data(0x2D);

    write_command(ST7735_FRMCTR3);
    write_data(0x01);
    write_data(0x2C);
    write_data(0x2D);
    write_data(0x01);
    write_data(0x2C);
    write_data(0x2D);

    write_command(ST7735_INVCTR);
    write_data(0x07);

    write_command(ST7735_PWCTR1);
    write_data(0xA2);
    write_data(0x02);
    write_data(0x84);

    write_command(ST7735_PWCTR2);
    write_data(0xC5);

    write_command(ST7735_PWCTR3);
    write_data(0x0A);
    write_data(0x00);

    write_command(ST7735_PWCTR4);
    write_data(0x8A);
    write_data(0x2A);

    write_command(ST7735_PWCTR5);
    write_data(0x8A);
    write_data(0xEE);

    write_command(ST7735_VMCTR1);
    write_data(0x0E);

    write_command(ST7735_INVOFF);
    write_command(ST7735_MADCTL);
    write_data(0xC0);

    write_command(ST7735_COLMOD);
    write_data(0x05); // 16 bits
}

void ST7735::enable() {
    // black pcb activation
    write_command(ST7735_CASET);
    write_data(0x00); write_data(0x00);
    write_data(0x00); write_data(0x7F);
    write_command(ST7735_RASET);
    write_data(0x00); write_data(0x00);
    write_data(0x00); write_data(0x9F);
}

void ST7735::final_cmd()
{
    write_command(ST7735_GMCTRP1);
    write_data(0x02);
    write_data(0x1C);
    write_data(0x07);
    write_data(0x12);
    write_data(0x37);
    write_data(0x32);
    write_data(0x29);
    write_data(0x2D);
    write_data(0x29);
    write_data(0x25);
    write_data(0x2B);
    write_data(0x39);
    write_data(0x00);
    write_data(0x01);
    write_data(0x03);
    write_data(0x10);
    write_command(ST7735_GMCTRN1);
    write_data(0x03);
    write_data(0x1D);
    write_data(0x07);
    write_data(0x06);
    write_data(0x2E);
    write_data(0x2C);
    write_data(0x29);
    write_data(0x2D);
    write_data(0x2E);
    write_data(0x2E);
    write_data(0x37);
    write_data(0x3F);
    write_data(0x00);
    write_data(0x00);
    write_data(0x02);
    write_data(0x10);
    write_command(ST7735_NORON);
    sleep_ms(10);
    write_command(ST7735_DISPON);
    sleep_ms(100);
}

void ST7735::start() {
    reset();
    first_cmd();
    enable();
    final_cmd();
}

void ST7735::setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    write_command(ST7735_CASET);
    write_data(0);
    write_data(x0);
    write_data(0);
    write_data(x1);
    write_command(ST7735_RASET);
    write_data(0);
    write_data(y0);
    write_data(0);
    write_data(y1);
    write_command(ST7735_RAMWR); // Write to RAM
}

void ST7735::fillRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
    uint8_t hi, lo;
    if ((x >= _width) || (y >= _height))
        return;
    if ((x + w - 1) >= _width)
        w = _width - x;
    if ((y + h - 1) >= _height)
        h = _height - y;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    hi = color >> 8;
    lo = color;
    tft_dc_high();
    tft_cs_low();
    for (y = h; y > 0; y--)
    {
        for (x = w; x > 0; x--)
        {
            spiwrite(hi);
            spiwrite(lo);
        }
    }
    tft_cs_high();
}

void ST7735::fillScreen(uint16_t color) {
    fillRectangle(0, 0, _width, _height, color);
}

void ST7735::drawPixel(uint8_t x, uint8_t y, uint16_t color) {
    setAddrWindow(x, y, x + 1, y + 1);
    tft_dc_high();
    tft_cs_low();
    uint8_t hi = color >> 8;
    uint8_t lo = color;
    spiwrite(hi);
    spiwrite(lo);
    tft_cs_high();
}

void ST7735::text(const char* text, uint8_t offset_x, uint8_t offset_y, uint8_t font, uint16_t color) {
    const unsigned char* font_data = (font == FONT_8x8 ? font_8x8 : font_12x16);
    uint8_t font_width = font_data[0];
    uint8_t font_height = font_data[1];
    uint16_t n = 0;
    uint16_t text_ofs = 0;
    uint8_t line = 0;
    while (text[text_ofs] != '\0') {
        if (text[text_ofs] == '\n') {
            n = 0;
            line++;
            text_ofs++;
            continue;
        }
        if (text[text_ofs] < 32) {
            text_ofs++; n++;
            continue;
        }
        uint16_t seek = (text[text_ofs] - 32) * (font_width * font_height) / 8 + 2;
        uint8_t b_seek = 0;
        for (uint8_t x = 0; x < font_width; x++) {
            for (uint8_t y = 0; y < font_height; y++) {
                if (font_data[seek] >> b_seek & 0b00000001) {
                    drawPixel(x + offset_x + (n * font_width), y + offset_y + (font_height + 1) * line, color);
                }
                b_seek++;
                if (b_seek == 8) {
                    b_seek = 0;
                    seek++;
                }
            }
        }
        n++;
        text_ofs++;
    }
}
