#include <Adafruit_NeoPixel.h>
#include <Adafruit_VL53L0X.h>

#define SDA_PIN 8
#define SCL_PIN 9
#define BUTTON_PIN 3
#define VIBRO_PIN 10
#define LED_PIN 5
#define LED_COUNT 10

Adafruit_NeoPixel leds(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
bool lox_initialized = false;

void setRingColor(uint32_t color) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds.setPixelColor(i, color);
  }
}

void setArcColor(uint16_t count, uint32_t color) {
  for (int i = 0; i < count; i++) {
    leds.setPixelColor(i, color);
  }
}

void test_leds() {
  for (uint32_t c = 0xFF0000; c >= 0x0000FF; c >>= 8) {
    for (int i = 0; i < LED_COUNT; i++) {
      leds.setPixelColor(i, c);
      leds.show();
      delay(100);
    }
  }
  delay(900);
  leds.clear();
  leds.show();
}

void test_vibro() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(VIBRO_PIN, HIGH);
    delay(100);
    digitalWrite(VIBRO_PIN, LOW);
    delay(100);
  }
}

void init_vl53l0x() {
  if (!lox.begin()) {
    for (int i = 0; i < 5; i++) {
      lox_initialized = false;
      setRingColor(0xFF0000);
      leds.show();
      delay(200);
      leds.clear();
      leds.show();
      delay(200);
    }
  } else {
    lox_initialized = true;
    setRingColor(0x00FF00);
    leds.show();
    delay(2000);
    leds.clear();
    leds.show();
  }
}

void loop_vl53l0x() {
  static uint16_t previousDistance = 0;
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  auto currentDistance = measure.RangeMilliMeter;

  if (abs(currentDistance - previousDistance) > 15) { // apply hysteresis
    previousDistance = currentDistance;
    leds.clear();
    if (currentDistance < 40) {
      setRingColor(0xFF0000);
    } else if (currentDistance <= 360) {
      auto led = map(currentDistance, 40, 360, 0, LED_COUNT);
      led = constrain(led, 0, LED_COUNT - 1);
      setArcColor(LED_COUNT - led, 0xFF0000);
    } else {
      setRingColor(0x0000FF);
    }
    leds.show();
  }
}

void loop_gpio() {
  digitalWrite(VIBRO_PIN, !digitalRead(BUTTON_PIN));
}

void loop_leds() {
  static auto led = 0;
  leds.clear();
  leds.setPixelColor(led, 0xFF0000);
  leds.show();
  led = (led + 1) % 10;
  delay(30);
}

void setup() {
  Wire.setPins(SDA_PIN, SCL_PIN);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(VIBRO_PIN, OUTPUT);

  leds.begin();
  leds.show();
  leds.setBrightness(10);
  test_leds();
  test_vibro();
  init_vl53l0x();
}

void loop() {
  if (lox_initialized) {
    loop_vl53l0x();
  } else {
    loop_leds();
  }
  loop_gpio();
}