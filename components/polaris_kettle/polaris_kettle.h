#include "esphome.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/water_heater/water_heater.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/button/button.h"
#include <vector>

namespace esphome {
namespace polaris_kettle {

// ========== ВСПОМОГАТЕЛЬНЫЕ КЛАССЫ ==========

class PolarisKettle;

class BlackTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
};

class MixTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
};

class WhiteTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
};

class GreenTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
};

class OolongTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
};

class BagTeaButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
};

class BoilButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
};

class KeepWarmButton : public button::Button, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void press_action() override;
 protected:
  PolarisKettle *parent_;
};

// ========== ТЕКУЩАЯ ТЕМПЕРАТУРА (СЕНСОР) ==========

class PolarisCurrentTemperatureSensor : public sensor::Sensor, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void setup() override {}
  void update() override {}
  void loop() override {}
 protected:
  PolarisKettle *parent_;
};

// ========== УСТАНОВЛЕННАЯ ТЕМПЕРАТУРА (СЕНСОР) ==========

class PolarisTargetTemperatureSensor : public sensor::Sensor, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void setup() override {}
  void update() override {}
  void loop() override {}
 protected:
  PolarisKettle *parent_;
};

// ========== ТЕКУЩИЙ РЕЖИМ (TEXT SENSOR) ==========

class PolarisModeTextSensor : public text_sensor::TextSensor, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void setup() override {}
  void update() override {}
  void loop() override {}
 protected:
  PolarisKettle *parent_;
};

// ========== НЕТ ЧАЙНИКА (BINARY SENSOR) ==========

class PolarisNoKettleBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void setup() override {}
  void update() override {}
  void loop() override {}
 protected:
  PolarisKettle *parent_;
};

// ========== НЕТ ВОДЫ (BINARY SENSOR) ==========

class PolarisNoWaterBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void set_parent(PolarisKettle *parent) { parent_ = parent; }
  void setup() override {}
  void update() override {}
  void loop() override {}
 protected:
  PolarisKettle *parent_;
};

// ========== ОСНОВНОЙ КОМПОНЕНТ ==========

class PolarisKettle : public water_heater::WaterHeater, public Component, public uart::UARTDevice {
 public:
  PolarisKettle() = default;
  
  void set_uart_parent(uart::UARTComponent *parent) {
    this->parent_ = parent;
    this->set_uart(parent);
  }
  
  // Регистрация сенсоров
  void register_current_temperature_sensor(PolarisCurrentTemperatureSensor *sensor) {
    current_temp_sensor_ = sensor;
  }
  
  void register_target_temperature_sensor(PolarisTargetTemperatureSensor *sensor) {
    target_temp_sensor_ = sensor;
  }
  
  void register_mode_text_sensor(PolarisModeTextSensor *sensor) {
    mode_text_sensor_ = sensor;
  }
  
  void register_no_kettle_binary_sensor(PolarisNoKettleBinarySensor *sensor) {
    no_kettle_sensor_ = sensor;
  }
  
  void register_no_water_binary_sensor(PolarisNoWaterBinarySensor *sensor) {
    no_water_sensor_ = sensor;
  }
  
  // Регистрация кнопок
  void register_black_tea_button(BlackTeaButton *button) {
    black_tea_button_ = button;
    black_tea_button_->set_parent(this);
  }
  
  void register_mix_tea_button(MixTeaButton *button) {
    mix_tea_button_ = button;
    mix_tea_button_->set_parent(this);
  }
  
  void register_white_tea_button(WhiteTeaButton *button) {
    white_tea_button_ = button;
    white_tea_button_->set_parent(this);
  }
  
  void register_green_tea_button(GreenTeaButton *button) {
    green_tea_button_ = button;
    green_tea_button_->set_parent(this);
  }
  
  void register_oolong_tea_button(OolongTeaButton *button) {
    oolong_tea_button_ = button;
    oolong_tea_button_->set_parent(this);
  }
  
  void register_bag_tea_button(BagTeaButton *button) {
    bag_tea_button_ = button;
    bag_tea_button_->set_parent(this);
  }
  
  void register_boil_button(BoilButton *button) {
    boil_button_ = button;
    boil_button_->set_parent(this);
  }
  
  void register_keep_warm_button(KeepWarmButton *button) {
    keep_warm_button_ = button;
    keep_warm_button_->set_parent(this);
  }
  
  // Геттеры для сенсоров
  float get_current_temperature() const { return current_temp_; }
  float get_target_temperature() const { return target_temperature; }
  bool get_no_kettle() const { return no_kettle_; }
  bool get_no_water() const { return no_water_; }
  std::string get_mode_text() const { return mode_text_; }
  
  // Команды для чайной церемонии
  void black_tea() { send_preset(95, 0x3D); }
  void mix_tea()   { send_preset(40, 0x3B); }
  void white_tea() { send_preset(65, 0x3C); }
  void green_tea() { send_preset(80, 0x7C); }
  void oolong_tea(){ send_preset(90, 0x7D); }
  void bag_tea()   { send_preset(100, 0x7E); }
  void boil()      { send_preset(100, 0x3B); }
  void keep_warm() { send_preset(40, 0x63); }
  void turn_on()   { send_power(0x00, 0x01, 70, 0x3C); }
  void turn_off()  { send_power(0x00, 0x00, 0x00, 0x00); }
  void set_temperature(float temp) { send_target_temperature((uint8_t)temp); }
  
