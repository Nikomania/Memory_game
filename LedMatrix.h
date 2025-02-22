#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "ws2818b.pio.h"

#define LED_COUNT_X 5
#define LED_COUNT_Y 5
#define LED_COUNT (LED_X_COUNT * LED_Y_COUNT)
#define LED_MATRIX_PIN 7

struct rgb_t {
  uint8_t R, G, B;
};

enum COLORS {
  BLACK,
  WHITE,
  RED,
  GREEN,
  BLUE,
  YELLOW,
  CYAN,
  MAGENTA,
  COLORS_COUNT
};

const rgb_t COLORS_ARRAY[COLORS_COUNT] = {
  {0, 0, 0},
  {16, 16, 16},
  {16, 0, 0},
  {0, 16, 0},
  {0, 0, 16},
  {16, 16, 0},
  {0, 16, 16},
  {16, 0, 16}
};

class LedMatrix {
public:
  static LedMatrix& getInstance();
  void setLED(const uint index_x, const uint index_y, const COLORS color);
  void setLEDs(const COLORS leds[LED_COUNT_X][LED_COUNT_Y]);
  void render();
  void clear();
  static void clear(COLORS leds[LED_COUNT_X][LED_COUNT_Y]);
  
  void setSmile(COLORS color);
  void setCheck(COLORS color);
  void setCross(COLORS color);

  void setNumber(const uint8_t number, COLORS color);
  void setNumberOne(COLORS color);
  void setNumberTwo(COLORS color);
  void setNumberThree(COLORS color);
  void setNumberFour(COLORS color);
  void setNumberFive(COLORS color);
  void setNumberSix(COLORS color);
  void setNumberSeven(COLORS color);
  void setNumberEight(COLORS color);
  void setNumberNine(COLORS color);
  void setNumberZero(COLORS color);
private:
  LedMatrix();
  COLORS led_matrix[LED_COUNT_X][LED_COUNT_Y];
  PIO led_matrix_pio;
  uint sm;

  bool was_changed;

  static LedMatrix* instance;
};

#endif // LED_MATRIX_H
