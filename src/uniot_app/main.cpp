#include <AppKit.h>
#include <Logger.h>
#include <Pixel.h>
#include <SerialIdentity.h>
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

// Taken from the flag itself rather than set separately, so the identity line cannot claim a
// variant the build does not have.
#ifdef ENABLE_LOWER_WIFI_TX_POWER
#define BADGE_VARIANT "compatible"
#else
#define BADGE_VARIANT "full-range"
#endif

using namespace uniot;

Pixel pixel(LED_COUNT, LED_PIN);
ToF tof(SDA_PIN, SCL_PIN);
Vibro vibro(VIBRO_PIN);
SerialIdentity identity("badge", BADGE_VERSION, BADGE_VARIANT);

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
  identity.begin();
}

void loop() {
  Uniot.loop();
}
