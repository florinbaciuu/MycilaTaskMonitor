// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaTaskMonitor.h>

#include <MycilaLogger.h>

#define TAG "MONITOR"

void Mycila::TaskMonitorClass::begin(const size_t expectedTaskCount) {
  if (_enabled)
    return;
  _taskNames.reserve(expectedTaskCount);
  _enabled = true;
}

void Mycila::TaskMonitorClass::end() {
  if (!_enabled)
    return;
  _enabled = false;
}

void Mycila::TaskMonitorClass::loop() {
  if (_enabled && _taskNames.size() > 0 && millis() - _last > MYCILA_TASK_MONITOR_FREQUENCY) {
    for (const char* name : _taskNames) {
      const TaskHandle_t handle = xTaskGetHandle(name);
      if (handle) {
        const UBaseType_t size = uxTaskGetStackHighWaterMark(handle);
        const UBaseType_t priority = uxTaskPriorityGet(handle);
        if (size < MYCILA_TASK_MONITOR_STACK_FREE_MIN)
          Mycila::Logger.warn(TAG, "%-10.10s (p=%u) %u bytes", name, priority, size);
        else if (size > MYCILA_TASK_MONITOR_STACK_FREE_MAX)
          Mycila::Logger.info(TAG, "%-10.10s (p=%u) %u bytes", name, priority, size);
        else
          Mycila::Logger.debug(TAG, "%-10.10s (p=%u) %u bytes", name, priority, size);
      } else {
        Logger.warn(TAG, "%-10.10s Handle not accessible", name);
      }
    }
    _last = millis();
  }
}

void Mycila::TaskMonitorClass::toJson(const JsonObject& root) const {
  for (const char* name : _taskNames) {
    TaskHandle_t handle = xTaskGetHandle(name);
    if (handle) {
      root[name]["stackHighWaterMark"] = uxTaskGetStackHighWaterMark(handle);
      root[name]["priority"] = uxTaskPriorityGet(handle);
    }
  }
}

namespace Mycila {
  TaskMonitorClass TaskMonitor;
} // namespace Mycila
