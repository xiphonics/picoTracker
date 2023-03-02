#include "input.h"
#include "Adapters/PICO/Utils/utils.h"
#include <cstdio>

// TODO: Finetune this, 4us seems safe. Maybe move inputs check to timer irq?
const unsigned int SLEEP_DELAY = 4;
const unsigned int COL1_GPIO = 16;
const unsigned int COL2_GPIO = 17;
const unsigned int COL3_GPIO = 18;
const unsigned int ROW1_GPIO = 19;
const unsigned int ROW2_GPIO = 20;
const unsigned int ROW3_GPIO = 21;

void inputInit() {

  gpio_init(COL1_GPIO);
  gpio_set_dir(COL1_GPIO, GPIO_IN);
  gpio_init(COL2_GPIO);
  gpio_set_dir(COL2_GPIO, GPIO_IN);
  gpio_init(COL3_GPIO);
  gpio_set_dir(COL3_GPIO, GPIO_IN);
  gpio_init(ROW1_GPIO);
  gpio_set_dir(ROW1_GPIO, GPIO_IN);
  gpio_init(ROW2_GPIO);
  gpio_set_dir(ROW2_GPIO, GPIO_IN);
  gpio_init(ROW3_GPIO);
  gpio_set_dir(ROW3_GPIO, GPIO_IN);

  gpio_pull_up(COL1_GPIO);
  gpio_pull_up(COL2_GPIO);
  gpio_pull_up(COL3_GPIO);
  gpio_pull_down(ROW1_GPIO);
  gpio_pull_down(ROW2_GPIO);
  gpio_pull_down(ROW3_GPIO);
}

uint16_t scanKeys() {
  uint16_t mask = 0;

  // TODO: make this nicer
  gpio_set_dir(ROW1_GPIO, GPIO_OUT);
  sleep_us(SLEEP_DELAY);
  if (gpio_get(COL1_GPIO) == 0) {
    mask |= KEY_UP;
  }
  if (gpio_get(COL2_GPIO) == 0) {
    mask |= KEY_B;
  }
  if (gpio_get(COL3_GPIO) == 0) {
    mask |= KEY_A;
  }
  gpio_set_dir(ROW1_GPIO, GPIO_IN);

  gpio_set_dir(ROW2_GPIO, GPIO_OUT);
  sleep_us(SLEEP_DELAY);
  if (gpio_get(COL1_GPIO) == 0) {
    mask |= KEY_LEFT;
  }
  if (gpio_get(COL2_GPIO) == 0) {
    mask |= KEY_DOWN;
  }
  if (gpio_get(COL3_GPIO) == 0) {
    mask |= KEY_RIGHT;
  }
  gpio_set_dir(ROW2_GPIO, GPIO_IN);

  gpio_set_dir(ROW3_GPIO, GPIO_OUT);
  sleep_us(SLEEP_DELAY);
  if (gpio_get(COL1_GPIO) == 0) {
    mask |= KEY_L;
  }
  if (gpio_get(COL2_GPIO) == 0) {
    mask |= KEY_R;
  }
  if (gpio_get(COL3_GPIO) == 0) {
    mask |= KEY_START;
  }
  gpio_set_dir(ROW3_GPIO, GPIO_IN);

  return mask;
}
