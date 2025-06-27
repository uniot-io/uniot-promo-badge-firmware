#include <Uniot.h>
#include <esp_sleep.h>

#define INTERNAL_LED_PIN 8
#define BUTTON_PIN 3
#define VIBRO_PIN 10
#define WAKEUP_PIN_MASK BIT3

using namespace uniot;

auto taskPrintHeap = Uniot.createTask("print_time", [](SchedulerTask& self, short t) {
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
});

auto taskPrintTime = Uniot.createTask("print_heap", [](SchedulerTask& self, short t) {
  Serial.print("Time: ");
  Serial.println(Date::getFormattedTime());
});

void setup() {
  delay(1000);

  Uniot.registerLispDigitalOutput(VIBRO_PIN, INTERNAL_LED_PIN);
  Uniot.registerLispAnalogOutput(VIBRO_PIN, INTERNAL_LED_PIN);
  Uniot.registerLispDigitalInput(BUTTON_PIN);
  Uniot.configWiFiResetButton(BUTTON_PIN);
  Uniot.configWiFiStatusLed(INTERNAL_LED_PIN, LOW);
  Uniot.configWiFiResetOnReboot(UINT8_MAX);

  Uniot.addLispPrimitive([](Root root, VarObject env, VarObject list) {
    auto expeditor = PrimitiveExpeditor::describe("sleep_setup", Lisp::Bool, 2, Lisp::BoolInt, Lisp::Int)
                         .init(root, env, list);
    expeditor.assertDescribedArgs();
    auto gpio_enabled = expeditor.getArgBool(0);
    auto sleep_ms = expeditor.getArgInt(1);

    if (gpio_enabled) {
      esp_deep_sleep_enable_gpio_wakeup(WAKEUP_PIN_MASK, ESP_GPIO_WAKEUP_GPIO_LOW);
    }

    if (sleep_ms > 0) {
      auto sleep_us = sleep_ms * 1000;
      esp_sleep_enable_timer_wakeup(sleep_us);
    }

    return expeditor.makeBool(true);
  });

  Uniot.addLispPrimitive([](Root root, VarObject env, VarObject list) {
    auto expeditor = PrimitiveExpeditor::describe("sleep_start", Lisp::Bool, 0)
                         .init(root, env, list);
    expeditor.assertDescribedArgs();

    UNIOT_LOG_INFO("Entering deep sleep now...");
    esp_deep_sleep_start();

    return expeditor.makeBool(true);
  });

  Uniot.addLispPrimitive([](Root root, VarObject env, VarObject list) {
    auto expeditor = PrimitiveExpeditor::describe("wakeup_cause", Lisp::Int, 0)
                         .init(root, env, list);
    expeditor.assertDescribedArgs();

    auto wakeupCause = esp_sleep_get_wakeup_cause();
    switch (wakeupCause) {
      case ESP_SLEEP_WAKEUP_GPIO: {
        UNIOT_LOG_INFO("Wakeup caused by GPIO");
        return expeditor.makeInt(1);
      }
      case ESP_SLEEP_WAKEUP_TIMER: {
        UNIOT_LOG_INFO("Wakeup caused by timer");
        return expeditor.makeInt(2);
      }
      default: {
        UNIOT_LOG_INFO("Wakeup was not caused by deep sleep: %d", wakeupCause);
        break;
      }
    }

    return expeditor.makeInt(0);
  });

  Uniot.begin();
  delay(1000);

  taskPrintHeap->attach(500);
  taskPrintTime->attach(500);
}

void loop() {
  Uniot.loop();
}
