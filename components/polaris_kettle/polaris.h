#pragma once

#include "esphome.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/water_heater/water_heater.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include <vector>

namespace esphome {
namespace polaris {

class PolarisKettle : public water_heater::WaterHeater, public Component, public uart::UARTDevice {
 public:
  PolarisKettle() = default;
  
  void set_uart_parent(uart::UARTComponent *parent) {
    this->parent_ = parent;
    this->uart_ = parent;
  }
  
  void set_sync_mode(bool sync_mode) { this->sync_mode_ = sync_mode; }
  
  // Регистрация сенсоров
  void set_current_temperature_sensor(sensor::Sensor *sensor) { this->current_temperature_sensor_ = sensor; }
  void set_target_temperature_sensor(sensor::Sensor *sensor) { this->target_temperature_sensor_ = sensor; }
  void set_mode_text_sensor(text_sensor::TextSensor *sensor) { this->mode_text_sensor_ = sensor; }
  void set_no_kettle_binary_sensor(binary_sensor::BinarySensor *sensor) { this->no_kettle_sensor_ = sensor; }
  void set_no_water_binary_sensor(binary_sensor::BinarySensor *sensor) { this->no_water_sensor_ = sensor; }
  
  // Регистрация кнопок
  void set_black_tea_button(button::Button *button);
  void set_mix_tea_button(button::Button *button);
  void set_white_tea_button(button::Button *button);
  void set_green_tea_button(button::Button *button);
  void set_oolong_tea_button(button::Button *button);
  void set_bag_tea_button(button::Button *button);
  void set_boil_button(button::Button *button);
  void set_keep_warm_button(button::Button *button);
  
  // WaterHeater методы
  void setup() override;
  void loop() override;
  void control(const water_heater::WaterHeaterCall &call) override;
  void dump_config() override;
  
 protected:
  uart::UARTComponent *parent_{nullptr};
  std::vector<uint8_t> buffer_;
  float current_temp_ = 0.0;
  float target_temp_ = 70.0;
  bool no_kettle_ = false;
  bool no_water_ = false;
  std::string mode_text_ = "Выключен";
  bool sync_mode_ = false;
  
  // Сенсоры
  sensor::Sensor *current_temperature_sensor_{nullptr};
  sensor::Sensor *target_temperature_sensor_{nullptr};
  text_sensor::TextSensor *mode_text_sensor_{nullptr};
  binary_sensor::BinarySensor *no_kettle_sensor_{nullptr};
  binary_sensor::BinarySensor *no_water_sensor_{nullptr};
  
  // Кнопки
  button::Button *black_tea_button_{nullptr};
  button::Button *mix_tea_button_{nullptr};
  button::Button *white_tea_button_{nullptr};
  button::Button *green_tea_button_{nullptr};
  button::Button *oolong_tea_button_{nullptr};
  button::Button *bag_tea_button_{nullptr};
  button::Button *boil_button_{nullptr};
  button::Button *keep_warm_button_{nullptr};
  
  void process_frame();
  uint16_t calculate_checksum(uint8_t *data, int len);
  void send_command(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5);
  void send_preset(uint8_t temp, uint8_t mode_byte);
  void update_sensors();
  void black_tea();
  void mix_tea();
  void white_tea();
  void green_tea();
  void oolong_tea();
  void bag_tea();
  void boil();
  void keep_warm();
};

}  // namespace polaris
}  // namespace esphome
