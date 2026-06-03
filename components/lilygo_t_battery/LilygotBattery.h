#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32
#include <driver/adc.h>
#endif

namespace esphome {
namespace lilygo_t_battery {

class LilygotBattery : public PollingComponent {
 public:
  // Сенсоры
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *bus_voltage_sensor_{nullptr};
  sensor::Sensor *battery_level_sensor_{nullptr};
  
  // Настройка
  void set_adc_sensor(sensor::Sensor *sensor) { adc_sensor_ = sensor; }
  void set_enable_pin(GPIOPin *enable) { enable_pin_ = enable; }
  void set_voltage_divider(float divider) { voltage_divider_ = divider; }
  void set_reference_voltage(float voltage) { reference_voltage_ = voltage; }
  
  // Методы для установки сенсоров
  void set_voltage_sensor(sensor::Sensor *sensor) { voltage_sensor_ = sensor; }
  void set_bus_voltage_sensor(sensor::Sensor *sensor) { bus_voltage_sensor_ = sensor; }
  void set_battery_level_sensor(sensor::Sensor *sensor) { battery_level_sensor_ = sensor; }

  // Основные методы
  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  sensor::Sensor *adc_sensor_{nullptr};
  GPIOPin *enable_pin_{nullptr};
  float voltage_divider_{6.0f};
  float reference_voltage_{3.3f};
  
  // Вспомогательные методы
  float read_adc_voltage_();
  float calculate_usb_voltage_(float battery_voltage);
  int calculate_battery_level_(float battery_voltage);
};

}  // namespace lilygo_t_battery
}  // namespace esphome
