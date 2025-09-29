#include "ssd1306.h"

#define DELTA_CIRCLE 0.01745f

FrameBuffer::FrameBuffer() {
    this->buffer = new unsigned char[FRAMEBUFFER_SIZE];
}

void FrameBuffer::byteOR(int n, unsigned char byte) {
    // return if index outside 0 - buffer length - 1
    if (n > (FRAMEBUFFER_SIZE-1)) return;
    this->buffer[n] |= byte;
}

void FrameBuffer::byteAND(int n, unsigned char byte) {
    // return if index outside 0 - buffer length - 1
    if (n > (FRAMEBUFFER_SIZE-1)) return;
    this->buffer[n] &= byte;
}

void FrameBuffer::byteXOR(int n, unsigned char byte) {
    // return if index outside 0 - buffer length - 1
    if (n > (FRAMEBUFFER_SIZE-1)) return;
    this->buffer[n] ^= byte;
}

void FrameBuffer::setBuffer(unsigned char *new_buffer) {
    this->buffer = new_buffer;
}

void FrameBuffer::clear() {
    //zeroes out the buffer via memset function from string library
    memset(this->buffer, 0, FRAMEBUFFER_SIZE);
}

unsigned char *FrameBuffer::get() {
    return this->buffer;
}

SSD1306Driver::SSD1306Driver(int16_t sda, int16_t scl, int8_t i2c_inst, uint16_t Address, Size size)
{
    i2c_init(i2c_inst == 0 ? i2c0 : i2c1, 400'000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);

    // Set class instanced variables
    this->i2CInst = i2c_inst == 0 ? i2c0 : i2c1;

    this->address = Address;

    this->size = size;

    this->width = 128;

    if (size == Size::SIZE_128x32)
    {
        this->height = 32;
    }
    else
    {
        this->height = 64;
    }

    // create a frame buffer
    this->frameBuffer = FrameBuffer();

    // display is not inverted by default
    this->inverted = false;

    // this is a list of setup commands for the display
    uint8_t setup[] = {
        SSD1306_DISPLAY_OFF,
        SSD1306_LOWCOLUMN,
        SSD1306_HIGHCOLUMN,
        SSD1306_STARTLINE,

        SSD1306_MEMORYMODE,
        SSD1306_MEMORYMODE_HORZONTAL,

        SSD1306_CONTRAST,
        0xFF,

        SSD1306_INVERTED_OFF,

        SSD1306_MULTIPLEX,
        63,

        SSD1306_DISPLAYOFFSET,
        0x00,

        SSD1306_DISPLAYCLOCKDIV,
        0x80,

        SSD1306_PRECHARGE,
        0x22,

        SSD1306_COMPINS,
        0x12,

        SSD1306_VCOMDETECT,
        0x40,

        SSD1306_CHARGEPUMP,
        0x14,

        SSD1306_DISPLAYALL_ON_RESUME,
        SSD1306_DISPLAY_ON};

    // send each one of the setup commands
    for (uint8_t &command : setup)
    {
        this->cmd(command);
    }

    // clear the buffer and send it to the display
    // if not done display shows garbage data
    this->clear();
    this->sendBuffer();
}

void SSD1306Driver::setPixel(int16_t x, int16_t y, WriteMode mode)
{
    // return if position out of bounds
    if ((x < 0) || (x >= this->width) || (y < 0) || (y >= this->height))
        return;

    // byte to be used for buffer operation
    uint8_t byte;

    if (size == Size::SIZE_128x32)
    {
        y = (y << 1) + 1;
        byte = 1 << (y & 7);
        char byte_offset = byte >> 1;
        byte = byte | byte_offset;
    }
    else
    {
        byte = 1 << (y & 7);
    }

    // check the write mode and manipulate the frame buffer
    if (mode == WriteMode::Add)
    {
        this->frameBuffer.byteOR(x + (y / 8) * this->width, byte);
    }
    else if (mode == WriteMode::Subtract)
    {
        this->frameBuffer.byteAND(x + (y / 8) * this->width, ~byte);
    }
    else if (mode == WriteMode::Invert)
    {
        this->frameBuffer.byteXOR(x + (y / 8) * this->width, byte);
    }
}

void SSD1306Driver::sendBuffer()
{
    this->cmd(SSD1306_PAGEADDR); // Set page address from min to max
    this->cmd(0x00);
    this->cmd(0x07);
    this->cmd(SSD1306_COLUMNADDR); // Set column address from min to max
    this->cmd(0x00);
    this->cmd(127);

    // create a temporary buffer of size of buffer plus 1 byte for startline command aka 0x40
    unsigned char data[FRAMEBUFFER_SIZE + 1];

    data[0] = SSD1306_STARTLINE;

    // copy framebuffer to temporary buffer
    memcpy(data + 1, frameBuffer.get(), FRAMEBUFFER_SIZE);

    // send data to device
    i2c_write_blocking(this->i2CInst, this->address, data, FRAMEBUFFER_SIZE + 1, false);
}

void SSD1306Driver::clear()
{
    this->frameBuffer.clear();
}

void SSD1306Driver::orientation(bool orientation)
{
    // remap columns and rows scan direction, effectively flipping the image on display
    if (orientation)
    {
        this->cmd(SSD1306_CLUMN_REMAP_OFF);
        this->cmd(SSD1306_COM_REMAP_OFF);
    }
    else
    {
        this->cmd(SSD1306_CLUMN_REMAP_ON);
        this->cmd(SSD1306_COM_REMAP_ON);
    }
}

void SSD1306Driver::image(uint8_t offset_x, uint8_t offset_y, uint8_t width, uint8_t height,
                                   uint8_t *image_data,
                                   WriteMode mode)
{
    uint8_t byte;
    // goes over every single bit in image and sets pixel data on its coordinates
    for (uint8_t y = 0; y < height; y++)
    {
        for (uint8_t x = 0; x < width / 8; x++)
        {
            byte = image_data[y * (width / 8) + x];
            for (uint8_t z = 0; z < 8; z++)
            {
                if ((byte >> (7 - z)) & 1)
                {
                    this->setPixel(x * 8 + z + offset_x, y + offset_y, mode);
                }
            }
        }
    }
}

void SSD1306Driver::text(const char* text, uint8_t offset_x, uint8_t offset_y, uint8_t font, WriteMode mode) {
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
                    setPixel(x + offset_x + (n * font_width), y + offset_y + (font_height + 1) * line, mode);
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

void SSD1306Driver::line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, WriteMode mode) {
    int x, y, dx, dy, dx0, dy0, px, py, xe, ye, i;
    dx = x1 - x0;
    dy = y1 - y0;
    dx0 = fabs(dx);
    dy0 = fabs(dy);
    px = 2 * dy0 - dx0;
    py = 2 * dx0 - dy0;
    if (dy0 <= dx0) {
        if (dx >= 0) {
            x = x0;
            y = y0;
            xe = x1;
        } else {
            x = x1;
            y = y1;
            xe = x0;
        }
        setPixel(x, y, mode);
        for (i = 0; x < xe; i++) {
            x = x + 1;
            if (px < 0) {
                px = px + 2 * dy0;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    y = y + 1;
                } else {
                    y = y - 1;
                }
                px = px + 2 * (dy0 - dx0);
            }
            setPixel(x, y, mode);
        }
    } else {
        if (dy >= 0) {
            x = x0;
            y = y0;
            ye = y1;
        } else {
            x = x1;
            y = y1;
            ye = y0;
        }
        setPixel(x, y, mode);
        for (i = 0; y < ye; i++) {
            y = y + 1;
            if (py <= 0) {
                py = py + 2 * dx0;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    x = x + 1;
                } else {
                    x = x - 1;
                }
                py = py + 2 * (dx0 - dy0);
            }
            setPixel(x, y, mode);
        }
    }
}

