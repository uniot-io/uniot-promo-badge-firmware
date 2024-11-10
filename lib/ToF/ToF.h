#pragma once

#include <Adafruit_VL53L0X.h>
#include <LimitedQueue.h>
#include <PrimitiveExpeditor.h>
#include <TaskScheduler.h>

namespace uniot {
class ToF : public ISchedulerConnectionKit {
 public:
  ToF(int sda, int scl) : mInitialized(false) {
    Wire.setPins(sda, scl);
    mFilterBuffer.limit(5);
    _initTasks();
  }

  virtual void pushTo(TaskScheduler &scheduler) {
    scheduler.push("tof_measure", mTaskMeasure);
    scheduler.push("tof_stop_measure", mTaskStopMeasuring);
  }

  virtual void attach() {
    mInitialized = mLox.begin();
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
    mTaskMeasure = TaskScheduler::make([this](SchedulerTask &self, short t) {
      auto distance = _getDistance();
      if (distance > 0) {
        mFilterBuffer.pushLimited(distance);
      }
    });
    mTaskStopMeasuring = TaskScheduler::make([this](SchedulerTask &self, short t) {
      mTaskMeasure->detach();
      mFilterBuffer.clean();
    });
  }

  bool _startMeasuring(uint32_t intervalMs, uint32_t durationMs) {
    if (!mInitialized) {
      return false;
    }

    if (!mTaskMeasure->isAttached()) {
      mTaskMeasure->attach(intervalMs);
    }
    mTaskStopMeasuring->once(durationMs);

    return true;
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
    if (!mInitialized) {
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