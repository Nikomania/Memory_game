#include "LedMatrix.h"

LedMatrix* LedMatrix::instance = nullptr;

LedMatrix& LedMatrix::getInstance() {
  if (instance == nullptr) {
    new LedMatrix();
  }
  return *instance;
}

LedMatrix::LedMatrix() : led_matrix(), led_matrix_pio(), sm() {
  instance = this;
  uint offset = pio_add_program(pio0, &ws2818b_program);
  led_matrix_pio = pio0;

  sm = pio_claim_unused_sm(led_matrix_pio, false);
  if (sm < 0) {  // sm is an unsigned int, therefore it can't be negative (how does this work?)
    led_matrix_pio = pio1;
    sm = pio_claim_unused_sm(led_matrix_pio, true);
  }

  ws2818b_program_init(led_matrix_pio, sm, offset, LED_MATRIX_PIN, 800000.f);

  clear();
}

void LedMatrix::setLED(const uint index_x, const uint index_y, const rgb_t led) {
  if (index_x >= LED_COUNT_X || index_y >= LED_COUNT_Y) {
    return;
  }

  led_matrix[index_x][index_y] = {led.R, led.G, led.B};
}

void LedMatrix::clear() {
  for (uint8_t i = 0; i < LED_COUNT_X; i++)
    for (uint8_t j = 0; j < LED_COUNT_Y; j++)
      setLED(i, j, RGB_BLACK);
}

void LedMatrix::render() {
  for (uint8_t j = 0; j < LED_COUNT_Y; j++) {
    for (int8_t i = 0; i < LED_COUNT_X; i++) {
      const uint8_t index_x = j % 2 == 0 ? LED_COUNT_X - 1 - i : i;
      pio_sm_put_blocking(led_matrix_pio, sm, led_matrix[index_x][j].G);
      pio_sm_put_blocking(led_matrix_pio, sm, led_matrix[index_x][j].R);
      pio_sm_put_blocking(led_matrix_pio, sm, led_matrix[index_x][j].B);
    }
  }
  sleep_us(100); // wait for 100us, RESET signal from datasheet
}

// setLED(0, 4, color); setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color); setLED(4, 4, color);
// setLED(0, 3, color); setLED(1, 3, color); setLED(2, 3, color); setLED(3, 3, color); setLED(4, 3, color);
// setLED(0, 2, color); setLED(1, 2, color); setLED(2, 2, color); setLED(3, 2, color); setLED(4, 2, color);
// setLED(0, 1, color); setLED(1, 1, color); setLED(2, 1, color); setLED(3, 1, color); setLED(4, 1, color);
// setLED(0, 0, color); setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color); setLED(4, 0, color);


void LedMatrix::setSmile(struct rgb_t color) {
    clear();
    setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
    setLED(1, 3, color);                      setLED(3, 3, color);
    setLED(1, 2, color);                      setLED(3, 2, color);
    setLED(1, 1, color);                      setLED(3, 1, color);
    setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
}

void LedMatrix::setNumberZero(struct rgb_t color) {
    clear();
    setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
    setLED(1, 3, color);                      setLED(3, 3, color);
    setLED(1, 2, color);                      setLED(3, 2, color);
    setLED(1, 1, color);                      setLED(3, 1, color);
    setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
}

void LedMatrix::setNumberOne(struct rgb_t color) {
  clear();
                       setLED(2, 4, color);
  setLED(1, 3, color); setLED(2, 3, color);
                       setLED(2, 2, color);
                       setLED(2, 1, color);
  setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
  render();
}

void LedMatrix::setNumberTwo(struct rgb_t color) {
  clear();
  setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
                                            setLED(3, 3, color);
  setLED(1, 2, color); setLED(2, 2, color); setLED(3, 2, color);
  setLED(1, 1, color);
  setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
  render();
}

void LedMatrix::setNumberThree(struct rgb_t color) {
    clear();
    setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
                                              setLED(3, 3, color);
    setLED(1, 2, color); setLED(2, 2, color); setLED(3, 2, color);
                                              setLED(3, 1, color);
    setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
}

void LedMatrix::setNumberFour(struct rgb_t color) {
    clear();
    setLED(1, 4, color);                      setLED(3, 4, color);
    setLED(1, 3, color);                      setLED(3, 3, color);
    setLED(1, 2, color); setLED(2, 2, color); setLED(3, 2, color);
                                              setLED(3, 1, color);
                                              setLED(3, 0, color);
}

void LedMatrix::setNumberFive(struct rgb_t color) {
    clear();
    setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
    setLED(1, 3, color);
    setLED(1, 2, color); setLED(2, 2, color); setLED(3, 2, color);
                                              setLED(3, 1, color);
    setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
}

void LedMatrix::setNumberSix(struct rgb_t color) {
    clear();
    setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
    setLED(1, 3, color);
    setLED(1, 2, color); setLED(2, 2, color); setLED(3, 2, color);
    setLED(1, 1, color);                      setLED(3, 1, color);
    setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
}

void LedMatrix::setNumberSeven(struct rgb_t color) {
    clear();
    setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
                                              setLED(3, 3, color);
                                              setLED(3, 2, color);
                                              setLED(3, 1, color);
                                              setLED(3, 0, color);
}

void LedMatrix::setNumberEight(struct rgb_t color) {
    clear();
    setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
    setLED(1, 3, color);                      setLED(3, 3, color);
    setLED(1, 2, color); setLED(2, 2, color); setLED(3, 2, color);
    setLED(1, 1, color);                      setLED(3, 1, color);
    setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
}

void LedMatrix::setNumberNine(struct rgb_t color) {
    clear();
    setLED(1, 4, color); setLED(2, 4, color); setLED(3, 4, color);
    setLED(1, 3, color);                      setLED(3, 3, color);
    setLED(1, 2, color); setLED(2, 2, color); setLED(3, 2, color);
                                              setLED(3, 1, color);
    setLED(1, 0, color); setLED(2, 0, color); setLED(3, 0, color);
}
