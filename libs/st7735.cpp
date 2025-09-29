

void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    write_command(ST7735_CASET);
    write_data(0);
    write_data(x0 + _xstart);
    write_data(0);
    write_data(x1 + _xstart);
    write_command(ST7735_RASET);
    write_data(0);
    write_data(y0 + _ystart);
    write_data(0);
    write_data(y1 + _ystart);
    write_command(ST7735_RAMWR); // Write to RAM 
}