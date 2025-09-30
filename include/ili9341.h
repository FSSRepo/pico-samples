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

#define     ILI9341_BLACK   0x0000
#define     ILI9341_BLUE    0x001F
#define     ILI9341_RED     0xF800
#define     ILI9341_GREEN   0x07E0

uint8_t initcmd[] = {
    24, // 24 commands
    0xEF, 3, 0x03, 0x80, 0x02,
    0xCF, 3, 0x00, 0xC1, 0x30,
    0xED, 4, 0x64, 0x03, 0x12, 0x81,
    0xE8, 3, 0x85, 0x00, 0x78,
    0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    0xF7, 1, 0x20,
    0xEA, 2, 0x00, 0x00,
    ILI9341_PWCTR1, 1, 0x23,       // Power control VRH[5:0]
    ILI9341_PWCTR2, 1, 0x10,       // Power control SAP[2:0];BT[3:0]
    ILI9341_VMCTR1, 2, 0x3e, 0x28, // VCM control
    ILI9341_VMCTR2, 1, 0x86,       // VCM control2
    ILI9341_MADCTL, 1, 0x48,       // Memory Access Control
    ILI9341_VSCRSADD, 1, 0x00,     // Vertical scroll zero
    ILI9341_PIXFMT, 1, 0x55,
    ILI9341_FRMCTR1, 2, 0x00, 0x18,
    ILI9341_DFUNCTR, 3, 0x08, 0x82, 0x27,                    // Display Function Control
    0xF2, 1, 0x00,                                           // 3Gamma Function Disable
    ILI9341_GAMMASET, 1, 0x01,                               // Gamma curve selected
    ILI9341_GMCTRP1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    ILI9341_GMCTRN1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    ILI9341_SLPOUT, 0x80, // Exit Sleep
    ILI9341_DISPON, 0x80, // Display on
    0x00                  // End of list
};

int def_width = 240, def_height = 320;

class ILI9341
{
private:
    int cs, dc, rst;
    spi_inst_t *spi_port;

public:
    ILI9341(spi_inst_t *spi, int mosi, int sck, int cs, int dc, int rst, int led) : cs(cs), dc(dc), rst(rst), spi_port(spi)
    {
        spi_init(spi, 8000000);
        spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
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

        gpio_init(led);
        gpio_set_dir(led, GPIO_OUT);
        gpio_put(led, 1);
    }

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

    void start()
    {
        gpio_put(rst, 0);
        sleep_ms(5);
        gpio_put(rst, 1);
        sleep_ms(150);

        uint8_t *addr = initcmd;
        uint8_t numCommands, cmd, numArgs;
        uint16_t ms;
        numCommands = *(addr++); // Number of commands to follow
        while (numCommands--)
        {                    // For each command...
            cmd = *(addr++); // Read command
            // numArgs = *(addr++);		 // Number of args to follow
            uint8_t x = *(addr++);
            numArgs = x & 0x7F; // Mask out delay bit

            tft_cs_low();
            write_command(cmd);
            write_data_buffer(addr, numArgs);
            tft_cs_high();
            addr += numArgs;

            if (x & 0x80)
                sleep_ms(150);
        }

        tft_cs_low();
        write_command(ILI9341_MADCTL);
        write_data(MADCTL_MX | MADCTL_BGR);
        tft_cs_high();
    }

    void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
    {
        static uint16_t old_x1 = 0xFFFF, old_x2 = 0xFFFF;
        static uint16_t old_y1 = 0xFFFF, old_y2 = 0xFFFF;

        if (x1 != old_x1 || x2 != old_x2) {
            uint32_t xa = ((uint32_t)x1 << 16) | x2;   // empaquetar (x1, x2)
            xa = __builtin_bswap32(xa);                // invertir endian
            write_command(ILI9341_CASET);              // Column address set
            write_data_buffer((uint8_t*)&xa, 4);
            old_x1 = x1;
            old_x2 = x2;
        }

        if (y1 != old_y1 || y2 != old_y2) {
            uint32_t ya = ((uint32_t)y1 << 16) | y2;   // empaquetar (y1, y2)
            ya = __builtin_bswap32(ya);
            write_command(ILI9341_PASET);              // Row address set
            write_data_buffer((uint8_t*)&ya, 4);
            old_y1 = y1;
            old_y2 = y2;
        }

        write_command(ILI9341_RAMWR); // Write to RAM
    }


    void fillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
    {
        uint8_t hi, lo;
        if ((x >= def_width) || (y >= def_height))
            return;
        if ((x + w - 1) >= def_width)
            w = def_width - x;
        if ((y + h - 1) >= def_height)
            h = def_height - y;
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

    void fillScreen(uint16_t color)
    {
        fillRectangle(0, 0, def_width, def_height, color);
    }

    void drawPixel(int x, int y, uint16_t color)
    {
        setAddrWindow(x, y, x + 1, y + 1);

        tft_dc_high();
        tft_cs_low();
        uint8_t hi = color >> 8;
        uint8_t lo = color;
        spiwrite(hi);
        spiwrite(lo);
        tft_cs_high();
    }
};
#endif