#include "led.h"
#include "ble.h"
#include "data.h"
#include "sensor.h"
#include "history.h"
#include "driver/gpio.h"

const int LED_PIN = 10;
const unsigned long PUSH_HISTORY_FREQUENCY_MS = 3600000UL;
const unsigned long long SLEEP_DURATION_US = 1000000ULL;

Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);
static uint32_t last_history_save; 

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("BOOT — built " __DATE__ " " __TIME__);
  setupLed(&led);
  ledOk(&led);
  setupBle("ATMOSPHANTASTIC");
  setupSensor();
  last_history_save = (uint32_t) millis();
  setupHistory();
  setCpuFrequencyMhz(80);
  setupConnectButton();
  Serial.println("Setup ready!");
  ledOff(&led);
}

void loop() {
  // Stay awake while a connection is ongoing.
  // Relies on the connector to disconnect when done, so vulnerable to locking - by mistake or an attacker.
  if (ble_device_connected) {
    ledConnecting(&led);
    delay(100);
    return;
  }

  // Advertise while holding the connect button (boot).
  bool boot_pressed = gpio_get_level(GPIO_NUM_9) == 0;
  if (boot_pressed) {
    startAdvertising();
    ledConnecting(&led);
    delay(50);
    ledOff(&led);
    delay(50);
    return;
  } else if (!boot_pressed && !ble_device_connected) {
    stopAdvertising();
  }

  ledOk(&led);
  sense();
  if (measurement_ready) {
    ledConnecting(&led);
    advertiseMeasurement(&measurement);
    updateHistory();
    measurement_ready = false;
  }

  lightSleep(SLEEP_DURATION_US); 
}

/**
* Configure the BOOT button to act as an input.
*/
void setupConnectButton() {
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << GPIO_NUM_9),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);
}

/**
* Checks if it's time to save another measurement in history and does so.
*/
void updateHistory() {
  uint32_t millis_now = (uint32_t)millis();
  if (measurement.timestamp > 0 && millis_now - last_history_save > PUSH_HISTORY_FREQUENCY_MS) {
    HistoricalDataPoint data_point = (HistoricalDataPoint){
      measurement.timestamp,
      measurement.temp,
      measurement.pressure,
      measurement.static_iaq
    };
    pushHistory(&data_point);
    last_history_save = millis_now;
  }
}

/**
* Enter light sleep, keeping peripherals powered, for duration microseconds.
*/
void lightSleep(unsigned long long duration_us) {
  ledOff(&led);
  esp_sleep_enable_timer_wakeup(duration_us);
  esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_ON);
  esp_err_t sleep_err = esp_light_sleep_start();
  Serial.printf("Woke up, sleep result: %d\n", sleep_err);
  Serial.flush();
}