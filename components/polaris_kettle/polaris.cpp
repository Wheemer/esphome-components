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
    this->target_temperature_ = target_opt.value();
    this->set_target_temperature(this->target_temperature_);
    updated = true;
  }
  
  auto mode_opt = call.get_mode();
  if (mode_opt.has_value()) {
    switch (mode_opt.value()) {
      case water_heater::WATER_HEATER_MODE_OFF:
        this->turn_off();
        break;
      case water_heater::WATER_HEATER_MODE_PERFORMANCE:
        this->boil();
        break;
      case water_heater::WATER_HEATER_MODE_ECO:
        this->keep_warm();
        break;
      default:
        this->set_target_temperature(this->target_temperature_);
        break;
    }
    updated = true;
  }
  
  if (updated) {
    this->publish_state();
  }
}

void PolarisKettle::set_target_temperature(float temp) {
  this->send_target_temperature_command((uint8_t)temp);
  ESP_LOGI(TAG, "Set target temperature: %.1f°C", temp);
}

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
    ESP_LOGW(TAG, "Checksum error: calc=0x%04X recv=0x%04X", calc_sum, recv_sum);
    return;
  }
  
  uint8_t status = frame[1];
  uint8_t mode = frame[2];
  uint8_t target_temp = frame[3];
  uint8_t current_temp = frame[4];
  
  this->current_temp_ = (float)current_temp;
  this->target_temperature_ = (float)target_temp;
  
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
  
  ESP_LOGD(TAG, "Current: %.1f°C, Target: %.1f°C, Mode: %s, Error: %s",
           this->current_temp_, this->target_temperature_, this->mode_text_.c_str(),
           this->no_kettle_ ? (this->no_water_ ? "NO_WATER" : "NO_KETTLE") : "OK");
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
  
  ESP_LOGD(TAG, "Send: %02X %02X %02X %02X %02X %02X %02X %02X",
           cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
}

void PolarisKettle::send_preset(uint8_t temp, uint8_t mode_byte) {
  this->send_command(0x00, 0x01, temp, mode_byte, 0x00);
  ESP_LOGI(TAG, "Send preset: %d°C, mode=0x%02X", temp, mode_byte);
}

void PolarisKettle::send_target_temperature_command(uint8_t temp) {
  this->send_command(0x00, 0x01, temp, 0x3C, 0x00);
}

}  // namespace polaris
}  // namespace esphome
