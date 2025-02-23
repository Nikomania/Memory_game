#include "GPIO.h"
#include "hardware/adc.h"
#include <stdio.h>

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
