#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace hts221 {

class hts221Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
 
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) { humidity_sensor_ = humidity_sensor; }

 protected:
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};

  enum ErrorCode {
    NONE = 0,
    COMMUNICATION_FAILED,
    WHO_AM_I_MISMATCH,
    CALIBRATION_FAILED,
  } error_code_{NONE};

 private:
  bool read_calibration_();
  bool read_measurements_(float &temperature, float &humidity);
  bool is_data_ready_();
  void set_power_mode_(bool enable);
  
  // Calibration values
  float humidity_slope_{0.0f};
  float humidity_zero_{0.0f};
  float temperature_slope_{0.0f};
  float temperature_zero_{0.0f};
  
  bool calibrated_{false};
};

}  // namespace hts221
}  // namespace esphome
