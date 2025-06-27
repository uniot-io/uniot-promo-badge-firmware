#pragma once

#include <Uniot.h>

namespace uniot {
class Vibro {
 public:
  Vibro(uint8_t pin, uint32_t period = 70) : mVibroPin(pin), mVibroPeriod(period) {
    pinMode(mVibroPin, OUTPUT);
    _initTasks();
  }

  Object primitive(Root root, VarObject env, VarObject list) {
    auto expeditor = PrimitiveExpeditor::describe("vibro", Lisp::Bool, 1, Lisp::Int)
                         .init(root, env, list);
    expeditor.assertDescribedArgs();

    auto times = expeditor.getArgInt(0);
    if (times <= 0) {
      return expeditor.makeBool(false);
    } else {
      mTaskVibro->attach(mVibroPeriod, times * 2);
    }

    return expeditor.makeBool(true);
  }

 private:
  void _initTasks() {
    mTaskVibro = Uniot.createTask("vibro", [this](SchedulerTask &self, short t) {
      if (!t) {
        digitalWrite(mVibroPin, LOW);
      } else if (t % 2 != 0) {
        digitalWrite(mVibroPin, HIGH);
      } else {
        digitalWrite(mVibroPin, LOW);
      }
    });
  }

  uint8_t mVibroPin;
  uint32_t mVibroPeriod;
  TaskScheduler::TaskPtr mTaskVibro;
};

}  // namespace uniot
