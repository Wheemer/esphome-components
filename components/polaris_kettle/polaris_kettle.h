#include "esphome.h"
#include "esphome/components/water_heater/water_heater.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/button/button.h"
#include <vector>

namespace esphome {
namespace polaris_kettle {

// ========== КНОПКИ ДЛЯ РЕЖИМОВ ЧАЯ ==========

class BlackTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override { parent_->black_tea(); }
 protected:
  PolarisKettle *parent_;
};

class MixTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override { parent_->mix_tea(); }
 protected:
  PolarisKettle *parent_;
};

class WhiteTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override { parent_->white_tea(); }
 protected:
  PolarisKettle *parent_;
};

class GreenTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override { parent_->green_tea(); }
 protected:
  PolarisKettle *parent_;
};

class OolongTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override { parent_->oolong_tea(); }
 protected:
  PolarisKettle *parent_;
};

class BagTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override { parent_->bag_tea(); }
 protected:
  PolarisKettle *parent_;
};

class BoilButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override { parent_->boil(); }
 protected:
  PolarisKettle *parent_;
};

class KeepWarmButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override { parent_->keep_warm(); }
 protected:
  PolarisKettle *parent_;
};

// ========== ОСНОВНОЙ КОМПОНЕНТ ВОДОНАГРЕВАТЕЛЯ ==========

class PolarisKettle : public water_heater::WaterHeater, public PollingComponent, public uart::UARTDevice {
 public:
  PolarisKettle() : PollingComponent(1000) {}
  
  void set_uart_parent(uart::UARTComponent *parent) {
    this->parent_ = parent;
    this->set_uart(parent);
  }
  
  // Связывание сенсоров
  void set_current_temperature_sensor(sensor::Sensor *sensor) { current_temp_sensor_ = sensor; }
  void set_target_temperature_sensor(sensor::Sensor *sensor) { target_temp_sensor_ = sensor; }
  void set_mode_text_sensor(text_sensor::TextSensor *sensor) { mode_text_sensor_ = sensor; }
  void set_no_kettle_binary_sensor(binary_sensor::BinarySensor *sensor) { no_kettle_sensor_ = sensor; }
  void set_no_water_binary_sensor(binary_sensor::BinarySensor *sensor) { no_water_sensor_ = sensor; }
  
  // Связывание кнопок
  void set_black_tea_button(button::Button *button) { black_tea_button_ = button; }
  void set_mix_tea_button(button::Button *button) { mix_tea_button_ = button; }
  void set_white_tea_button(button::Button *button) { white_tea_button_ = button; }
  void set_green_tea_button(button::Button *button) { green_tea_button_ = button; }
  void set_oolong_tea_button(button::Button *button) { oolong_tea_button_ = button; }
  void set_bag_tea_button(button::Button *button) { bag_tea_button_ = button; }
  void set_boil_button(button::Button *button) { boil_button_ = button; }
  void set_keep_warm_button(button::Button *button) { keep_warm_button_ = button; }
  
  void setup() override {
    ESP_LOGI("polaris", "Polaris Kettle Component Started");
    
    // Настройка возможностей водонагревателя
    this->traits_.set_supports_current_temperature(true);
    this->traits_.set_supports_target_temperature(true);
    this->traits_.set_supports_away_mode(false);
    this->traits_.set_min_temperature(40.0);
    this->traits_.set_max_temperature(100.0);
    this->traits_.set_target_temperature_step(1.0);
    this->traits_.set_supported_modes({
        water_heater::WATER_HEATER_MODE_OFF,
        water_heater::WATER_HEATER_MODE_ECO,
        water_heater::WATER_HEATER_MODE_PERFORMANCE,
        water_heater::WATER_HEATER_MODE_GAS,
    });
  }
  
  void loop() override {
    while (this->available()) {
      uint8_t data;
      this->read_byte(&data);
      buffer_.push_back(data);
      
      // Ищем префикс 0x81
      if (buffer_.size() == 1 && buffer_[0] != 0x81) {
        buffer_.clear();
        continue;
      }
      
      if (buffer_.size() >= 8) {
        process_frame();
        buffer_.clear();
      }
    }
  }
  
