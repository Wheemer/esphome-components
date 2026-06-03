#include "rad_sens.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace rad_sens {

static const char *const TAG = "rad_sens";

void RadSensComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RadSens...");
  
  // Проверяем наличие устройства через чтение регистра
  uint8_t test_read = this->read_register_8_(REG_DEVICE_ID);
  if (test_read == 0 || test_read == 0xFF) {
    ESP_LOGE(TAG, "RadSens not found at I2C address 0x%02X", this->address_);
    this->mark_failed();
    return;
  }
  
  this->device_id_ = test_read;
  this->firmware_version_ = this->read_register_8_(REG_FIRMWARE_VER);
  
  ESP_LOGCONFIG(TAG, "  Device ID: 0x%02X", this->device_id_);
  ESP_LOGCONFIG(TAG, "  Firmware Version: %d", this->firmware_version_);
  
  if (this->device_id_ == 0 || this->device_id_ == 0xFF) {
    ESP_LOGE(TAG, "Invalid device ID");
    this->mark_failed();
    return;
  }
  
  // Читаем текущую чувствительность
  this->sensitivity_ = this->read_register_16_(REG_SENSITIVITY);
  ESP_LOGCONFIG(TAG, "  Current Sensitivity: %d imp/µR", this->sensitivity_);
  
  // Инициализация состояний
  this->hv_generator_state_ = this->get_hv_generator_state();
  this->hv_generator_initialized_ = true;
  
  this->led_state_ = false;
  this->led_initialized_ = true;
  
  this->low_power_state_ = false;
  this->low_power_initialized_ = true;
  
  // Публикация начальных состояний для switch
  if (this->hv_generator_switch_ != nullptr) {
    this->hv_generator_switch_->publish_state(this->hv_generator_state_);
  }
  
  if (this->led_switch_ != nullptr) {
    this->led_switch_->publish_state(this->led_state_);
  }
  
  if (this->low_power_switch_ != nullptr) {
    this->low_power_switch_->publish_state(this->low_power_state_);
  }
  
  if (this->sensitivity_number_ != nullptr) {
    this->sensitivity_number_->publish_state(static_cast<float>(this->sensitivity_));
    this->sensitivity_initialized_ = true;
  }
  
  this->initialized_ = true;
  
  ESP_LOGI(TAG, "RadSens initialized successfully");
  ESP_LOGI(TAG, "HV Generator: %s", this->hv_generator_state_ ? "ON" : "OFF");
  ESP_LOGI(TAG, "Sensitivity: %d imp/µR", this->sensitivity_);
}

void RadSensComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "RadSens:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Device ID: 0x%02X", this->device_id_);
  ESP_LOGCONFIG(TAG, "  Firmware Version: %d", this->firmware_version_);
  ESP_LOGCONFIG(TAG, "  Sensitivity: %d imp/µR", this->sensitivity_);
  ESP_LOGCONFIG(TAG, "  Update Interval: %u ms", this->update_interval_);
  ESP_LOGCONFIG(TAG, "  HV Generator State: %s", this->hv_generator_state_ ? "ON" : "OFF");
  
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Communication failed!");
  }
}

void RadSensComponent::loop() {
  if (this->is_failed()) {
    return;
  }
  
  uint32_t now = millis();
  if (now - this->last_update_ < this->update_interval_) {
    return;
  }
  
  this->last_update_ = now;
  this->update_sensors_();
}

