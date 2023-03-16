#include "input.h"
#include "Adapters/PICO/Utils/utils.h"
#include "Adapters/PICO/platform/platform.h"
#include <cstdio>

// TODO: Finetune this, 4us seems safe. Maybe move inputs check to timer irq?
const unsigned int SLEEP_DELAY = 4;

uint16_t scanKeys() {
  uint16_t mask = 0;

  // TODO: make this nicer
  gpio_set_dir(INPUT_ROW1, GPIO_OUT);
  sleep_us(SLEEP_DELAY);
  if (gpio_get(INPUT_COL1) == 0) {
    mask |= KEY_UP;
  }
  if (gpio_get(INPUT_COL2) == 0) {
    mask |= KEY_B;
  }
  if (gpio_get(INPUT_COL3) == 0) {
    mask |= KEY_A;
  }
  gpio_set_dir(INPUT_ROW1, GPIO_IN);

  gpio_set_dir(INPUT_ROW2, GPIO_OUT);
  sleep_us(SLEEP_DELAY);
  if (gpio_get(INPUT_COL1) == 0) {
    mask |= KEY_LEFT;
  }
  if (gpio_get(INPUT_COL2) == 0) {
    mask |= KEY_DOWN;
  }
  if (gpio_get(INPUT_COL3) == 0) {
    mask |= KEY_RIGHT;
  }
  gpio_set_dir(INPUT_ROW2, GPIO_IN);

  gpio_set_dir(INPUT_ROW3, GPIO_OUT);
  sleep_us(SLEEP_DELAY);
  if (gpio_get(INPUT_COL1) == 0) {
    mask |= KEY_L;
  }
  if (gpio_get(INPUT_COL2) == 0) {
    mask |= KEY_R;
  }
  if (gpio_get(INPUT_COL3) == 0) {
    mask |= KEY_START;
  }
  gpio_set_dir(INPUT_ROW3, GPIO_IN);

  return mask;
}