  void update() override {
    // Периодический запрос состояния
    send_command(0x00, 0x00, 0x00, 0x00, 0x00);
  }
  
  void control(const water_heater::WaterHeaterCall &call) override {
    bool updated = false;
    
    if (call.get_target_temperature().has_value()) {
      float new_temp = *call.get_target_temperature();
      this->target_temperature = new_temp;
      send_target_temperature((uint8_t)new_temp);
      if (target_temp_sensor_) target_temp_sensor_->publish_state(new_temp);
      updated = true;
    }
    
    if (call.get_mode().has_value()) {
      this->mode = *call.get_mode();
      send_mode(this->mode);
      updated = true;
    }
    
    if (updated) {
      this->publish_state();
    }
  }
  
  void publish_state() override {
    this->current_temperature = current_temp_;
    if (current_temp_sensor_) current_temp_sensor_->publish_state(current_temp_);
    if (target_temp_sensor_) target_temp_sensor_->publish_state(this->target_temperature);
    this->water_heater::WaterHeater::publish_state();
  }
  
  // ========== ПУБЛИЧНЫЕ МЕТОДЫ ДЛЯ КНОПОК ==========
  void black_tea() { send_preset(95, 0x3D); }      // черный чай / кофе / пауэр 95
  void mix_tea()   { send_preset(40, 0x3B); }      // смесь 40
  void white_tea() { send_preset(65, 0x3C); }      // белый 65
  void green_tea() { send_preset(80, 0x7C); }      // зеленый цветочный 80
  void oolong_tea(){ send_preset(90, 0x7D); }      // красный улун травяной 90
  void bag_tea()   { send_preset(100, 0x7E); }     // пакетированный 100
  void boil()      { send_preset(100, 0x3B); }     // кипячение
  void keep_warm() { send_preset(40, 0x63); }      // разогрев с удержанием
  void turn_on()   { send_power(0x00, 0x01, 70, 0x3C); }
  void turn_off()  { send_power(0x00, 0x00, 0x00, 0x00); }
  void set_temperature(float temp) { send_target_temperature((uint8_t)temp); }

 protected:
  uart::UARTComponent *parent_{nullptr};
  std::vector<uint8_t> buffer_;
  float current_temp_ = 0.0;
  bool has_error_ = false;
  bool no_water_ = false;
  
  // Сенсоры
  sensor::Sensor *current_temp_sensor_{nullptr};
  sensor::Sensor *target_temp_sensor_{nullptr};
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
  
