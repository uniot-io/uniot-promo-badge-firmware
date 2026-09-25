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

// Release builds get the version from CI, which passes the git tag; local builds say "dev".
#ifndef BADGE_VERSION
#define BADGE_VERSION "dev"
#endif

// Taken from the flag itself rather than set separately, so the banner cannot claim a
// variant the build does not have.
#ifdef ENABLE_LOWER_WIFI_TX_POWER
#define BADGE_VARIANT "compatible"
#else
#define BADGE_VARIANT "full-range"
#endif

#define SEMVER_PARTS(v) (v) / 10000, (v) / 100 % 100, (v) % 100

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

  // One machine-readable line naming this firmware, at every boot. The web installer resets
  // the badge and looks for it, to tell an update from a new install and to keep the radio
  // variant the badge already has. Written to Serial directly, not through the logger, so no
  // log level can filter it out.
  Serial.printf("UNIOT-BADGE version=%s core=%d.%d.%d lisp=%d.%d.%d variant=%s\n",
                BADGE_VERSION,
                SEMVER_PARTS(UNIOT_CORE_VERSION),
                SEMVER_PARTS(LISP_VERSION),
                BADGE_VARIANT);

  taskPrintHeap->attach(500);
  taskPrintTime->attach(500);
}

void loop() {
  Uniot.loop();
}