void RadSensComponent::update_sensors_() {
  // Обновление состояния HV генератора
  bool new_hv_state = this->get_hv_generator_state();
  if (this->hv_generator_initialized_ && new_hv_state != this->hv_generator_state_) {
    this->hv_generator_state_ = new_hv_state;
    
    if (this->hv_generator_state_sensor_ != nullptr) {
      this->hv_generator_state_sensor_->publish_state(this->hv_generator_state_);
    }
    
    if (this->hv_generator_switch_ != nullptr) {
      this->hv_generator_switch_->publish_state(this->hv_generator_state_);
    }
    
    ESP_LOGD(TAG, "HV Generator state changed to: %s", this->hv_generator_state_ ? "ON" : "OFF");
  }
  
  // Обновление чувствительности
  uint16_t new_sensitivity = this->read_register_16_(REG_SENSITIVITY);
  if (new_sensitivity != this->sensitivity_) {
    this->sensitivity_ = new_sensitivity;
    
    if (this->sensitivity_number_ != nullptr && this->sensitivity_initialized_) {
      this->sensitivity_number_->publish_state(static_cast<float>(this->sensitivity_));
    }
    
    ESP_LOGD(TAG, "Sensitivity changed to: %d imp/µR", this->sensitivity_);
  }
  
  // Чтение датчиков
  uint16_t dyn_intensity_raw = this->read_register_16_(REG_DYNAMIC_INTENSITY_LOW);
  float dynamic_intensity = dyn_intensity_raw * 0.1f;
  
  if (this->dynamic_intensity_sensor_ != nullptr) {
    this->dynamic_intensity_sensor_->publish_state(dynamic_intensity);
  }
  
  uint16_t stat_intensity_raw = this->read_register_16_(REG_STATIC_INTENSITY_LOW);
  float static_intensity = stat_intensity_raw * 0.1f;
  
  if (this->static_intensity_sensor_ != nullptr) {
    this->static_intensity_sensor_->publish_state(static_intensity);
  }
  
  uint32_t pulses = this->read_register_32_(REG_PULSE_COUNT_LOW);
  
  if (this->pulses_sensor_ != nullptr) {
    this->pulses_sensor_->publish_state(pulses);
  }
  
  ESP_LOGD(TAG, "Dynamic: %.1f µR/h, Static: %.1f µR/h, Pulses: %lu, HV: %s, Sens: %d imp/µR",
           dynamic_intensity, static_intensity, pulses,
           this->hv_generator_state_ ? "ON" : "OFF",
           this->sensitivity_);
}

void RadSensComponent::set_hv_generator(bool state) {
  uint8_t value = state ? HV_GENERATOR_ON : HV_GENERATOR_OFF;
  if (this->write_register_8_(REG_HV_GENERATOR_CONTROL, value)) {
    this->hv_generator_state_ = state;
    ESP_LOGI(TAG, "HV Generator set to: %s", state ? "ON" : "OFF");
  } else {
    ESP_LOGE(TAG, "Failed to set HV Generator state");
  }
}

bool RadSensComponent::get_hv_generator_state() {
  uint8_t state = this->read_register_8_(REG_HV_GENERATOR_STATE);
  return (state > 0);
}

void RadSensComponent::set_led(bool state) {
  this->led_state_ = state;
  ESP_LOGI(TAG, "LED set to: %s", state ? "ON" : "OFF");
}

bool RadSensComponent::get_led_state() {
  return this->led_state_;
}

void RadSensComponent::set_low_power(bool state) {
  this->low_power_state_ = state;
  ESP_LOGI(TAG, "Low Power mode set to: %s", state ? "ON" : "OFF");
}

bool RadSensComponent::get_low_power_state() {
  return this->low_power_state_;
}

void RadSensComponent::set_sensitivity(uint16_t sensitivity) {
  if (sensitivity < 100 || sensitivity > 1100) {
    ESP_LOGW(TAG, "Sensitivity %d out of range (100-1100)", sensitivity);
    return;
  }
  
  if (this->write_register_16_(REG_SENSITIVITY, sensitivity)) {
    this->sensitivity_ = sensitivity;
    ESP_LOGI(TAG, "Sensitivity set to: %d imp/µR", sensitivity);
  } else {
    ESP_LOGE(TAG, "Failed to set sensitivity");
  }
}

bool RadSensComponent::check_device_ready() {
  uint8_t test = this->read_register_8_(REG_DEVICE_ID);
  return (test != 0 && test != 0xFF);
}

uint8_t RadSensComponent::read_register_8_(uint8_t reg) {
  uint8_t value = 0;
  if (this->read_byte(reg, &value)) {
    return value;
  }
  return 0;
}

uint16_t RadSensComponent::read_register_16_(uint8_t reg) {
  uint8_t data[2] = {0, 0};
  if (this->read_bytes(reg, data, 2)) {
    return (uint16_t(data[1]) << 8) | data[0];
  }
  return 0;
}

uint32_t RadSensComponent::read_register_32_(uint8_t reg) {
  uint8_t data[4] = {0, 0, 0, 0};
  if (this->read_bytes(reg, data, 4)) {
    return (uint32_t(data[3]) << 24) | (uint32_t(data[2]) << 16) | 
           (uint32_t(data[1]) << 8) | data[0];
  }
  return 0;
}

bool RadSensComponent::write_register_8_(uint8_t reg, uint8_t value) {
  return this->write_byte(reg, value);
}

bool RadSensComponent::write_register_16_(uint8_t reg, uint16_t value) {
  uint8_t data[2] = {uint8_t(value & 0xFF), uint8_t(value >> 8)};
  return this->write_bytes(reg, data, 2);
}

}  // namespace rad_sens
}  // namespace esphome
