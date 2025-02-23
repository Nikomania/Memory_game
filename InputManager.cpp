#include "InputManager.h"
#include "GPIO.h"

InputManager::InputManager() {
  btn_A_state = {0, false, BTN_A_PIN, false};
  btn_B_state = {0, false, BTN_B_PIN, false};
  sw_state = {0, false, SW_PIN, false};
  jst_X_state = {0, NEUTRAL, JST_X_PIN, false};
  jst_Y_state = {0, NEUTRAL, JST_Y_PIN, false};
}

InputManager& InputManager::getInstance() {
  static InputManager instance;
  return instance;
}

void InputManager::update() {
  checkButtonState(&btn_A_state);
  checkButtonState(&btn_B_state);
  checkButtonState(&sw_state);

  checkJoystickState(&jst_X_state);
  checkJoystickState(&jst_Y_state);
}

void InputManager::checkButtonState(button_state_t* btn_state) {
  bool button_pressed = !gpio_get(btn_state->pin);

  btn_state->clicked = false;
  if (
    button_pressed &&
    !btn_state->was_pressed &&
    absolute_time_diff_us(btn_state->last_press_time, get_absolute_time()) > BTN_DEBOUNCE * 1000
  ) {
    btn_state->last_press_time = get_absolute_time();
    btn_state->clicked = true;
  }

  btn_state->was_pressed = button_pressed;
}

void InputManager::checkJoystickState(joystick_state_t* jst_state) {
  direction_t direction = getJoystickDirection(jst_state->pin);

  jst_state->changed = false;
  if (
    direction != jst_state->last_direction &&
    direction != NEUTRAL &&
    absolute_time_diff_us(jst_state->last_press_time, get_absolute_time()) > BTN_DEBOUNCE * 1000
  ) {
    jst_state->last_press_time = get_absolute_time();
    jst_state->changed = true;
  }

  jst_state->last_direction = direction;
}

direction_t InputManager::getJoystickDirection(int pin) {
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

bool InputManager::isButtonPressed(int pin) {
  switch (pin) {
  case BTN_A_PIN:
    return btn_A_state.was_pressed;
    break;
  case BTN_B_PIN:
    return btn_B_state.was_pressed;
    break;
  case SW_PIN:
    return sw_state.was_pressed;
    break;
  default:
    return false;
    break;
  }
}

bool InputManager::isButtonClicked(int pin) {
  switch (pin) {
  case BTN_A_PIN:
    return btn_A_state.clicked;
    break;
  case BTN_B_PIN:
    return btn_B_state.clicked;
    break;
  case SW_PIN:
    return sw_state.clicked;
    break;
  default:
    return false;
    break;
  }
}

bool InputManager::isJoystickXChanged() {
  return jst_X_state.changed;
}

bool InputManager::isJoystickYChanged() {
  return jst_Y_state.changed;
}

direction_t InputManager::getJoystickXDirection() {
  return jst_X_state.last_direction;
}

direction_t InputManager::getJoystickYDirection() {
  return jst_Y_state.last_direction;
}
