#include "esphome.h"
#include "esphome/components/water_heater/water_heater.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include <vector>

namespace esphome {
namespace polaris_kettle {

// Forward declarations
class PolarisSensor;
class PolarisTextSensor;
class PolarisBinarySensor;
class PolarisButton;

class PolarisKettle : public water_heater::WaterHeater, public Component, public uart::UARTDevice {
 public:
  PolarisKettle() = default;
  
  void set_uart_parent(uart::UARTComponent *parent) {
    parent_ = parent;
    this->set_uart(parent);
  }
  
  // Регистрация сенсоров
  void register_current_sensor(PolarisSensor *sensor) { current_sensor_ = sensor; }
  void register_target_sensor(PolarisSensor *sensor) { target_sensor_ = sensor; }
  void register_mode_text_sensor(PolarisTextSensor *sensor) { mode_text_sensor_ = sensor; }
  void register_no_kettle_sensor(PolarisBinarySensor *sensor) { no_kettle_sensor_ = sensor; }
  void register_no_water_sensor(PolarisBinarySensor *sensor) { no_water_sensor_ = sensor; }
  void register_button(PolarisButton *button, const std::string &type);
  
  // Публичные методы
  void black_tea() { send_preset(95, 0x3D); }
  void mix_tea()   { send_preset(40, 0x3B); }
  void white_tea() { send_preset(65, 0x3C); }
  void green_tea() { send_preset(80, 0x7C); }
  void oolong_tea(){ send_preset(90, 0x7D); }
  void bag_tea()   { send_preset(100, 0x7E); }
  void boil()      { send_preset(100, 0x3B); }
  void keep_warm() { send_preset(40, 0x63); }
  void turn_off()  { send_command(0x00, 0x00, 0x00, 0x00, 0x00); }
  void set_temperature(float temp) { send_target_temperature((uint8_t)temp); }
  
  // Геттеры
  float get_current_temperature() const { return current_temp_; }
  
  void setup() override;
  void loop() override;
  void control(const water_heater::WaterHeaterCall &call) override;
  void publish_state() override;

 protected:
  uart::UARTComponent *parent_{nullptr};
  std::vector<uint8_t> buffer_;
  float current_temp_ = 0.0;
  bool no_kettle_ = false;
  bool no_water_ = false;
  std::string mode_text_ = "Выключен";
  
  PolarisSensor *current_sensor_{nullptr};
  PolarisSensor *target_sensor_{nullptr};
  PolarisTextSensor *mode_text_sensor_{nullptr};
  PolarisBinarySensor *no_kettle_sensor_{nullptr};
  PolarisBinarySensor *no_water_sensor_{nullptr};
  
  void process_frame();
  uint16_t calculate_checksum(uint8_t *data, int len);
  void send_command(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5);
  void send_preset(uint8_t temp, uint8_t mode_byte);
  void send_target_temperature(uint8_t temp);
  void send_mode(water_heater::WaterHeaterMode mode);
  void update_sensors();
};

// Вспомогательные классы
class PolarisSensor : public sensor::Sensor, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void setup() override {}
  void update() override {}
  void loop() override {}
  void publish(float value) { publish_state(value); }
 protected:
  PolarisKettle *parent_;
};

class PolarisTextSensor : public text_sensor::TextSensor, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void setup() override {}
  void update() override {}
  void loop() override {}
  void publish(const std::string &value) { publish_state(value); }
 protected:
  PolarisKettle *parent_;
};

class PolarisBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void setup() override {}
  void update() override {}
  void loop() override {}
  void publish(bool value) { publish_state(value); }
 protected:
  PolarisKettle *parent_;
};

class PolarisButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void set_type(const std::string &type) { type_ = type; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
  std::string type_;
};

}  // namespace polaris_kettle
}  // namespace esphome
