#include <Adafruit_VL53L0X.h>
#include <AppKit.h>
#include <Date.h>
#include <Logger.h>
#include <Pixel.h>
#include <Uniot.h>

#define INTERNAL_LED_PIN 8
#define SDA_PIN 8
#define SCL_PIN 9
#define BUTTON_PIN 3
#define VIBRO_PIN 10
#define LED_PIN 5
#define LED_COUNT 10

using namespace uniot;

Pixel pixel(LED_COUNT, LED_PIN);

auto taskPrintHeap = TaskScheduler::make([](SchedulerTask& self, short t) {
  Serial.println(ESP.getFreeHeap());
});

auto taskPrintTime = TaskScheduler::make([](SchedulerTask& self, short t) {
  Serial.println(Date::getFormattedTime());
});

void inject() {
  PrimitiveExpeditor::getRegisterManager().setDigitalOutput(VIBRO_PIN);
  PrimitiveExpeditor::getRegisterManager().setDigitalInput(BUTTON_PIN);

  auto& MainAppKit = AppKit::getInstance();
  MainAppKit.getLisp().pushPrimitive([](Root root, VarObject env, VarObject list) {
    return pixel.primitiveSet(root, env, list);
  });
  MainAppKit.getLisp().pushPrimitive([](Root root, VarObject env, VarObject list) {
    return pixel.primitiveClear(root, env, list);
  });
  MainAppKit.getLisp().pushPrimitive([](Root root, VarObject env, VarObject list) {
    return pixel.primitiveShow(root, env, list);
  });
  MainAppKit.configureNetworkController({.pinBtn = BUTTON_PIN,
                                         .pinLed = INTERNAL_LED_PIN,
                                         .activeLevelLed = LOW,
                                         .maxRebootCount = 255});
  MainEventBus.registerKit(MainAppKit);

  MainScheduler
      .push(MainAppKit)
      .push("print_time", taskPrintTime)
      .push("print_heap", taskPrintHeap);

  taskPrintHeap->attach(500);
  taskPrintTime->attach(500);

  MainAppKit.attach();

  UNIOT_LOG_INFO("%s: %s", "DEVICE_ID", MainAppKit.getCredentials().getDeviceId().c_str());
  UNIOT_LOG_INFO("%s: %s", "OWNER_ID", MainAppKit.getCredentials().getOwnerId().c_str());
}
