#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32
#include <driver/adc.h>
#include "esp_adc_cal.h"
#endif

namespace esphome {
namespace lilygo_t_battery {

class LilygotBattery : public PollingComponent {
 public:
  sensor::Sensor *voltage{nullptr};
  sensor::Sensor *bus_voltage{nullptr};
  sensor::Sensor *battery_level{nullptr};

  // Настройки делителя напряжения (по умолчанию для TTGO T-Display)
  void set_voltage_divider(float divider) { voltage_divider_ = divider; }
  void set_enable_pin(GPIOPin *enable) { enable_pin_ = enable; }
  void set_adc_sensor(sensor::Sensor *sensor) { adc_sensor_ = sensor; }
  
  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  sensor::Sensor *adc_sensor_{nullptr};
  GPIOPin *enable_pin_{nullptr};
  float voltage_divider_ = 6.0f;  // (120k + 20k) / 20k = 6
  float reference_voltage_ = 3.3f;
  
  float read_adc_voltage_();
  float calculate_battery_voltage_(float adc_voltage);
  int calculate_battery_level_(float battery_voltage, bool is_usb_present);
  bool is_usb_present_();
  
  // Калибровка ADC для ESP-IDF
  esp_adc_cal_characteristics_t adc_chars_;
  bool adc_calibrated_ = false;
};

}  // namespace lilygo_t_battery
}  // namespace esphome
