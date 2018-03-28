#ifndef DISPLAYELEMENTS_HPP
#define DISPLAYELEMENTS_HPP

#include "./Display.hpp"
#include "../MIDI_Inputs/MIDI_Input_V-Pot_Ring.h"
#include "../MIDI_Inputs/MIDI_Input_VU.h"
#include "../MIDI_Inputs/MIDI_Input_LED.h"

#undef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#undef min
#define min(a, b) ((a) < (b) ? (a) : (b))

class VPotDisplay : public DisplayElement
{

public:
  VPotDisplay(Adafruit_GFX &display, MCU_VPot_Ring &vpot,
              int16_t x, int16_t y,
              uint16_t radius, uint16_t innerRadius, uint16_t color)
      : DisplayElement(display), vpot(vpot),
        x(x + radius), y(y + radius),
        radius(radius), innerRadius(innerRadius), color(color) {}
  void draw()
  {
    drawCircle(display, x, y, radius, color);
    if (vpot.getCenterLED())
      fillCircle(display, x, y, innerRadius / 4, color);
    else
      drawCircle(display, x, y, innerRadius / 4, color);

    uint8_t value = vpot.getPosition();
    if (value == 0)
      return;
    value--;
    switch (vpot.getMode())
    {
    case 0:
    {
      drawVPotSegment(value);
    }
    break;
    case 1:
    {
      uint8_t startOn = min(value, 5);
      uint8_t startOff = max(value, 5) + 1;
      for (uint8_t pin = startOn; pin < startOff; pin++)
        drawVPotSegment(pin);
    }
    break;
    case 2:
    {
      for (uint8_t pin = 0; pin <= value; pin++)
        drawVPotSegment(pin);
    }
    break;
    case 3:
    {
      uint8_t startOn = max(5 - value, 0);
      uint8_t startOff = min(6 + value, 11);
      for (uint8_t pin = startOn; pin < startOff; pin++)
        drawVPotSegment(pin);
    }
    break;
    }
  }

private:
  MCU_VPot_Ring &vpot;

  int16_t x, y;
  uint16_t radius, innerRadius, color;

  const static float angleSpacing;

protected:
  void drawVPotSegment(uint8_t segment)
  {
    // segment 5 (i.e. the sixth segment) = 0° (i.e. 12 o'clock)
    float angle = angleSpacing * (segment - 5);

    uint16_t x_start = x + round((float)innerRadius * sin(angle) / 2.0);
    uint16_t y_start = y - round((float)innerRadius * cos(angle) / 2.0);

    uint16_t x_end = x + round((float)innerRadius * sin(angle));
    uint16_t y_end = y - round((float)innerRadius * cos(angle));

    display.drawLine(x_start, y_start, x_end, y_end, color);
  }
};

const float VPotDisplay::angleSpacing = 0.4887; // 28°

// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- //

class VUDisplay : public DisplayElement
{
public:
  VUDisplay(Adafruit_GFX &display, MCU_VU &vu,
            int16_t x, int16_t y, uint16_t width,
            uint8_t blockheight, uint8_t spacing,
            uint16_t color)
      : DisplayElement(display), vu(vu),
        x(x), y(y), width(width),
        blockheight(blockheight), spacing(spacing),
        color(color) {}
  void draw()
  {
    uint8_t value = vu.getValue();

    if (value >= peak)
    {
      peak = value;
      previousDecay = millis();
    }
    else if (millis() - previousDecay > decayTime)
    {
      if (peak > 0)
      {
        peak--;
        previousDecay = millis();
      }
    }
    if (peak > 0)
    {
      display
          .drawFastHLine(x,
                         y - spacing + blockheight - peak * (blockheight + spacing),
                         width,
                         color);
      for (uint8_t i = 0; i < value; i++)
        display.fillRect(x,
                         y - i * (blockheight + spacing),
                         width,
                         blockheight,
                         color);
    }
  }

private:
  MCU_VU &vu;

  int16_t x, y;
  uint16_t width;
  uint8_t blockheight, spacing;
  uint16_t color;

  unsigned long peak = 0;
  unsigned long previousDecay = 0;

  const static unsigned long decayTime;
};

const unsigned long VUDisplay::decayTime = 350;

// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- //

struct XBitmap
{
  XBitmap(uint16_t width, uint16_t height, const uint8_t bits[])
      : width(width), height(height), bits(bits) {}
  uint16_t width;
  uint16_t height;
  const uint8_t *bits;
};

class NoteDisplay : public DisplayElement
{
public:
  NoteDisplay(Adafruit_GFX &display, MIDI_Input_Note_Buffer &notebuffer,
              const XBitmap &xbm, int16_t x, int16_t y,
              uint16_t color)
      : DisplayElement(display), notebuffer(notebuffer),
        xbm(xbm), x(x), y(y), color(color) {}

  void draw()
  {
    if (notebuffer.getState())
      display.drawXBitmap(x, y, xbm.bits, xbm.width, xbm.height, color);
  }

private:
  MIDI_Input_Note_Buffer &notebuffer;
  const XBitmap &xbm;
  int16_t x, y;
  uint16_t color;
};

// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- //

class SelectorDisplay : public DisplayElement
{
public:
  SelectorDisplay(Adafruit_GFX &display, Selector &selector,
                  int16_t offset, int16_t multiplier,
                  int16_t x, int16_t y,
                  uint8_t size, uint16_t color)
      : DisplayElement(display), selector(selector),
        offset(offset), multiplier(multiplier),
        x(x), y(y),
        size(size), color(color) {}
  void draw()
  {
    display.setTextSize(size);
    display.setCursor(x, y);
    display.print(selector.getSetting() * multiplier + offset);
  }

private:
  Selector &selector;
  int16_t offset, multiplier, x, y;
  uint8_t size;
  uint16_t color;
};

// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- //

class TimeDisplay : public DisplayElement
{
public:
  TimeDisplay(Adafruit_GFX &display, MCU_TimeDisplay &tdisp,
              int16_t x, int16_t y,
              uint8_t size, uint16_t color)
      : DisplayElement(display), tdisp(tdisp),
        x(x), y(y),
        size(size), color(color) {}
  void draw()
  {
    display.setTextSize(size);
    display.setCursor(x, y);
    char barStr[5], beatStr[3], frameStr[4]; // TODO: use tdisp pointers directly
    tdisp.getBars(barStr);
    tdisp.getBeats(beatStr);
    tdisp.getFrames(frameStr);
    display.print(barStr);
    display.print(' ');
    display.print(beatStr);
    display.print(' ');
    display.print(frameStr);
  }

private:
  MCU_TimeDisplay &tdisp;
  int16_t x, y;
  uint8_t size;
  uint16_t color;
};

#endif // DISPLAYELEMENTS_HPP