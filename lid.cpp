#include "lid.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include "pin_config.h"

void lidBegin() {
  // After a deep sleep the pin is still an RTC GPIO and PA is still held low;
  // both have to be handed back before anything else uses them.
  rtc_gpio_deinit((gpio_num_t)LID_PIN);
  gpio_hold_dis((gpio_num_t)PA);
  gpio_deep_sleep_hold_dis();
  pinMode(LID_PIN, INPUT_PULLUP);
}

bool lidClosed() { return digitalRead(LID_PIN) == LOW; }

bool lidWokeUs() { return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0; }

void lidSleepNow() {
  // A floating PA can switch the amplifier on for the whole sleep, which would
  // cost more than everything else asleep put together.
  digitalWrite(PA, LOW);
  gpio_hold_en((gpio_num_t)PA);
  gpio_deep_sleep_hold_en();

  // ext0 keeps the RTC peripherals powered, and wakes on the pin going high:
  // the sensor letting go of it once the magnet moves away.
  rtc_gpio_init((gpio_num_t)LID_PIN);
  rtc_gpio_set_direction((gpio_num_t)LID_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_dis((gpio_num_t)LID_PIN);
#if LID_SLEEP_PULLUP
  rtc_gpio_pullup_en((gpio_num_t)LID_PIN);     // a bare reed switch needs it
#else
  rtc_gpio_pullup_dis((gpio_num_t)LID_PIN);    // the Hall sensor drives the pin
#endif
  esp_sleep_enable_ext0_wakeup((gpio_num_t)LID_PIN, 1);

  // If the lid already opened again between the check and here, ext0 is
  // level-triggered and the chip wakes straight back up: nothing is missed.
  esp_deep_sleep_start();
}
