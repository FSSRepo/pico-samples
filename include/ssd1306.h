#pragma once

#ifndef SSD1306_SSD1306_H
#define SSD1306_SSD1306_H

#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "fonts/8x8_font.h"
#include "fonts/12x16_font.h"
#include <math.h>

#define FONT_8x8 0
#define FONT_12x16 1

/// Register addresses from datasheet
enum REG_ADDRESSES : unsigned char
{
    SSD1306_CONTRAST = 0x81,
    SSD1306_DISPLAYALL_ON_RESUME = 0xA4,
    SSD1306_DISPLAYALL_ON = 0xA5,
    SSD1306_INVERTED_OFF = 0xA6,
    SSD1306_INVERTED_ON = 0xA7,
    SSD1306_DISPLAY_OFF = 0xAE,
    SSD1306_DISPLAY_ON = 0xAF,
    SSD1306_DISPLAYOFFSET = 0xD3,
    SSD1306_COMPINS = 0xDA,
    SSD1306_VCOMDETECT = 0xDB,
    SSD1306_DISPLAYCLOCKDIV = 0xD5,
    SSD1306_PRECHARGE = 0xD9,
    SSD1306_MULTIPLEX = 0xA8,
    SSD1306_LOWCOLUMN = 0x00,
    SSD1306_HIGHCOLUMN = 0x10,
    SSD1306_STARTLINE = 0x40,
    SSD1306_MEMORYMODE = 0x20,
    SSD1306_MEMORYMODE_HORZONTAL = 0x00,
    SSD1306_MEMORYMODE_VERTICAL = 0x01,
    SSD1306_MEMORYMODE_PAGE = 0x10,
    SSD1306_COLUMNADDR = 0x21,
    SSD1306_PAGEADDR = 0x22,
    SSD1306_COM_REMAP_OFF = 0xC0,
    SSD1306_COM_REMAP_ON = 0xC8,
    SSD1306_CLUMN_REMAP_OFF = 0xA0,
    SSD1306_CLUMN_REMAP_ON = 0xA1,
    SSD1306_CHARGEPUMP = 0x8D,
    SSD1306_EXTERNALVCC = 0x1,
    SSD1306_SWITCHCAPVCC = 0x2,
};

enum class Size
{
    SIZE_128x64,
    SIZE_128x32
};

/// \enum pico_ssd1306::WriteMode
enum class WriteMode : const unsigned char
{
    /// sets pixel on regardless of its state
    Add = 0,
    /// sets pixel off regardless of its state
    Subtract = 1,
    /// inverts pixel, so 1->0 or 0->1
    Invert = 2
};

#define FRAMEBUFFER_SIZE 1024

class FrameBuffer {
    unsigned char * buffer;
public:
    FrameBuffer();

    void byteOR(int n, unsigned char byte);

    void byteAND(int n, unsigned char byte);

    void byteXOR(int n, unsigned char byte);

    void setBuffer(unsigned char * new_buffer);

    void clear();

    unsigned char * get();
};

class SSD1306Driver
{
private:
    i2c_inst *i2CInst;
    uint16_t address;
    Size size;

    FrameBuffer frameBuffer;

    uint8_t width, height;

    bool inverted;

    void cmd(unsigned char command);

public:

    SSD1306Driver(int16_t sda,int16_t scl, int8_t i2c_inst, uint16_t Address, Size size);

    void setPixel(int16_t x, int16_t y, WriteMode mode = WriteMode::Add);

    void sendBuffer();

    void image(uint8_t offset_x, uint8_t offset_y, uint8_t width, uint8_t height,
                                   uint8_t *image_data,
                                   WriteMode mode = WriteMode::Add);

    void text(const char* text, uint8_t offset_x, uint8_t offset_y, uint8_t font = FONT_8x8, WriteMode mode = WriteMode::Add);

    void line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, WriteMode mode = WriteMode::Add);

    void rect(uint8_t offset_x, uint8_t offset_y, uint8_t width, uint8_t height, bool fill, WriteMode mode = WriteMode::Add);

    void circle(uint8_t center_x, uint8_t center_y, uint8_t radius, bool fill, WriteMode mode = WriteMode::Add);

    void setBuffer(unsigned char *buffer);

    void orientation(bool orientation);

    void clear();

    void invert();

    void contrast(unsigned char contrast);
};

#endif // SSD1306_SSD1306_H