void SSD1306Driver::rect(uint8_t offset_x, uint8_t offset_y, uint8_t width, uint8_t height, bool fill, WriteMode mode) {
   if(fill) {
        for (uint8_t x = 0; x < width; x++)
        {
            for (uint8_t y = 0; y < height; y++)
            {
                setPixel(offset_x + x, offset_y + y, mode);
            }
        }
    } else {
        line(offset_x, offset_y, offset_x + width, offset_y, mode);
        line(offset_x, offset_y, offset_x, offset_y + height, mode);
        line(offset_x + width, offset_y, offset_x + width, offset_y + height, mode);
        line(offset_x, offset_y + height, offset_x + width, offset_y  + height, mode);
    }
}

void SSD1306Driver::circle(uint8_t center_x, uint8_t center_y, uint8_t radius, bool fill, WriteMode mode) {
    if(fill){
        int8_t start_x = (center_x - radius), end_x = (center_x + radius);
        int8_t start_y = (center_y - radius), end_y = (center_y + radius);
        int16_t r2 = radius * radius * 0.95f;
        for (uint8_t x = start_x; x < end_x; x++)
        {
            int16_t dx = (x - center_x);
            int16_t x2 = dx * dx;
            for (int8_t y = start_y; y < end_y; y++)
            {
                int8_t dy = (y - center_y);
                uint16_t dc = x2 + dy * dy;
                if (dc <= r2)
                {
                    setPixel(x, y, mode);
                }
            }
        }
    } else {
        float angle = 0;
        for (uint16_t s = 0; s < 360; s++)
        {
            setPixel((int16_t)(cos(angle) * radius) + center_x, (int16_t)(sin(angle) * radius) + center_y, mode);
            angle += DELTA_CIRCLE;
        }
    }
}

void SSD1306Driver::invert()
{
    this->cmd(SSD1306_INVERTED_OFF | !this->inverted);
    inverted = !inverted;
}

void SSD1306Driver::cmd(unsigned char command)
{
    // 0x00 is a byte indicating to ssd1306 that a command is being sent
    uint8_t data[2] = {0x00, command};
    i2c_write_blocking(this->i2CInst, this->address, data, 2, false);
}

void SSD1306Driver::contrast(unsigned char contrast)
{
    this->cmd(SSD1306_CONTRAST);
    this->cmd(contrast);
}

void SSD1306Driver::setBuffer(unsigned char *buffer)
{
    this->frameBuffer.setBuffer(buffer);
}