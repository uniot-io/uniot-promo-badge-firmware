# Changelog

## 1.0.0

The first versioned release, and the first one the web installer can put on a badge.

- **Built on uniot-core 0.9.0 and uniot-lisp 0.4.0.** Scripts written for the firmware badges
  first shipped with may need changes — the Lisp language moved on in between. See the
  [uniot-core changelog](https://github.com/uniot-io/uniot-core/blob/master/CHANGELOG.md).
- **Two radio builds.** *Compatible*, at reduced WiFi transmit power, works on every module
  and is the default. *Full range* keeps the chip's full power, for modules whose radio copes
  with it.
- **The badge says what it's running.** It prints one identifying line at boot and answers
  `UNIOT?` on the serial port, which is how the web installer tells an update from a new
  install.
- The heap and time lines the firmware printed twice a second are gone, so the log shows
  what the badge is doing.
