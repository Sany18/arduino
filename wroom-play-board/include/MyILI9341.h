#pragma once
#include <Adafruit_ILI9341.h>
#include <stdint.h>

#define MADCTL_MX  0x40
#define MADCTL_MY  0x80
#define MADCTL_MV  0x20
#define MADCTL_BGR 0x08
#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320
#define ILI9341_MADCTL 0x36
#define ILI9341_CASET  0x2A
#define ILI9341_PASET  0x2B
#define ILI9341_RAMWR  0x2C

extern void SPI_WRITE16(uint16_t d);
extern void writeCommand(uint8_t cmd);
extern void sendCommand(uint8_t cmd, const uint8_t *data, uint8_t num);

class MyILI9341 : public Adafruit_ILI9341 {
public:
  using Adafruit_ILI9341::Adafruit_ILI9341;
  void setRotation(uint8_t m) override {
    rotation = m % 4;
    uint8_t madctl = 0;
    switch (rotation) {
      case 0:
        madctl = (MADCTL_MV | MADCTL_MY | MADCTL_BGR);
        _width  = ILI9341_TFTHEIGHT;
        _height = ILI9341_TFTWIDTH;
        break;
      case 1:
        madctl = (MADCTL_MX | MADCTL_MY | MADCTL_BGR);
        _width  = ILI9341_TFTWIDTH;
        _height = ILI9341_TFTHEIGHT;
        break;
      case 2:
        madctl = (MADCTL_MV | MADCTL_MX | MADCTL_BGR);
        _width  = ILI9341_TFTHEIGHT;
        _height = ILI9341_TFTWIDTH;
        break;
      case 3:
        madctl = (MADCTL_BGR);
        _width  = ILI9341_TFTWIDTH;
        _height = ILI9341_TFTHEIGHT;
        break;
    }
    sendCommand(ILI9341_MADCTL, &madctl, 1);
  }
  
  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) override {
    // x=0,y=0 is top-left, x increases downward, y increases rightward
    const uint16_t colstart = 0;
    const uint16_t rowstart = 0;
    uint16_t x0 = x + colstart;
    uint16_t x1 = x + w - 1 + colstart;
    uint16_t y0 = y + rowstart;
    uint16_t y1 = y + h - 1 + rowstart;
    writeCommand(ILI9341_CASET); // Column addr set
    SPI_WRITE16(y0);
    SPI_WRITE16(y1);
    writeCommand(ILI9341_PASET); // Page addr set
    SPI_WRITE16(x0);
    SPI_WRITE16(x1);
    writeCommand(ILI9341_RAMWR); // Write to RAM
  }
};
