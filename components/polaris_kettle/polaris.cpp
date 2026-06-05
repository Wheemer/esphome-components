#include "polaris.h"

namespace esphome {
namespace polaris {

static const char *const TAG = "polaris";

void PolarisKettle::setup() {
  ESP_LOGI(TAG, "Polaris Kettle Component Started");
  
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

void PolarisKettle::loop() {
  while (this->available()) {
    uint8_t data;
    this->read_byte(&data);
    this->buffer_.push_back(data);
    
    if (this->buffer_.size() == 1 && this->buffer_[0] != 0x81) {
      this->buffer_.clear();
      continue;
    }
    
    if (this->buffer_.size() >= 8) {
      this->process_frame();
      this->buffer_.clear();
    }
  }
}

void PolarisKettle::control(const water_heater::WaterHeaterCall &call) {
  bool updated = false;
  
  auto target_opt = call.get_target_temperature();
  if (target_opt.has_value()) {
    this->target_temp_ = target_opt.value();
    this->send_preset((uint8_t)this->target_temp_, 0x3C);
    updated = true;
  }
  
  auto mode_opt = call.get_mode();
  if (mode_opt.has_value()) {
    switch (mode_opt.value()) {
      case water_heater::WATER_HEATER_MODE_OFF:
        this->send_command(0x00, 0x00, 0x00, 0x00, 0x00);
        break;
      case water_heater::WATER_HEATER_MODE_PERFORMANCE:
        this->boil();
        break;
      case water_heater::WATER_HEATER_MODE_ECO:
        this->keep_warm();
        break;
      default:
        this->send_preset((uint8_t)this->target_temp_, 0x3C);
        break;
    }
    updated = true;
  }
  
  if (updated) {
    this->publish_state();
  }
}

void PolarisKettle::update_sensors() {
  if (this->current_temperature_sensor_) {
    this->current_temperature_sensor_->publish_state(this->current_temp_);
  }
  if (this->target_temperature_sensor_) {
    this->target_temperature_sensor_->publish_state(this->target_temp_);
  }
  if (this->mode_text_sensor_) {
    this->mode_text_sensor_->publish_state(this->mode_text_);
  }
  if (this->no_kettle_sensor_) {
    this->no_kettle_sensor_->publish_state(this->no_kettle_);
  }
  if (this->no_water_sensor_) {
    this->no_water_sensor_->publish_state(this->no_water_);
  }
}

void PolarisKettle::black_tea() {
  this->send_preset(95, 0x3D);
  ESP_LOGI(TAG, "Black tea mode: 95°C");
}

void PolarisKettle::mix_tea() {
  this->send_preset(40, 0x3B);
  ESP_LOGI(TAG, "Mix tea mode: 40°C");
}

void PolarisKettle::white_tea() {
  this->send_preset(65, 0x3C);
  ESP_LOGI(TAG, "White tea mode: 65°C");
}

void PolarisKettle::green_tea() {
  this->send_preset(80, 0x7C);
  ESP_LOGI(TAG, "Green tea mode: 80°C");
}

void PolarisKettle::oolong_tea() {
  this->send_preset(90, 0x7D);
  ESP_LOGI(TAG, "Oolong tea mode: 90°C");
}

void PolarisKettle::bag_tea() {
  this->send_preset(100, 0x7E);
  ESP_LOGI(TAG, "Bag tea mode: 100°C");
}

void PolarisKettle::boil() {
  this->send_preset(100, 0x3B);
  ESP_LOGI(TAG, "Boil mode");
}

void PolarisKettle::keep_warm() {
  this->send_preset(40, 0x63);
  ESP_LOGI(TAG, "Keep warm mode");
}

void PolarisKettle::set_black_tea_button(button::Button *button) {
  this->black_tea_button_ = button;
  // Кнопка будет вызывать black_tea() при нажатии через отдельный класс
}

// Аналогичные методы для других кнопок...

void PolarisKettle::dump_config() {
  ESP_LOGCONFIG(TAG, "Polaris Kettle:");
  ESP_LOGCONFIG(TAG, "  Sync mode: %s", YESNO(this->sync_mode_));
}

void PolarisKettle::process_frame() {
  if (this->buffer_.size() < 8) return;
  uint8_t *frame = this->buffer_.data();
  
  if (frame[0] != 0x81) return;
  
  uint16_t calc_sum = 0;
  for (int i = 0; i < 6; i++) calc_sum += frame[i];
  uint16_t recv_sum = (frame[6] << 8) | frame[7];
  
  if (calc_sum != recv_sum) {
    ESP_LOGW(TAG, "Checksum error");
    return;
  }
  
  uint8_t status = frame[1];
  uint8_t mode = frame[2];
  uint8_t target = frame[3];
  uint8_t current = frame[4];
  
  this->current_temp_ = (float)current;
  this->target_temp_ = (float)target;
  
  if (status == 0xFF) {
    this->no_kettle_ = true;
    this->no_water_ = (frame[3] == 0x64 && frame[4] == 0x55);
    this->mode_text_ = this->no_water_ ? "Нет воды" : "Нет чайника";
    this->mode = water_heater::WATER_HEATER_MODE_OFF;
  } else {
    this->no_kettle_ = false;
    this->no_water_ = false;
    
    if (mode == 0x00) {
      this->mode = water_heater::WATER_HEATER_MODE_OFF;
      this->mode_text_ = "Выключен";
    } else if (mode == 0x01) {
      if (frame[4] == 0x3B || frame[4] == 0x7B) {
        this->mode = water_heater::WATER_HEATER_MODE_PERFORMANCE;
        this->mode_text_ = "Кипячение";
      } else if (frame[4] == 0x63 || frame[4] == 0x3C || frame[4] == 0x3D) {
        this->mode = water_heater::WATER_HEATER_MODE_ECO;
        this->mode_text_ = "Подогрев";
      } else {
        this->mode = water_heater::WATER_HEATER_MODE_GAS;
        this->mode_text_ = "Чайная церемония";
      }
    } else {
      this->mode = water_heater::WATER_HEATER_MODE_GAS;
      this->mode_text_ = "Чайная церемония";
    }
  }
  
  this->publish_state();
  this->update_sensors();
  
  ESP_LOGD(TAG, "Current: %.1f°C, Target: %.1f°C, Mode: %s",
           this->current_temp_, this->target_temp_, this->mode_text_.c_str());
}

uint16_t PolarisKettle::calculate_checksum(uint8_t *data, int len) {
  uint16_t sum = 0;
  for (int i = 0; i < len; i++) sum += data[i];
  return sum;
}

void PolarisKettle::send_command(uint8_t b1, uint8_t b2, uint8_t b3, 
                                  uint8_t b4, uint8_t b5) {
  uint8_t cmd[8] = {0x81, b1, b2, b3, b4, b5, 0x00, 0x00};
  uint16_t sum = this->calculate_checksum(cmd, 6);
  cmd[6] = (sum >> 8) & 0xFF;
  cmd[7] = sum & 0xFF;
  this->write_array(cmd, 8);
}

void PolarisKettle::send_preset(uint8_t temp, uint8_t mode_byte) {
  this->send_command(0x00, 0x01, temp, mode_byte, 0x00);
}

}  // namespace polaris
}  // namespace esphome
