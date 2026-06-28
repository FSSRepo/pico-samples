//
//    FILE: SHT31.cpp
//  AUTHOR: Rob Tillaart (adaptado para Raspberry Pi Pico SDK)
// VERSION: 0.5.3-pico
//    DATE: 2019-02-08
// PURPOSE: Libreria para el sensor de temperatura y humedad SHT31
//          Adaptada para Raspberry Pi Pico SDK
//     URL: https://github.com/RobTillaart/SHT31

#include "SHT31.h"

//  SUPPORTED COMMANDS - single shot mode only
static constexpr uint16_t SHT31_READ_STATUS       = 0xF32D;
static constexpr uint16_t SHT31_CLEAR_STATUS      = 0x3041;

static constexpr uint16_t SHT31_SOFT_RESET        = 0x30A2;
static constexpr uint16_t SHT31_HARD_RESET        = 0x0006;

static constexpr uint16_t SHT31_MEASUREMENT_FAST  = 0x2416;     //  page 10 datasheet
static constexpr uint16_t SHT31_MEASUREMENT_SLOW  = 0x2400;     //  no clock stretching

static constexpr uint16_t SHT31_HEAT_ON           = 0x306D;
static constexpr uint16_t SHT31_HEAT_OFF          = 0x3066;
static constexpr uint32_t SHT31_HEATER_TIMEOUT    = 180000UL;   //  milliseconds

static constexpr uint16_t SHT31_GET_SERIAL_NUMBER = 0x3682;     //  no clock stretching


SHT31::SHT31(uint8_t address, i2c_inst_t *i2c)
{
  _address        = address;
  _i2c            = i2c;
  _lastRead       = 0;
  _rawTemperature = 0;
  _rawHumidity    = 0;
  _heatTimeout    = 0;
  _heaterStart    = 0;
  _heaterStop     = 0;
  _heaterOn       = false;
  _error          = SHT31_OK;
}


bool SHT31::begin()
{
  if ((_address != 0x44) && (_address != 0x45))
  {
    return false;
  }
  if (_i2c == nullptr)
  {
    _i2c = i2c0;  // default I2C port
  }
  return reset();
}


bool SHT31::isConnected()
{
  uint8_t dummy = 0;
  int rv = i2c_read_blocking(_i2c, _address, &dummy, 1, false);
  if (rv < 0)
  {
    _error = SHT31_ERR_NOT_CONNECT;
    return false;
  }
  return true;
}


uint8_t SHT31::getAddress()
{
  return _address;
}


bool SHT31::read(bool fast)
{
  if (writeCmd(fast ? SHT31_MEASUREMENT_FAST : SHT31_MEASUREMENT_SLOW) == false)
  {
    return false;
  }
  sleep_ms(fast ? 4 : 15);   //  table 4 datasheet
  return readData(fast);
}


/////////////////////////////////////////////////////////////////
//
//  STATUS
//

uint16_t SHT31::readStatus()
{
  uint8_t status[3] = { 0, 0, 0 };
  //  page 13 datasheet
  if (writeCmd(SHT31_READ_STATUS) == false)
  {
    return 0xFFFF;
  }
  //  16 bit status + CRC
  if (readBytes(3, (uint8_t*) &status[0]) == false)
  {
    return 0xFFFF;
  }

  if (status[2] != crc8(status, 2))
  {
    _error = SHT31_ERR_CRC_STATUS;
    return 0xFFFF;
  }
  return (uint16_t) (status[0] << 8) + status[1];
}


//  resets the following bits
//  15  Alert pending status
//  11  Humidity tracking alert
//  10  Temp tracking alert
//  4   System reset detected
bool SHT31::clearStatus()
{
  if (writeCmd(SHT31_CLEAR_STATUS) == false)
  {
    return false;
  }
  return true;
}


bool SHT31::reset(bool hard)
{
  bool b = writeCmd(hard ? SHT31_HARD_RESET : SHT31_SOFT_RESET);
  if (b == false)
  {
    return false;
  }
  sleep_ms(1);   //  table 4 datasheet
  return true;
}


void SHT31::setHeatTimeout(uint8_t seconds)
{
  _heatTimeout = seconds;
  if (_heatTimeout > 180) _heatTimeout = 180;
}


/////////////////////////////////////////////////////////////////
//
//  HEATER
//
bool SHT31::heatOn()
{
  if (isHeaterOn()) return true;
  if ((_heaterStop > 0) && (to_ms_since_boot(get_absolute_time()) - _heaterStop < SHT31_HEATER_TIMEOUT))
  {
    _error = SHT31_ERR_HEATER_COOLDOWN;
    return false;
  }
  if (writeCmd(SHT31_HEAT_ON) == false)
  {
    _error = SHT31_ERR_HEATER_ON;
    return false;
  }
  _heaterStart = to_ms_since_boot(get_absolute_time());
  _heaterOn    = true;
  return true;
}


