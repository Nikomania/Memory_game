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

#define RGB_BLACK {0, 0, 0}
#define RGB_WHITE {16, 16, 16}
#define RGB_RED {16, 0, 0}
#define RGB_GREEN {0, 16, 0}
#define RGB_BLUE {0, 0, 16}
#define RGB_YELLOW {16, 16, 0}
#define RGB_CYAN {0, 16, 16}
#define RGB_MAGENTA {16, 0, 16}

class LedMatrix {
public:
  static LedMatrix& getInstance();
  void setLED(const uint index_x, const uint index_y, const rgb_t led);
  void clear();
  void render();
  
  void setSmile(struct rgb_t color);
  void setNumberOne(struct rgb_t color);
  void setNumberTwo(struct rgb_t color);
  void setNumberThree(struct rgb_t color);
  void setNumberFour(struct rgb_t color);
  void setNumberFive(struct rgb_t color);
  void setNumberSix(struct rgb_t color);
  void setNumberSeven(struct rgb_t color);
  void setNumberEight(struct rgb_t color);
  void setNumberNine(struct rgb_t color);
  void setNumberZero(struct rgb_t color);
private:
  LedMatrix();
  rgb_t led_matrix[LED_COUNT_X][LED_COUNT_Y];
  PIO led_matrix_pio;
  uint sm;

  static LedMatrix* instance;
};

#endif // LED_MATRIX_H
