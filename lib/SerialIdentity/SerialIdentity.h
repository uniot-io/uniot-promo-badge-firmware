#pragma once

#include <Uniot.h>

namespace uniot {

/*
 * One line that says what a device is running, for tools that need to know -- the web
 * installer above all.
 *
 *   query:  UNIOT?
 *   reply:  UNIOT device=badge version=0.4.0 core=0.9.0 lisp=0.4.0 variant=compatible
 *
 * The query is one line on the serial port, ended by CR, LF or CRLF. The reply is printed
 * once by begin(), for whoever is reading the console, and again for every query.
 *
 * A tool should ask rather than wait for the boot print. The ESP32-C3's USB serial goes down
 * with a chip reset and drops output while no host is reading, and a device that has been
 * running for an hour printed its boot line long ago. The port is polled every 100 ms, so
 * allow a few hundred milliseconds per attempt and retry a couple of times before deciding
 * nothing is there to answer.
 *
 * The reply is space-separated key=value pairs, values without spaces. A tool should ignore
 * keys it does not know, so fields can be added without breaking it. variant= is left out
 * for a device that has no variants.
 */
class SerialIdentity {
 public:
  SerialIdentity(const char *device, const char *version, const char *variant = nullptr)
      : mDevice(device), mVersion(version), mVariant(variant), mLength(0), mOverflow(false) {}

  /*
   * Call from setup(), after Uniot.begin() -- not from a constructor. The query task lives on
   * Uniot's scheduler, and Uniot is a global defined in another translation unit, so a
   * global's constructor cannot count on it having been constructed yet.
   */
  void begin() {
    print();
    mTaskQuery = Uniot.createTask("serial_identity", [this](SchedulerTask &, short) {
      _poll();
    });
    mTaskQuery->attach(100);
  }

  /*
   * Formatted whole and written in one call, so nothing printing from another task can land
   * in the middle of the line; and written to Serial directly rather than through the logger,
   * so no log level can filter it out.
   */
  void print() const {
    char line[128];
    int length = snprintf(line, sizeof(line), "UNIOT device=%s version=%s core=%d.%d.%d lisp=%d.%d.%d",
                          mDevice, mVersion,
                          UNIOT_CORE_VERSION / 10000, UNIOT_CORE_VERSION / 100 % 100, UNIOT_CORE_VERSION % 100,
                          LISP_VERSION / 10000, LISP_VERSION / 100 % 100, LISP_VERSION % 100);

    if (mVariant && length > 0 && length < (int)sizeof(line)) {
      snprintf(line + length, sizeof(line) - length, " variant=%s", mVariant);
    }

    Serial.println(line);
  }

 private:
  /*
   * Reads what has arrived, a line at a time. Anything but the query is discarded, and a line
   * too long for the buffer is ignored whole rather than judged by its first bytes. The core
   * never reads the serial port, so nothing else competes for its input.
   */
  void _poll() {
    while (Serial.available()) {
      char c = Serial.read();

      if (c == '\n' || c == '\r') {
        mLine[mLength] = '\0';
        if (!mOverflow && !strcmp(mLine, "UNIOT?")) {
          print();
        }
        mLength = 0;
        mOverflow = false;
      } else if (mLength < sizeof(mLine) - 1) {
        mLine[mLength++] = c;
      } else {
        mOverflow = true;
      }
    }
  }

  const char *mDevice;
  const char *mVersion;
  const char *mVariant;

  char mLine[16];
  size_t mLength;
  bool mOverflow;

  TaskScheduler::TaskPtr mTaskQuery;
};

}  // namespace uniot
