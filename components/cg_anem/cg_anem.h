#pragma once

#include "esphome/core/component.h"
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
  
  // Сеттеры для сенсоров
  void set_duct(float duct) { duct_ = duct; }
  void set_ambient_temperature_sensor(sensor::Sensor *sensor) { ambient_temperature_sensor_ = sensor; }
  void set_hotend_temperature_sensor(sensor::Sensor *sensor) { hotend_temperature_sensor_ = sensor; }
  void set_heat_power_sensor(sensor::Sensor *sensor) { heat_power_sensor_ = sensor; }
  void set_air_consumption_sensor(sensor::Sensor *sensor) { air_consumption_sensor_ = sensor; }
  void set_air_flow_rate_sensor(sensor::Sensor *sensor) { air_flow_rate_sensor_ = sensor; }
  void set_firmware_version_sensor(sensor::Sensor *sensor) { firmware_version_sensor_ = sensor; }
  void set_min_air_flow_rate_sensor(sensor::Sensor *sensor) { min_air_flow_rate_sensor_ = sensor; }
  void set_max_air_flow_rate_sensor(sensor::Sensor *sensor) { max_air_flow_rate_sensor_ = sensor; }
  
  // Сеттеры для бинарных сенсоров (добавлены)
  void set_status_up_binary_sensor(binary_sensor::BinarySensor *sensor) { status_up_binary_sensor_ = sensor; }
  void set_status_ov_binary_sensor(binary_sensor::BinarySensor *sensor) { status_ov_binary_sensor_ = sensor; }
  void set_status_wdt_binary_sensor(binary_sensor::BinarySensor *sensor) { status_wdt_binary_sensor_ = sensor; }

 protected:
  // Сенсоры
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *hotend_temperature_sensor_{nullptr};
  sensor::Sensor *heat_power_sensor_{nullptr};
  sensor::Sensor *air_consumption_sensor_{nullptr};
  sensor::Sensor *air_flow_rate_sensor_{nullptr};
  sensor::Sensor *firmware_version_sensor_{nullptr};
  sensor::Sensor *min_air_flow_rate_sensor_{nullptr};
  sensor::Sensor *max_air_flow_rate_sensor_{nullptr};
  
  // Бинарные сенсоры (добавлены)
  binary_sensor::BinarySensor *status_up_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *status_ov_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *status_wdt_binary_sensor_{nullptr};

  float duct_{91.6};  // Значение по умолчанию
  float version_{0.0};

  enum ErrorCode {
    NONE = 0,
    COMMUNICATION_FAILED,
  } error_code_{NONE};

 private:
  void read_status();
  bool read_uint16(uint8_t reg_h, uint8_t reg_l, uint16_t &value);
  float calculate_air_consumption(float speed);
};

}  // namespace cg_anem
}  // namespace esphome
