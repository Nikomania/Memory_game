#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "GPIO.h"
#include "InputManager.h"
#include "LedMatrix.h"

#define UPDATE_STATE_DELAY 250
#define MAX_FRAMES 9
#define MIN_FRAMES 1

#define abs(x) ((x) < 0 ? -(x) : (x))

enum state_t {
  INIT_STATE,
  SETTING_STATE,
  FRAME_STATE,
  REMEMBER_STATE,
  GAME_STATE,
  FINAL_STATE
};
enum COMPARE_STATE {
    CORRECT,
    PLAYER,
    COMPARE,
    COMPARE_STATE_COUNT
  };
state_t current_state = INIT_STATE;
uint8_t frames_to_remember = MIN_FRAMES;
uint8_t current_index_x = 0, current_index_y = 0, current_frame = 0;
COMPARE_STATE show_frames_comp = CORRECT;

COLORS frames[MAX_FRAMES][LED_COUNT_X][LED_COUNT_Y];
COLORS player_frames[MAX_FRAMES][LED_COUNT_X][LED_COUNT_Y];

void init_state();
void setting_state();
void frame_state();
void remember_state();
void game_state();
void final_state();
bool update_state();

bool compare_frames(
  COLORS frame1[LED_COUNT_X][LED_COUNT_Y],
  COLORS frame2[LED_COUNT_X][LED_COUNT_Y]
);
void navigate_leds();
void switch_frames();
void set_frames(COLORS current_frames[MAX_FRAMES][LED_COUNT_X][LED_COUNT_Y]);

int main() {
  stdio_init_all();
  gpio_init_all();

  LedMatrix* led_matrix = &LedMatrix::getInstance();
  InputManager* input_manager = &InputManager::getInstance();

  // struct repeating_timer timer;
  // add_repeating_timer_ms(UPDATE_STATE_DELAY, update_state_callback, NULL, &timer);
  while (true) {
    input_manager->update();
    update_state();
    led_matrix->render();
    sleep_ms(UPDATE_STATE_DELAY);
  }

  delete led_matrix;
  delete input_manager;
  return 0;
}

void init_state() {
  printf("Init state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();
  led_matrix->setSmile(MAGENTA);

  InputManager& input_manager = InputManager::getInstance();

  if (input_manager.isButtonClicked(SW_PIN)) {
    current_state = SETTING_STATE;
    frames_to_remember = MIN_FRAMES;
  }
}

void setting_state() {
  printf("Setting state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();
  InputManager& input_manager = InputManager::getInstance();

  if (input_manager.isButtonClicked(SW_PIN)) {
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
    if (
      input_manager.isButtonClicked(BTN_A_PIN) || (
        input_manager.isJoystickXChanged() &&
        input_manager.getJoystickXDirection() == NEG
      )
    ) {
      frames_to_remember--;
    }
  }

  if (frames_to_remember < MAX_FRAMES) {
    if (
      input_manager.isButtonClicked(BTN_B_PIN) || (
        input_manager.isJoystickXChanged() &&
        input_manager.getJoystickXDirection() == POS
      )
    ) {
      frames_to_remember++;
    }
  }
  printf("Frames to remember: %d\n", frames_to_remember);

  led_matrix->setNumber(frames_to_remember, BLUE);
}

void frame_state() {
  printf("Frame state\n");
  InputManager& input_manager = InputManager::getInstance();

  if (input_manager.isButtonClicked(SW_PIN)) {
    current_state = REMEMBER_STATE;
    current_frame = 0;
    return;
  }

  set_frames(frames);
}

void remember_state() {
  printf("Remember state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();
  InputManager& input_manager = InputManager::getInstance();

  if (input_manager.isButtonClicked(SW_PIN)) {
    current_state = GAME_STATE;
    for (uint8_t i = 0; i < frames_to_remember; i++) {
      LedMatrix::clear(player_frames[i]);
    }
    current_index_x = 0;
    current_index_y = 0;
    current_frame = 0;
    return;
  }

  switch_frames();

  led_matrix->setLEDs(frames[current_frame]);
}

void game_state() {
  printf("Game state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();
  InputManager& input_manager = InputManager::getInstance();

  if (input_manager.isButtonClicked(SW_PIN)) {
    current_state = FINAL_STATE;
    show_frames_comp = CORRECT;
    current_frame = 0;
    return;
  }

  set_frames(player_frames);
}

void final_state() {
  printf("Final state\n");
  LedMatrix* led_matrix = &LedMatrix::getInstance();
  InputManager& input_manager = InputManager::getInstance();

  if (input_manager.isButtonClicked(SW_PIN)) {
    current_state = INIT_STATE;
  }

  switch_frames();

  switch (show_frames_comp) {
  case CORRECT:
    led_matrix->setLEDs(frames[current_frame]);
    break;
  case PLAYER:
    led_matrix->setLEDs(player_frames[current_frame]);
    break;
  case COMPARE:
    if (compare_frames(frames[current_frame], player_frames[current_frame])) {
      led_matrix->setCheck(GREEN);
    } else {
      led_matrix->setCross(RED);
    }
  default:
    break;
  }

  if (show_frames_comp == COMPARE) {
    show_frames_comp = CORRECT;
  } else {
    show_frames_comp = (COMPARE_STATE)((int)show_frames_comp + 1); // show_frames_comp++ doesn't work
  }
}

bool update_state() {
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
    final_state();
    break;
  default:
    break;
  }

  return true;
}

void switch_frames() {
  InputManager& input_manager = InputManager::getInstance();

  if (current_frame > 0) {
    if (
      input_manager.isButtonClicked(BTN_A_PIN) || (
        input_manager.isJoystickXChanged() &&
        input_manager.getJoystickXDirection() == NEG
      )
    ) {
      current_frame--;
    }
  }

  if (current_frame + 1 < frames_to_remember) {
    if (
      input_manager.isButtonClicked(BTN_B_PIN) || (
        input_manager.isJoystickXChanged() &&
        input_manager.getJoystickXDirection() == POS
      )
    ) {
      current_frame++;
    }
  }
}

bool compare_frames(
  COLORS frame1[LED_COUNT_X][LED_COUNT_Y],
  COLORS frame2[LED_COUNT_X][LED_COUNT_Y]
) {
  for (uint8_t i = 0; i < LED_COUNT_X; i++)
    for (uint8_t j = 0; j < LED_COUNT_Y; j++)
      if (frame1[i][j] != frame2[i][j])
        return false;
  return true;
}

void set_frames(COLORS current_frames[MAX_FRAMES][LED_COUNT_X][LED_COUNT_Y]) {
  navigate_leds();
  InputManager& input_manager = InputManager::getInstance();

  int current_color = current_frames[current_frame][current_index_x][current_index_y];
  if (input_manager.isButtonClicked(BTN_A_PIN)) {
    if (current_color > 0) {
      current_color--;
    } else {
      current_color = COLORS_COUNT - 1;
    }
  }

  if (input_manager.isButtonClicked(BTN_B_PIN)) {
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
  InputManager& input_manager = InputManager::getInstance();

  if (input_manager.isJoystickXChanged()) {
    if (input_manager.getJoystickXDirection() == NEG) {
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

  if (input_manager.isJoystickYChanged()) {
    if (input_manager.getJoystickYDirection() == NEG) {
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