  void process_frame() {
    if (buffer_.size() < 8) return;
    uint8_t *frame = buffer_.data();
    
    if (frame[0] != 0x81) return;
    
    // Проверка контрольной суммы
    uint16_t calc_sum = 0;
    for (int i = 0; i < 6; i++) calc_sum += frame[i];
    uint16_t recv_sum = (frame[6] << 8) | frame[7];
    
    if (calc_sum != recv_sum) {
      ESP_LOGW("polaris", "Checksum error");
      return;
    }
    
    uint8_t status = frame[1];
    uint8_t mode = frame[2];
    uint8_t target_temp = frame[3];
    uint8_t current_temp = frame[4];
    
    current_temp_ = (float)current_temp;
    this->target_temperature = (float)target_temp;
    
    // Обновляем сенсоры
    if (current_temp_sensor_) current_temp_sensor_->publish_state(current_temp_);
    if (target_temp_sensor_) target_temp_sensor_->publish_state(this->target_temperature);
    
    // Обработка ошибок
    if (status == 0xFF) {
      has_error_ = true;
      if (frame[3] == 0x64 && frame[4] == 0x55) {
        no_water_ = true;
        ESP_LOGW("polaris", "НЕТ ВОДЫ!");
        if (no_water_sensor_) no_water_sensor_->publish_state(true);
        if (no_kettle_sensor_) no_kettle_sensor_->publish_state(false);
      } else {
        ESP_LOGW("polaris", "НЕТ ЧАЙНИКА!");
        if (no_kettle_sensor_) no_kettle_sensor_->publish_state(true);
        if (no_water_sensor_) no_water_sensor_->publish_state(false);
      }
      this->mode = water_heater::WATER_HEATER_MODE_OFF;
      if (mode_text_sensor_) mode_text_sensor_->publish_state("Ошибка");
    } else {
      has_error_ = false;
      no_water_ = false;
      if (no_kettle_sensor_) no_kettle_sensor_->publish_state(false);
      if (no_water_sensor_) no_water_sensor_->publish_state(false);
      
      // Определяем режим
      if (mode == 0x00) {
        this->mode = water_heater::WATER_HEATER_MODE_OFF;
        if (mode_text_sensor_) mode_text_sensor_->publish_state("Выключен");
      } else if (mode == 0x01) {
        if (frame[4] == 0x3B || frame[4] == 0x7B) {
          this->mode = water_heater::WATER_HEATER_MODE_PERFORMANCE;
          if (mode_text_sensor_) mode_text_sensor_->publish_state("Кипячение");
        } else if (frame[4] == 0x63 || frame[4] == 0x3C || frame[4] == 0x3D) {
          this->mode = water_heater::WATER_HEATER_MODE_ECO;
          if (mode_text_sensor_) mode_text_sensor_->publish_state("Подогрев");
        } else {
          this->mode = water_heater::WATER_HEATER_MODE_GAS;
          if (mode_text_sensor_) mode_text_sensor_->publish_state("Чайная церемония");
        }
      } else {
        this->mode = water_heater::WATER_HEATER_MODE_GAS;
        if (mode_text_sensor_) mode_text_sensor_->publish_state("Чайная церемония");
      }
    }
    
    this->publish_state();
    
    ESP_LOGD("polaris", "Tекущая: %.1f°C | Целевая: %.1f°C | Режим: %d",
             current_temp_, this->target_temperature, this->mode);
  }
  
  uint16_t calculate_checksum(uint8_t *data, int len) {
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) sum += data[i];
    return sum;
  }
  
  void send_command(uint8_t byte1, uint8_t byte2, uint8_t byte3, 
                    uint8_t byte4, uint8_t byte5) {
    uint8_t cmd[8] = {0x81, byte1, byte2, byte3, byte4, byte5, 0x00, 0x00};
    uint16_t sum = calculate_checksum(cmd, 6);
    cmd[6] = (sum >> 8) & 0xFF;
    cmd[7] = sum & 0xFF;
    this->write_array(cmd, 8);
  }
  
  void send_preset(uint8_t temp, uint8_t mode_byte) {
    send_command(0x00, 0x01, temp, mode_byte, 0x00);
    ESP_LOGI("polaris", "Команда: температура=%d°C, режим=0x%02X", temp, mode_byte);
  }
  
  void send_power(uint8_t status, uint8_t mode, uint8_t temp, uint8_t flag) {
    send_command(status, mode, temp, flag, 0x00);
  }
  
  void send_target_temperature(uint8_t temp) {
    send_command(0x00, 0x01, temp, 0x3C, 0x00);
    ESP_LOGI("polaris", "Установка температуры: %d°C", temp);
  }
  
  void send_mode(water_heater::WaterHeaterMode mode) {
    switch (mode) {
      case water_heater::WATER_HEATER_MODE_OFF:
        send_command(0x00, 0x00, 0x00, 0x00, 0x00);
        break;
      case water_heater::WATER_HEATER_MODE_PERFORMANCE:
        send_preset(100, 0x3B);
        break;
      case water_heater::WATER_HEATER_MODE_ECO:
        send_preset(40, 0x63);
        break;
      default:
        send_preset((uint8_t)this->target_temperature, 0x3C);
        break;
    }
  }
};

}  // namespace polaris_kettle
}  // namespace esphome
