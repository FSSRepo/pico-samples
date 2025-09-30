#ifndef ILI9341_h
#define ILI9341_h

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdint.h>
#include <stdlib.h>

#define ILI9341_NOP         0x00 ///< No-op register
#define ILI9341_SWRESET     0x01 ///< Software reset register
#define ILI9341_RDDID       0x04 ///< Read display identification information
#define ILI9341_RDDST       0x09 ///< Read Display Status
#define ILI9341_SLPIN       0x10 ///< Enter Sleep Mode
#define ILI9341_SLPOUT      0x11 ///< Sleep Out
#define ILI9341_PTLON       0x12 ///< Partial Mode ON
#define ILI9341_NORON       0x13 ///< Normal Display Mode ON

#define ILI9341_CASET       0x2A ///< Column Address Set
#define ILI9341_PASET       0x2B ///< Page Address Set
#define ILI9341_RAMWR       0x2C ///< Memory Write
#define ILI9341_RAMRD       0x2E ///< Memory Read

#define MADCTL_MY           0x80  ///< Bottom to top
#define MADCTL_MX           0x40  ///< Right to left
#define MADCTL_MV           0x20  ///< Reverse Mode
#define MADCTL_ML           0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB          0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR          0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH           0x04  ///< LCD refresh right to left

#define ILI9341_FRMCTR1     0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2     0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3     0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR      0xB4 ///< Display Inversion Control
#define ILI9341_DFUNCTR     0xB6 ///< Display Function Control

#define ILI9341_PWCTR1      0xC0 ///< Power Control 1
#define ILI9341_PWCTR2      0xC1 ///< Power Control 2
#define ILI9341_PWCTR3      0xC2 ///< Power Control 3
#define ILI9341_PWCTR4      0xC3 ///< Power Control 4
#define ILI9341_PWCTR5      0xC4 ///< Power Control 5
#define ILI9341_VMCTR1      0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2      0xC7 ///< VCOM Control 2

#define ILI9341_PTLAR       0x30 ///< Partial Area
#define ILI9341_VSCRDEF     0x33 ///< Vertical Scrolling Definition
#define ILI9341_MADCTL      0x36 ///< Memory Access Control
#define ILI9341_VSCRSADD    0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT      0x3A ///< COLMOD: Pixel Format Set

#define ILI9341_INVOFF      0x20 ///< Display Inversion OFF
#define ILI9341_INVON       0x21 ///< Display Inversion ON
#define ILI9341_GAMMASET    0x26 ///< Gamma Set
#define ILI9341_DISPOFF     0x28 ///< Display OFF
#define ILI9341_DISPON      0x29 ///< Display ON

#define ILI9341_VSCRSADD    0x37 ///< Vertical Scrolling Start Address
#define ILI9341_GAMMASET    0x26 ///< Gamma Set
#define ILI9341_GMCTRP1     0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1     0xE1 ///< Negative Gamma Correction

#define   ILI9341_BLACK   0x0000
#define   ILI9341_BLUE    0x001F
#define   ILI9341_RED     0xF800
#define   ILI9341_GREEN   0x07E0
#define   ILI9341_CYAN    0x07FF
#define   ILI9341_MAGENTA 0xF81F
#define   ILI9341_YELLOW  0xFFE0
#define   ILI9341_WHITE   0xFFFF

#define FONT_8x8 0
#define FONT_12x16 1

class ILI9341
{
private:
    int cs, dc, rst;
    spi_inst_t *spi_port;

    void spiwrite(uint8_t b)
    {
        spi_write_blocking(spi_port, &b, 1);
    }

    inline void tft_cs_low() { gpio_put(cs, 0); }
    inline void tft_cs_high() { gpio_put(cs, 1); }
    inline void tft_dc_low() { gpio_put(dc, 0); }
    inline void tft_dc_high() { gpio_put(dc, 1); }

    void write_command(uint8_t cmd)
    {
        tft_cs_low();
        tft_dc_low();
        spi_write_blocking(spi_port, &cmd, 1);
        tft_cs_high();
    }
    void write_data(uint8_t data)
    {
        tft_cs_low();
        tft_dc_high();
        spi_write_blocking(spi_port, &data, 1);
        tft_cs_high();
    }

    void write_data_buffer(uint8_t *data, size_t data_size)
    {
        tft_cs_low();
        tft_dc_high();
        spi_write_blocking(spi_port, data, data_size);
        tft_cs_high();
    }


public:
    ILI9341(spi_inst_t *spi, int mosi, int sck, int cs, int dc, int rst, int led);

    void start();

    void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void fillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

    void fillScreen(uint16_t color);

    void drawPixel(int x, int y, uint16_t color);

    void text(const char* text, uint8_t offset_x, uint8_t offset_y, uint8_t font, uint16_t color);
};
#endif