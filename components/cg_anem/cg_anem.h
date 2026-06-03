#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace cg_anem {

class CGAnemComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void update() override;

  float get_setup_priority() const override { return setup_priority::DATA; }
  
  // Setter методы
  void set_diameter(float diameter_mm) { 
    diameter_mm_ = diameter_mm; 
    if (diameter_mm_ > 0) {
      // ИСПРАВЛЕНИЕ: Правильный перевод мм в см для радиуса
      float radius_cm = diameter_mm_ / 20.0f;
      duct_area_cm2_ = 3.14159265358979323846f * radius_cm * radius_cm;
    } else {
      duct_area_cm2_ = -1.0f;
    }
  }
  
  void set_sleep_pin(GPIOPin *pin) { sleep_pin_ = pin; }
  void set_enable_sleep(bool enable) { enable_sleep_ = enable; }
  
  void set_ambient_temperature_sensor(sensor::Sensor *sensor) { ambient_temperature_sensor_ = sensor; }
  void set_hotend_temperature_sensor(sensor::Sensor *sensor) { hotend_temperature_sensor_ = sensor; }
  void set_temperature_difference_sensor(sensor::Sensor *sensor) { temperature_difference_sensor_ = sensor; }
  void set_heat_power_sensor(sensor::Sensor *sensor) { heat_power_sensor_ = sensor; }
  void set_air_consumption_sensor(sensor::Sensor *sensor) { air_consumption_sensor_ = sensor; }
  void set_air_flow_rate_sensor(sensor::Sensor *sensor) { air_flow_rate_sensor_ = sensor; }
  void set_firmware_version_sensor(sensor::Sensor *sensor) { firmware_version_sensor_ = sensor; }
  void set_min_air_flow_rate_sensor(sensor::Sensor *sensor) { min_air_flow_rate_sensor_ = sensor; }
  void set_max_air_flow_rate_sensor(sensor::Sensor *sensor) { max_air_flow_rate_sensor_ = sensor; }
  void set_supply_voltage_sensor(sensor::Sensor *sensor) { supply_voltage_sensor_ = sensor; }
  
  // Бинарные сенсоры (остаются для внутреннего использования)
  void set_status_up_binary_sensor(binary_sensor::BinarySensor *sensor) { status_up_binary_sensor_ = sensor; }
  void set_status_ov_binary_sensor(binary_sensor::BinarySensor *sensor) { status_ov_binary_sensor_ = sensor; }
  void set_status_wdt_binary_sensor(binary_sensor::BinarySensor *sensor) { status_wdt_binary_sensor_ = sensor; }
  void set_status_incorrect_taring_binary_sensor(binary_sensor::BinarySensor *sensor) { status_incorrect_taring_binary_sensor_ = sensor; }
  void set_status_incorrect_taring_range_binary_sensor(binary_sensor::BinarySensor *sensor) { status_incorrect_taring_range_binary_sensor_ = sensor; }

 protected:
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *hotend_temperature_sensor_{nullptr};
  sensor::Sensor *temperature_difference_sensor_{nullptr};
  sensor::Sensor *heat_power_sensor_{nullptr};
  sensor::Sensor *air_consumption_sensor_{nullptr};
  sensor::Sensor *air_flow_rate_sensor_{nullptr};
  sensor::Sensor *firmware_version_sensor_{nullptr};
  sensor::Sensor *min_air_flow_rate_sensor_{nullptr};
  sensor::Sensor *max_air_flow_rate_sensor_{nullptr};
  sensor::Sensor *supply_voltage_sensor_{nullptr};
  
  binary_sensor::BinarySensor *status_up_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *status_ov_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *status_wdt_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *status_incorrect_taring_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *status_incorrect_taring_range_binary_sensor_{nullptr};

  GPIOPin *sleep_pin_{nullptr};
  bool enable_sleep_{false};

  float diameter_mm_{-1.0f};
  float duct_area_cm2_{-1.0f};
  float firmware_version_{0.0f};

 private:
  void read_status();
  bool read_data();
  void publish_binary_sensor_state(binary_sensor::BinarySensor *sensor, bool state);
  float calculate_air_consumption(float wind_speed_mps);
};

}  // namespace cg_anem
}  // namespace esphome