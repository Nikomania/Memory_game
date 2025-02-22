#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "LedMatrix.h"

#define INPUT_DELAY 100
#define BTN_DEBOUNCE 50
#define ADC_BITS 12
#define ADC_MAX (1 << ADC_BITS) - 1 // 12-bit ADC
#define JST_THRESHOLD 300

#define abs(x) ((x) < 0 ? -(x) : (x))


// GPIO
#define BTN_A_PIN 5
#define BTN_B_PIN 6
#define SW_PIN 22
#define JST_X_PIN 27
#define JST_Y_PIN 26
void gpio_init_all();
// ----------------------------
// INPUT MANAGER
struct button_state_t {
  absolute_time_t last_press_time;
  bool was_pressed;
  int pin;
};
enum direction_t {
  POS,
  NEG,
  NEUTRAL
};
struct joystick_state_t {
  absolute_time_t last_press_time;
  direction_t last_direction;
  int pin;
};

void input_manager();
bool check_button_state(volatile struct button_state_t *btn_state);
bool check_joystick_state(volatile struct joystick_state_t *jst_state);
direction_t get_joystick_direction();

volatile struct button_state_t btn_A_state = {0, false, BTN_A_PIN};
volatile struct button_state_t btn_B_state = {0, false, BTN_B_PIN};
volatile struct button_state_t sw_state = {0, false, SW_PIN};
volatile struct joystick_state_t jst_X_state = {0, NEUTRAL, JST_X_PIN};
volatile struct joystick_state_t jst_Y_state = {0, NEUTRAL, JST_Y_PIN};

bool btn_A_clicked = false, btn_B_clicked = false, sw_clicked = false;
bool jst_x_changed = false, jst_y_changed = false;
// ----------------------------

enum state_t {
  INIT_STATE,
  SETTING_STATE,
  FRAME_STATE,
  REMEMBER_STATE,
  GAME_STATE,
  FINAL_STATE
};
state_t current_state = INIT_STATE;

void init_state();
void setting_state();
bool update_state_callback(struct repeating_timer *t);

int main() {
  stdio_init_all();
  gpio_init_all();

  LedMatrix* led_matrix = &LedMatrix::getInstance();

  struct repeating_timer timer;
  add_repeating_timer_ms(INPUT_DELAY, update_state_callback, NULL, &timer);
  while (true) {
    tight_loop_contents();
  }

  delete led_matrix;
  return 0;
}

void init_state() {
  printf("Init state\n");
  sleep_ms(1000);

  current_state = SETTING_STATE;
}

void setting_state() {
  printf("Setting state\n");
  sleep_ms(1000);
  current_state = FRAME_STATE;
}

bool update_state_callback(struct repeating_timer *t) {
  input_manager();

  switch (current_state) {
  case INIT_STATE:
    init_state();
    break;
  case SETTING_STATE:
    setting_state();
    break;
  default:
    break;
  }

  return true;
}

void input_manager() {
  btn_A_clicked = check_button_state(&btn_A_state);
  btn_B_clicked = check_button_state(&btn_B_state);
  sw_clicked = check_button_state(&sw_state);

  jst_x_changed = check_joystick_state(&jst_X_state);
  jst_y_changed = check_joystick_state(&jst_Y_state);
}

bool check_button_state(volatile struct button_state_t *btn_state) {
  bool button_pressed = !gpio_get(btn_state->pin);

  if (
    button_pressed &&
    !btn_state->was_pressed &&
    absolute_time_diff_us(btn_state->last_press_time, get_absolute_time()) > BTN_DEBOUNCE * 1000
  ) {
    btn_state->last_press_time = get_absolute_time();
    btn_state->was_pressed = true;
    return true;
  } else if (!button_pressed) {
    btn_state->was_pressed = false;
  }

  return false;
}

bool check_joystick_state(volatile struct joystick_state_t *jst_state) {
  direction_t direction = get_joystick_direction(jst_state->pin);
  bool result = false;

  if (
    direction != jst_state->last_direction &&
    direction != NEUTRAL &&
    absolute_time_diff_us(jst_state->last_press_time, get_absolute_time()) > BTN_DEBOUNCE * 1000
  ) {
    jst_state->last_press_time = get_absolute_time();
    result = true;
  }

  jst_state->last_direction = direction;

  return result;
}

direction_t get_joystick_direction(int pin) {
  switch (pin) {
  case JST_X_PIN:
    adc_select_input(1);
    break;
  case JST_Y_PIN:
    adc_select_input(0);
    break;
  default:
    return NEUTRAL;
    break;
  }
  uint16_t jst_adc_raw = adc_read();

  if (jst_adc_raw < JST_THRESHOLD) {
    return NEG;
  } else if (jst_adc_raw > ADC_MAX - JST_THRESHOLD) {
    return POS;
  } else {
    return NEUTRAL;
  }
}

void gpio_init_all() {
  gpio_init(BTN_A_PIN);
  gpio_set_dir(BTN_A_PIN, GPIO_IN);
  gpio_pull_up(BTN_A_PIN);

  gpio_init(BTN_B_PIN);
  gpio_set_dir(BTN_B_PIN, GPIO_IN);
  gpio_pull_up(BTN_B_PIN);

  gpio_init(SW_PIN);
  gpio_set_dir(SW_PIN, GPIO_IN);
  gpio_pull_up(SW_PIN);

  adc_init();
  // Make sure GPIO is high-impedance, no pullups etc
  adc_gpio_init(JST_Y_PIN);
  adc_gpio_init(JST_X_PIN);
}
