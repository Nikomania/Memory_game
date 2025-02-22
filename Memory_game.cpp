#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "LedMatrix.h"

#define UPDATE_STATE_DELAY 250
#define BTN_DEBOUNCE 100
#define ADC_BITS 12
#define ADC_MAX (1 << ADC_BITS) - 1 // 12-bit ADC
#define JST_THRESHOLD 300
#define MAX_FRAMES 9
#define MIN_FRAMES 1

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
direction_t get_joystick_direction(int pin);

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
uint8_t frames_to_remember = MIN_FRAMES;
uint8_t current_index_x = 0, current_index_y = 0, current_frame = 0;

COLORS frames[MAX_FRAMES][LED_COUNT_X][LED_COUNT_Y];
COLORS player_frames[MAX_FRAMES][LED_COUNT_X][LED_COUNT_Y];

void init_state();
void setting_state();
void frame_state();
void remember_state();
void game_state();
bool update_state();

void navigate_leds();
void set_frames(COLORS current_frames[MAX_FRAMES][LED_COUNT_X][LED_COUNT_Y]);

int main() {
  stdio_init_all();
  gpio_init_all();

  LedMatrix* led_matrix = &LedMatrix::getInstance();

  // struct repeating_timer timer;
  // add_repeating_timer_ms(UPDATE_STATE_DELAY, update_state_callback, NULL, &timer);
  while (true) {
    update_state();
    led_matrix->render();
    sleep_ms(UPDATE_STATE_DELAY);
  }

  delete led_matrix;
  return 0;
}

void init_state() {
  printf("Init state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();
  led_matrix->setSmile(MAGENTA);

  if (sw_clicked) {
    current_state = SETTING_STATE;
  }
}

void setting_state() {
  printf("Setting state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();

  if (sw_clicked) {
    current_state = FRAME_STATE;
    for (uint8_t i = 0; i < frames_to_remember; i++) {
      LedMatrix::clear(frames[i]);
    }
    current_index_x = 0;
    current_index_y = 0;
    current_frame = 0;
    return;
  }

  if (frames_to_remember > MIN_FRAMES) {
    if (btn_A_clicked || (jst_x_changed && jst_X_state.last_direction == NEG)) {
      frames_to_remember--;
    }
  }

  if (frames_to_remember < MAX_FRAMES) {
    if (btn_B_clicked || (jst_x_changed && jst_X_state.last_direction == POS)) {
      frames_to_remember++;
    }
  }
  printf("Frames to remember: %d\n", frames_to_remember);

  led_matrix->setNumber(frames_to_remember, BLUE);
}

void frame_state() {
  printf("Frame state\n");
  if (sw_clicked) {
    current_state = REMEMBER_STATE;
    return;
  }

  set_frames(frames);
}

void remember_state() {
  printf("Remember state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();

  if (sw_clicked) {
    current_state = GAME_STATE;
    for (uint8_t i = 0; i < frames_to_remember; i++) {
      LedMatrix::clear(player_frames[i]);
    }
    current_index_x = 0;
    current_index_y = 0;
    current_frame = 0;
    return;
  }

  if (current_frame > 0) {
    if (btn_A_clicked || (jst_x_changed && jst_X_state.last_direction == NEG)) {
      current_frame--;
    }
  }

  if (current_frame + 1 < frames_to_remember) {
    if (btn_B_clicked || (jst_x_changed && jst_X_state.last_direction == POS)) {
      current_frame++;
    }
  }

  led_matrix->setLEDs(frames[current_frame]);
}

void game_state() {
  printf("Game state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();

  if (sw_clicked) {
    current_state = FINAL_STATE;
    return;
  }

  set_frames(player_frames);
}

bool update_state() {
  input_manager();

  switch (current_state) {
  case INIT_STATE:
    init_state();
    break;
  case SETTING_STATE:
    setting_state();
    break;
  case FRAME_STATE:
    frame_state();
    break;
  case REMEMBER_STATE:
    remember_state();
    break;
  case GAME_STATE:
    game_state();
    break;
  case FINAL_STATE:
    printf("Final state\n");
    break;
  default:
    break;
  }

  return true;
}

void set_frames(COLORS current_frames[MAX_FRAMES][LED_COUNT_X][LED_COUNT_Y]) {
  navigate_leds();

  int current_color = current_frames[current_frame][current_index_x][current_index_y];
  if (btn_A_clicked) {
    if (current_color > 0) {
      current_color--;
    } else {
      current_color = COLORS_COUNT - 1;
    }
  }

  if (btn_B_clicked) {
    current_color = (current_color + 1) % COLORS_COUNT;
  }
  current_frames[current_frame][current_index_x][current_index_y] = (COLORS)current_color;

  static bool blink = false;

  LedMatrix* led_matrix = &LedMatrix::getInstance();
  led_matrix->setLEDs(current_frames[current_frame]);
  if (blink) {
    // current_frames[current_state][current_index_x][current_index_y] == (rgb_t)RGB_BLACK
    if (
      current_color == BLACK
    ) {
      current_color = WHITE;
    }
  } else {
    current_color = BLACK;
  }
  led_matrix->setLED(
    current_index_x,
    current_index_y,
    (COLORS)current_color
  );

  blink = !blink;
}

void navigate_leds() {
  if (jst_x_changed) {
    if (jst_X_state.last_direction == NEG) {
      if (current_index_x > 0) {
        current_index_x--;
      } else if (current_frame > 0) {
        current_index_x = LED_COUNT_X - 1;
        current_frame--;
      }
    } else { // jst_x_changed == POS, because jst_x_changed is false when NEUTRAL
      if (current_index_x + 1 < LED_COUNT_X) {
        current_index_x++;
      } else if (current_frame + 1 < frames_to_remember) {
        current_index_x = 0;
        current_frame++;
      }
    }
  }

  if (jst_y_changed) {
    if (jst_Y_state.last_direction == NEG) {
      if (current_index_y > 0) {
        current_index_y--;
      } else {
        current_index_y = LED_COUNT_Y - 1;
      }
    } else { // jst_y_changed == POS, because jst_y_changed is false when NEUTRAL
      if (current_index_y + 1 < LED_COUNT_Y) {
        current_index_y++;
      } else {
        current_index_y = 0;
      }
    }
  }
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
