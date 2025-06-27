#include <AppKit.h>
#include <Date.h>
#include <Logger.h>
#include <Pixel.h>
#include <ToF.h>
#include <Uniot.h>
#include <Vibro.h>

#define INTERNAL_LED_PIN 8
#define SDA_PIN 8
#define SCL_PIN 9
#define BUTTON_PIN 3
#define VIBRO_PIN 10
#define LED_PIN 5
#define LED_COUNT 10

using namespace uniot;

Pixel pixel(LED_COUNT, LED_PIN);
ToF tof(SDA_PIN, SCL_PIN);
Vibro vibro(VIBRO_PIN);

auto taskPrintHeap = Uniot.createTask("print_time", [](SchedulerTask& self, short t) {
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
});

auto taskPrintTime = Uniot.createTask("print_heap", [](SchedulerTask& self, short t) {
  Serial.print("Time: ");
  Serial.println(Date::getFormattedTime());
});

void setup() {
  Uniot.registerLispDigitalOutput(VIBRO_PIN);
  Uniot.registerLispDigitalInput(BUTTON_PIN);
  Uniot.configWiFiResetButton(BUTTON_PIN);
  Uniot.configWiFiStatusLed(INTERNAL_LED_PIN, LOW);
  Uniot.configWiFiResetOnReboot(5);

  Uniot.addLispPrimitive([](Root root, VarObject env, VarObject list) {
    return pixel.primitiveSet(root, env, list);
  });
  Uniot.addLispPrimitive([](Root root, VarObject env, VarObject list) {
    return pixel.primitiveClear(root, env, list);
  });
  Uniot.addLispPrimitive([](Root root, VarObject env, VarObject list) {
    return pixel.primitiveShow(root, env, list);
  });
  Uniot.addLispPrimitive([](Root root, VarObject env, VarObject list) {
    return tof.primitive(root, env, list);
  });
  Uniot.addLispPrimitive([](Root root, VarObject env, VarObject list) {
    return vibro.primitive(root, env, list);
  });

  Uniot.begin();

  taskPrintHeap->attach(500);
  taskPrintTime->attach(500);
}

void loop() {
  Uniot.loop();
}