bool SHT31::heatOff()
{
  //  always switch off the heater - ignore _heaterOn flag.
  if (writeCmd(SHT31_HEAT_OFF) == false)
  {
    _error = SHT31_ERR_HEATER_OFF;  // can be serious!
    return false;
  }
  _heaterStop = to_ms_since_boot(get_absolute_time());
  _heaterOn   = false;
  return true;
}


bool SHT31::isHeaterOn()
{
  if (_heaterOn == false)
  {
    return false;
  }
  //  did not exceed time out
  if (to_ms_since_boot(get_absolute_time()) - _heaterStart < (_heatTimeout * 1000UL))
  {
    return true;
  }
  heatOff();
  return false;
}


/////////////////////////////////////////////////////////////////
//
//  ASYNCHRONUOUS INTERFACE
//
bool SHT31::requestData()
{
  if (writeCmd(SHT31_MEASUREMENT_SLOW) == false)
  {
    return false;
  }
  _lastRequest = to_ms_since_boot(get_absolute_time());
  return true;
}


bool SHT31::dataReady()
{
  return ((to_ms_since_boot(get_absolute_time()) - _lastRequest) > 15);  //  TODO MAGIC NR
}


bool SHT31::readData(bool fast)
{
  uint8_t buffer[6];
  if (readBytes(6, (uint8_t*) &buffer[0]) == false)
  {
    return false;
  }

  if (!fast)
  {
    if (buffer[2] != crc8(buffer, 2))
    {
      _error = SHT31_ERR_CRC_TEMP;
      return false;
    }
    if (buffer[5] != crc8(buffer + 3, 2))
    {
      _error = SHT31_ERR_CRC_HUM;
      return false;
    }
  }

  _rawTemperature = (buffer[0] << 8) + buffer[1];
  _rawHumidity    = (buffer[3] << 8) + buffer[4];

  _lastRead = to_ms_since_boot(get_absolute_time());
  return true;
}


/////////////////////////////////////////////////////////////////
//
//  MISC
//
int SHT31::getError()
{
  int rv = _error;
  _error = SHT31_OK;
  return rv;
}


/**
 * See https://sensirion.com/media/documents/E5762713/63D103C2/Sensirion_electronic_identification_code_SHT3x.pdf
 */
bool SHT31::getSerialNumber(uint32_t &serial, bool fast) {
  if (writeCmd(SHT31_GET_SERIAL_NUMBER) == false) {
      return false;
  }
  sleep_ms(1);
  uint8_t buffer[6];
  if (readBytes(6, &buffer[0]) == false) {
    return false;
  }

  if (!fast) {
      if (buffer[2] != crc8(buffer, 2)) {
      _error = SHT31_ERR_SERIAL_NUMBER_CRC;
      return false;
      }
      if (buffer[5] != crc8(buffer + 3, 2)) {
      _error = SHT31_ERR_SERIAL_NUMBER_CRC;
      return false;
      }
  }
  serial = buffer[0];
  serial <<= 8;
  serial += buffer[1];
  serial <<= 8;
  serial += buffer[3];
  serial <<= 8;
  serial += buffer[4];
  return true;
}


/////////////////////////////////////////////////////////////////
//
//  PROTECTED
//
uint8_t SHT31::crc8(const uint8_t *data, uint8_t len)
{
  //  CRC-8 formula from page 14 of SHT spec pdf
  const uint8_t POLY(0x31);
  uint8_t crc(0xFF);

  for (uint8_t j = len; j; --j)
  {
    crc ^= *data++;

    for (uint8_t i = 8; i; --i)
    {
      crc = (crc & 0x80) ? (crc << 1) ^ POLY : (crc << 1);
    }
  }
  return crc;
}


bool SHT31::writeCmd(uint16_t cmd)
{
  uint8_t buf[2];
  buf[0] = cmd >> 8;
  buf[1] = cmd & 0xFF;

  int ret = i2c_write_blocking(_i2c, _address, buf, 2, false);
  if (ret != 2)
  {
    _error = SHT31_ERR_WRITECMD;
    return false;
  }
  _error = SHT31_OK;
  return true;
}


bool SHT31::readBytes(uint8_t n, uint8_t *val)
{
  int ret = i2c_read_blocking(_i2c, _address, val, n, false);
  if (ret != (int)n)
  {
    _error = SHT31_ERR_READBYTES;
    return false;
  }
  _error = SHT31_OK;
  return true;
}


//  -- END OF FILE --
