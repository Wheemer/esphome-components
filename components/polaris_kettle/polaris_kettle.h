#pragma once

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

class PolarisKettle : public water_heater::WaterHeater, public Component, public uart::UARTDevice {
 public:
  PolarisKettle() = default;
  
  void set_uart_parent(uart::UARTComponent *parent) {
    parent_ = parent;
    this->uart_ = parent;
  }
  
  // Регистрация сенсоров
  void register_current_sensor(sensor::Sensor *sensor) { current_sensor_ = sensor; }
  void register_target_sensor(sensor::Sensor *sensor) { target_sensor_ = sensor; }
  void register_mode_text_sensor(text_sensor::TextSensor *sensor) { mode_text_sensor_ = sensor; }
  void register_no_kettle_sensor(binary_sensor::BinarySensor *sensor) { no_kettle_sensor_ = sensor; }
  void register_no_water_sensor(binary_sensor::BinarySensor *sensor) { no_water_sensor_ = sensor; }
  void register_button(button::Button *button, const std::string &type);
  
  // Публичные методы для кнопок
  void black_tea() { send_preset(95, 0x3D); }
  void mix_tea()   { send_preset(40, 0x3B); }
  void white_tea() { send_preset(65, 0x3C); }
  void green_tea() { send_preset(80, 0x7C); }
  void oolong_tea(){ send_preset(90, 0x7D); }
  void bag_tea()   { send_preset(100, 0x7E); }
  void boil()      { send_preset(100, 0x3B); }
  void keep_warm() { send_preset(40, 0x63); }
  
  void setup() override;
  void loop() override;
  void control(const water_heater::WaterHeaterCall &call) override;
  
  // Геттеры для сенсоров
  float get_current_temp() const { return current_temp_; }
  float get_target_temp() const { return target_temperature_; }
  std::string get_mode_text() const { return mode_text_; }
  bool get_no_kettle() const { return no_kettle_; }
  bool get_no_water() const { return no_water_; }

 protected:
  uart::UARTComponent *parent_{nullptr};
  std::vector<uint8_t> buffer_;
  float current_temp_ = 0.0;
  float target_temperature_ = 70.0;
  bool no_kettle_ = false;
  bool no_water_ = false;
  std::string mode_text_ = "Выключен";
  
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *target_sensor_{nullptr};
  text_sensor::TextSensor *mode_text_sensor_{nullptr};
  binary_sensor::BinarySensor *no_kettle_sensor_{nullptr};
  binary_sensor::BinarySensor *no_water_sensor_{nullptr};
  
  void process_frame();
  uint16_t calculate_checksum(uint8_t *data, int len);
  void send_command(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5);
  void send_preset(uint8_t temp, uint8_t mode_byte);
  void send_target_temperature(uint8_t temp);
  void send_mode(water_heater::WaterHeaterMode mode);
  void update_sensors();
};

// Вспомогательный класс для кнопок
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
