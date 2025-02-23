#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define BTN_DEBOUNCE 100
#define ADC_BITS 12
#define ADC_MAX (1 << ADC_BITS) - 1 // 12-bit ADC
#define JST_THRESHOLD 300

struct button_state_t {
  absolute_time_t last_press_time;
  bool was_pressed;
  int pin;
  bool clicked;
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
  bool changed;
};

class InputManager {
public:
  void update();
  
  bool isButtonPressed(int pin);
  bool isButtonClicked(int pin);
  bool isJoystickXChanged();
  bool isJoystickYChanged();
  direction_t getJoystickXDirection();
  direction_t getJoystickYDirection();

  static InputManager& getInstance();
private:
  InputManager();

  void checkButtonState(button_state_t* btn_state);
  void checkJoystickState(joystick_state_t* jst_state);
  direction_t getJoystickDirection(int pin);

  button_state_t btn_A_state;
  button_state_t btn_B_state;
  button_state_t sw_state;
  joystick_state_t jst_X_state;
  joystick_state_t jst_Y_state;
};

#endif // INPUT_MANAGER_H
