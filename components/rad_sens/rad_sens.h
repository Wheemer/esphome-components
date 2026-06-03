#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/number/number.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace rad_sens {

// Адреса регистров RadSens
static const uint8_t REG_DEVICE_ID = 0x00;
static const uint8_t REG_FIRMWARE_VER = 0x01;
static const uint8_t REG_HV_GENERATOR_STATE = 0x03;
static const uint8_t REG_HV_GENERATOR_CONTROL = 0x04;
static const uint8_t REG_SENSITIVITY = 0x10;
static const uint8_t REG_LED_CONTROL = 0x05;
static const uint8_t REG_LOW_POWER_CONTROL = 0x06;
static const uint8_t REG_DYNAMIC_INTENSITY_LOW = 0x24;
static const uint8_t REG_DYNAMIC_INTENSITY_HIGH = 0x25;
static const uint8_t REG_STATIC_INTENSITY_LOW = 0x26;
static const uint8_t REG_STATIC_INTENSITY_HIGH = 0x27;
static const uint8_t REG_PULSE_COUNT_LOW = 0x28;
static const uint8_t REG_PULSE_COUNT_HIGH = 0x29;
static const uint8_t REG_PULSE_COUNT_2 = 0x2A;
static const uint8_t REG_PULSE_COUNT_3 = 0x2B;

static const uint8_t HV_GENERATOR_ON = 0x01;
static const uint8_t HV_GENERATOR_OFF = 0x00;
static const uint8_t LED_ON = 0x01;
static const uint8_t LED_OFF = 0x00;
static const uint8_t LOW_POWER_ON = 0x01;
static const uint8_t LOW_POWER_OFF = 0x00;

class RadSensComponent : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  
  // Настройки
  void set_update_interval(uint32_t interval) { update_interval_ = interval; }
  void set_sensitivity(uint16_t sensitivity);
  
  // Сенсоры
  void set_dynamic_intensity_sensor(sensor::Sensor *sensor) { dynamic_intensity_sensor_ = sensor; }
  void set_static_intensity_sensor(sensor::Sensor *sensor) { static_intensity_sensor_ = sensor; }
  void set_pulses_sensor(sensor::Sensor *sensor) { pulses_sensor_ = sensor; }
  void set_firmware_version_sensor(sensor::Sensor *sensor) { firmware_version_sensor_ = sensor; }

  // Binary Sensor
  void set_hv_generator_state_sensor(binary_sensor::BinarySensor *sensor) { hv_generator_state_sensor_ = sensor; }
  
  // Switch
  void set_control_led_switch(switch_::Switch *sw) { led_switch_ = sw; }
  void set_control_high_voltage_switch(switch_::Switch *sw) { hv_generator_switch_ = sw; }
  void set_control_low_power_switch(switch_::Switch *sw) { low_power_switch_ = sw; }
  
  // Number
  void set_sensitivity_number(number::Number *num) { sensitivity_number_ = num; }
  
  // Управление
  void set_hv_generator(bool state);
  bool get_hv_generator_state();
  void set_led(bool state);
  bool get_led_state();
  void set_low_power(bool state);
  bool get_low_power_state();
  uint16_t get_sensitivity() const { return sensitivity_; }
  
  float get_setup_priority() const override { return setup_priority::DATA; }
  
 protected:
  uint32_t update_interval_{60000};
  uint32_t last_update_{0};
  
  // Сенсоры
  sensor::Sensor *dynamic_intensity_sensor_{nullptr};
  sensor::Sensor *static_intensity_sensor_{nullptr};
  sensor::Sensor *pulses_sensor_{nullptr};
  sensor::Sensor *firmware_version_sensor_{nullptr};

  // Binary Sensor
  binary_sensor::BinarySensor *hv_generator_state_sensor_{nullptr};
  
  // Switch
  switch_::Switch *hv_generator_switch_{nullptr};
  switch_::Switch *led_switch_{nullptr};
  switch_::Switch *low_power_switch_{nullptr};
  
  // Number
  number::Number *sensitivity_number_{nullptr};
  
  // Состояния
  uint8_t device_id_{0};
  uint8_t firmware_version_{0};
  uint16_t sensitivity_{0};
  bool hv_generator_state_{false};
  bool led_state_{false};
  bool low_power_state_{false};
  
  bool initialized_{false};
  
  // I2C методы
  uint8_t read_register_8_(uint8_t reg);
  uint16_t read_register_16_(uint8_t reg);
  uint32_t read_register_32_(uint8_t reg);
  bool write_register_8_(uint8_t reg, uint8_t value);
  bool write_register_16_(uint8_t reg, uint16_t value);
  
  void update_sensors_();
};

}  // namespace rad_sens
}  // namespace esphome
