#include "polaris_kettle.h"

namespace esphome {
namespace polaris_kettle {

void PolarisKettle::setup() {
  ESP_LOGI("polaris", "Polaris Kettle Component Started");
  
  traits_.set_supports_current_temperature(true);
  traits_.set_supports_target_temperature(true);
  traits_.set_supports_away_mode(false);
  traits_.set_min_temperature(40.0);
  traits_.set_max_temperature(100.0);
  traits_.set_target_temperature_step(1.0);
  traits_.set_supported_modes({
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

void PolarisKettle::control(const water_heater::WaterHeaterCall &call) {
  bool updated = false;
  
  if (call.get_target_temperature().has_value()) {
    float new_temp = *call.get_target_temperature();
    target_temperature = new_temp;
    send_target_temperature((uint8_t)new_temp);
    if (target_sensor_) target_sensor_->publish(new_temp);
    updated = true;
  }
  
  if (call.get_mode().has_value()) {
    mode = *call.get_mode();
    send_mode(mode);
    updated = true;
  }
  
  if (updated) {
    publish_state();
  }
}

void PolarisKettle::publish_state() {
  current_temperature = current_temp_;
  water_heater::WaterHeater::publish_state();
}

void PolarisKettle::update_sensors() {
  if (current_sensor_) current_sensor_->publish(current_temp_);
  if (target_sensor_) target_sensor_->publish(target_temperature);
  if (mode_text_sensor_) mode_text_sensor_->publish(mode_text_);
  if (no_kettle_sensor_) no_kettle_sensor_->publish(no_kettle_);
  if (no_water_sensor_) no_water_sensor_->publish(no_water_);
}

void PolarisKettle::register_button(PolarisButton *button, const std::string &type) {
  button->set_parent(this);
  button->set_type(type);
}

void PolarisButton::press_action() {
  if (!parent_) return;
  if (type_ == "black_tea") parent_->black_tea();
  else if (type_ == "mix_tea") parent_->mix_tea();
  else if (type_ == "white_tea") parent_->white_tea();
  else if (type_ == "green_tea") parent_->green_tea();
  else if (type_ == "oolong_tea") parent_->oolong_tea();
  else if (type_ == "bag_tea") parent_->bag_tea();
  else if (type_ == "boil") parent_->boil();
  else if (type_ == "keep_warm") parent_->keep_warm();
}

void PolarisKettle::process_frame() {
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
  uint8_t mod = frame[2];
  uint8_t target_temp = frame[3];
  uint8_t current_temp = frame[4];
  
  current_temp_ = (float)current_temp;
  target_temperature = (float)target_temp;
  
  if (status == 0xFF) {
    no_kettle_ = true;
    no_water_ = (frame[3] == 0x64 && frame[4] == 0x55);
    mode_text_ = no_water_ ? "Нет воды" : "Нет чайника";
    mode = water_heater::WATER_HEATER_MODE_OFF;
  } else {
    no_kettle_ = false;
    no_water_ = false;
    
    if (mod == 0x00) {
      mode = water_heater::WATER_HEATER_MODE_OFF;
      mode_text_ = "Выключен";
    } else if (mod == 0x01) {
      if (frame[4] == 0x3B || frame[4] == 0x7B) {
        mode = water_heater::WATER_HEATER_MODE_PERFORMANCE;
        mode_text_ = "Кипячение";
      } else if (frame[4] == 0x63 || frame[4] == 0x3C || frame[4] == 0x3D) {
        mode = water_heater::WATER_HEATER_MODE_ECO;
        mode_text_ = "Подогрев";
      } else {
        mode = water_heater::WATER_HEATER_MODE_GAS;
        mode_text_ = "Чайная церемония";
      }
    } else {
      mode = water_heater::WATER_HEATER_MODE_GAS;
      mode_text_ = "Чайная церемония";
    }
  }
  
  publish_state();
  update_sensors();
  
  ESP_LOGD("polaris", "T: %.1f°C / %.1f°C | %s", 
           current_temp_, target_temperature, mode_text_.c_str());
}

uint16_t PolarisKettle::calculate_checksum(uint8_t *data, int len) {
  uint16_t sum = 0;
  for (int i = 0; i < len; i++) sum += data[i];
  return sum;
}

void PolarisKettle::send_command(uint8_t b1, uint8_t b2, uint8_t b3, 
                                  uint8_t b4, uint8_t b5) {
  uint8_t cmd[8] = {0x81, b1, b2, b3, b4, b5, 0x00, 0x00};
  uint16_t sum = calculate_checksum(cmd, 6);
  cmd[6] = (sum >> 8) & 0xFF;
  cmd[7] = sum & 0xFF;
  write_array(cmd, 8);
}

void PolarisKettle::send_preset(uint8_t temp, uint8_t mode_byte) {
  send_command(0x00, 0x01, temp, mode_byte, 0x00);
  ESP_LOGI("polaris", "Preset: %d°C 0x%02X", temp, mode_byte);
}

void PolarisKettle::send_target_temperature(uint8_t temp) {
  send_command(0x00, 0x01, temp, 0x3C, 0x00);
}

void PolarisKettle::send_mode(water_heater::WaterHeaterMode m) {
  switch (m) {
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
      send_preset((uint8_t)target_temperature, 0x3C);
      break;
  }
}

}  // namespace polaris_kettle
}  // namespace esphome
