#pragma once

#include <Adafruit_VL53L0X.h>
#include <Uniot.h>

namespace uniot {
class ToF {
 public:
  ToF(int sda, int scl) : mInitialized(false) {
    Wire.setPins(sda, scl);
    mFilterBuffer.limit(5);
    _initTasks();
  }

  Object primitive(Root root, VarObject env, VarObject list) {
    auto expeditor = PrimitiveExpeditor::describe("tof_distance", Lisp::Int, 0)
                         .init(root, env, list);
    expeditor.assertDescribedArgs();

    _startMeasuring(30, 10 * 1000);
    auto distance = _getMeasuredDistance();

    return expeditor.makeInt(distance);
  }

 private:
  void _initTasks() {
    mTaskMeasure = Uniot.createTask("tof_measure", [this](SchedulerTask &self, short t) {
      auto distance = _getDistance();
      if (distance > 0) {
        mFilterBuffer.pushLimited(distance);
      }
    });
    mTaskStopMeasuring = Uniot.createTask("tof_stop_measure", [this](SchedulerTask &self, short t) {
      mTaskMeasure->detach();
      mFilterBuffer.clean();
    });
  }

  bool _initialize() {
    if (!mInitialized) {
      mInitialized = mLox.begin();
    }

    return mInitialized;
  }

  void _startMeasuring(uint32_t intervalMs, uint32_t durationMs) {
    if (!mTaskMeasure->isAttached()) {
      mTaskMeasure->attach(intervalMs);
    }
    mTaskStopMeasuring->once(durationMs);
  }

  uint16_t _getMeasuredDistance() {
    auto size = mFilterBuffer.size();
    if (size == 0) {
      return 0;
    }

    uint32_t sum = 0;
    mFilterBuffer.forEach([&sum](const uint16_t &value) {
      sum += value;
    });
    return sum / size;
  }

  int32_t _getDistance() {
    if (!_initialize()) {
      return -1;
    }

    VL53L0X_RangingMeasurementData_t measurement;
    mLox.rangingTest(&measurement);
    // if (measurement.RangeStatus == 4) {
    //   return -1;
    // }
    return measurement.RangeMilliMeter;
  }

  Adafruit_VL53L0X mLox;
  bool mInitialized;
  LimitedQueue<uint16_t> mFilterBuffer;

  TaskScheduler::TaskPtr mTaskMeasure;
  TaskScheduler::TaskPtr mTaskStopMeasuring;
};

}  // namespace uniot