#pragma once

#include "esphome.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/water_heater/water_heater.h"
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
  
  // Публичные методы для кнопок
  void black_tea() { this->send_preset(95, 0x3D); }
  void mix_tea()   { this->send_preset(40, 0x3B); }
  void white_tea() { this->send_preset(65, 0x3C); }
  void green_tea() { this->send_preset(80, 0x7C); }
  void oolong_tea(){ this->send_preset(90, 0x7D); }
  void bag_tea()   { this->send_preset(100, 0x7E); }
  void boil()      { this->send_preset(100, 0x3B); }
  void keep_warm() { this->send_preset(40, 0x63); }
  void turn_off()  { this->send_command(0x00, 0x00, 0x00, 0x00, 0x00); }
  void set_target_temperature(float temp);
  
  // Геттеры для сенсоров
  float get_current_temperature() const { return this->current_temp_; }
  float get_target_temperature() const { return this->target_temperature_; }
  std::string get_mode_text() const { return this->mode_text_; }
  bool get_no_kettle() const { return this->no_kettle_; }
  bool get_no_water() const { return this->no_water_; }
  
  // WaterHeater методы
  void setup() override;
  void loop() override;
  void control(const water_heater::WaterHeaterCall &call) override;
  void dump_config() override;
  
 protected:
  uart::UARTComponent *parent_{nullptr};
  std::vector<uint8_t> buffer_;
  float current_temp_ = 0.0;
  float target_temperature_ = 70.0;
  bool no_kettle_ = false;
  bool no_water_ = false;
  std::string mode_text_ = "Выключен";
  bool sync_mode_ = false;
  uint32_t last_command_time_ = 0;
  
  void process_frame();
  uint16_t calculate_checksum(uint8_t *data, int len);
  void send_command(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5);
  void send_preset(uint8_t temp, uint8_t mode_byte);
  void send_target_temperature_command(uint8_t temp);
  void update_state();
};

}  // namespace polaris
}  // namespace esphome