  // Жизненный цикл
  void setup() override {
    ESP_LOGI("polaris", "Polaris Kettle Component Started");
    
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

 protected:
  uart::UARTComponent *parent_{nullptr};
  std::vector<uint8_t> buffer_;
  float current_temp_ = 0.0;
  bool no_kettle_ = false;
  bool no_water_ = false;
  std::string mode_text_ = "Выключен";
  
  PolarisCurrentTemperatureSensor *current_temp_sensor_{nullptr};
  PolarisTargetTemperatureSensor *target_temp_sensor_{nullptr};
  PolarisModeTextSensor *mode_text_sensor_{nullptr};
  PolarisNoKettleBinarySensor *no_kettle_sensor_{nullptr};
  PolarisNoWaterBinarySensor *no_water_sensor_{nullptr};
  
  BlackTeaButton *black_tea_button_{nullptr};
  MixTeaButton *mix_tea_button_{nullptr};
  WhiteTeaButton *white_tea_button_{nullptr};
  GreenTeaButton *green_tea_button_{nullptr};
  OolongTeaButton *oolong_tea_button_{nullptr};
  BagTeaButton *bag_tea_button_{nullptr};
  BoilButton *boil_button_{nullptr};
  KeepWarmButton *keep_warm_button_{nullptr};
  
  void process_frame() {
    if (buffer_.size() < 8) return;
    uint8_t *frame = buffer_.data();
    
    if (frame[0] != 0x81) return;
    
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
    
    if (current_temp_sensor_) current_temp_sensor_->publish_state(current_temp_);
    if (target_temp_sensor_) target_temp_sensor_->publish_state(this->target_temperature);
    
    if (status == 0xFF) {
      no_kettle_ = true;
      no_water_ = (frame[3] == 0x64 && frame[4] == 0x55);
      
      if (no_kettle_sensor_) no_kettle_sensor_->publish_state(no_kettle_);
      if (no_water_sensor_) no_water_sensor_->publish_state(no_water_);
      
      mode_text_ = no_water_ ? "Нет воды" : "Нет чайника";
      this->mode = water_heater::WATER_HEATER_MODE_OFF;
    } else {
      no_kettle_ = false;
      no_water_ = false;
      
      if (no_kettle_sensor_) no_kettle_sensor_->publish_state(false);
      if (no_water_sensor_) no_water_sensor_->publish_state(false);
      
      if (mode == 0x00) {
        this->mode = water_heater::WATER_HEATER_MODE_OFF;
        mode_text_ = "Выключен";
      } else if (mode == 0x01) {
        if (frame[4] == 0x3B || frame[4] == 0x7B) {
          this->mode = water_heater::WATER_HEATER_MODE_PERFORMANCE;
          mode_text_ = "Кипячение";
        } else if (frame[4] == 0x63 || frame[4] == 0x3C || frame[4] == 0x3D) {
          this->mode = water_heater::WATER_HEATER_MODE_ECO;
          mode_text_ = "Подогрев";
        } else {
          this->mode = water_heater::WATER_HEATER_MODE_GAS;
          mode_text_ = "Чайная церемония";
        }
      } else {
        this->mode = water_heater::WATER_HEATER_MODE_GAS;
        mode_text_ = "Чайная церемония";
      }
    }
    
    if (mode_text_sensor_) mode_text_sensor_->publish_state(mode_text_);
    this->publish_state();
    
    ESP_LOGD("polaris", "T: %.1f°C / %.1f°C | %s", current_temp_, this->target_temperature, mode_text_.c_str());
  }
  
  uint16_t calculate_checksum(uint8_t *data, int len) {
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) sum += data[i];
    return sum;
  }
  
  void send_command(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5) {
    uint8_t cmd[8] = {0x81, b1, b2, b3, b4, b5, 0x00, 0x00};
    uint16_t sum = calculate_checksum(cmd, 6);
    cmd[6] = (sum >> 8) & 0xFF;
    cmd[7] = sum & 0xFF;
    this->write_array(cmd, 8);
  }
  
  void send_preset(uint8_t temp, uint8_t mode_byte) {
    send_command(0x00, 0x01, temp, mode_byte, 0x00);
  }
  
  void send_power(uint8_t s, uint8_t m, uint8_t t, uint8_t f) {
    send_command(s, m, t, f, 0x00);
  }
  
  void send_target_temperature(uint8_t temp) {
    send_command(0x00, 0x01, temp, 0x3C, 0x00);
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

// ========== РЕАЛИЗАЦИЯ МЕТОДОВ КНОПОК ==========

void BlackTeaButton::press_action() { if (parent_) parent_->black_tea(); }
void MixTeaButton::press_action() { if (parent_) parent_->mix_tea(); }
void WhiteTeaButton::press_action() { if (parent_) parent_->white_tea(); }
void GreenTeaButton::press_action() { if (parent_) parent_->green_tea(); }
void OolongTeaButton::press_action() { if (parent_) parent_->oolong_tea(); }
void BagTeaButton::press_action() { if (parent_) parent_->bag_tea(); }
void BoilButton::press_action() { if (parent_) parent_->boil(); }
void KeepWarmButton::press_action() { if (parent_) parent_->keep_warm(); }

}  // namespace polaris_kettle
}  // namespace esphome
}  // namespace polaris_kettle
}  // namespace esphome
