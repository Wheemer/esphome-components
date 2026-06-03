#include "cg_anem.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace cg_anem {

static const char *const TAG = "cg_anem.sensor";

// Регистры
static const uint8_t CG_ANEM_REGISTER_VERSION = 0x04;
static const uint8_t CG_ANEM_REGISTER_WHO_I_AM = 0x05;
static const uint8_t CG_ANEM_REGISTER_STATUS = 0x06;
static const uint8_t CG_ANEM_REGISTER_WIND_H = 0x07;
static const uint8_t CG_ANEM_REGISTER_WIND_L = 0x08;
static const uint8_t CG_ANEM_REGISTER_COLD_H = 0x10;
static const uint8_t CG_ANEM_REGISTER_COLD_L = 0x11;
static const uint8_t CG_ANEM_REGISTER_HOT_H = 0x12;
static const uint8_t CG_ANEM_REGISTER_HOT_L = 0x13;
static const uint8_t CG_ANEM_REGISTER_WIND_MAX_H = 0x21;
static const uint8_t CG_ANEM_REGISTER_WIND_MAX_L = 0x22;
static const uint8_t CG_ANEM_REGISTER_WIND_MIN_H = 0x23;
static const uint8_t CG_ANEM_REGISTER_WIND_MIN_L = 0x24;
static const uint8_t CG_ANEM_REGISTER_HEAT_WT = 0x0E;
static const uint8_t CG_ANEM_REGISTER_SUPPLY_V = 0x0D;

// Биты статуса
static const uint8_t CG_ANEM_STATUS_INCORRECT_TARING_RANGE = 0b10000000;
static const uint8_t CG_ANEM_STATUS_INCORRECT_TARING = 0b01000000;
static const uint8_t CG_ANEM_STATUS_WATCHDOG_TIMER = 0b00100000;
static const uint8_t CG_ANEM_STATUS_OVERVOLTAGE = 0b00000010;
static const uint8_t CG_ANEM_STATUS_UNSTEADY_PROCESS = 0b00000001;

inline uint16_t combine_bytes(uint8_t msb, uint8_t lsb) { 
  return (msb << 8) | lsb; 
}

bool CGAnemComponent::read_uint16(uint8_t reg_h, uint8_t reg_l, uint16_t &value) {
  uint8_t msb, lsb;
  if (!this->read_byte(reg_h, &msb)) return false;
  if (!this->read_byte(reg_l, &lsb)) return false;
  value = combine_bytes(msb, lsb);
  return true;
}

void CGAnemComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CG Anem...");

  // Сброс failed состояния
  if ((this->component_state_ & COMPONENT_STATE_MASK) == COMPONENT_STATE_FAILED) {
    this->component_state_ &= ~COMPONENT_STATE_MASK;
    this->component_state_ |= COMPONENT_STATE_CONSTRUCTION;
  }
  
  // Чтение версии
  uint8_t versionRaw = 0;
  if (!this->read_byte(CG_ANEM_REGISTER_VERSION, &versionRaw)) {
    ESP_LOGE(TAG, "Failed to read version register");
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  
  version_ = versionRaw / 10.0f;
  ESP_LOGI(TAG, "Firmware version: %.1f", version_);
  
  if (this->firmware_version_sensor_ != nullptr)
    this->firmware_version_sensor_->publish_state(version_);
  
  // Проверка связи
  if (!this->write_byte(CG_ANEM_REGISTER_WHO_I_AM, 0x11)) {
    ESP_LOGE(TAG, "Communication test failed");
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  
  ESP_LOGI(TAG, "CG Anem setup complete, duct area: %.2f cm²", duct_);
}

void CGAnemComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "CG Anem:");
  LOG_I2C_DEVICE(this);
  
  switch (this->error_code_) {
    case COMMUNICATION_FAILED:
      ESP_LOGE(TAG, "Communication with CG Anem failed!");
      break;
    case NONE:
    default:
      break;
  }

  ESP_LOGCONFIG(TAG, "  Duct area: %.2f cm²", duct_);
  ESP_LOGCONFIG(TAG, "  Firmware version: %.1f", version_);
  LOG_UPDATE_INTERVAL(this);

  LOG_SENSOR("  ", "Ambient temperature", this->ambient_temperature_sensor_);
  LOG_SENSOR("  ", "Hotend temperature", this->hotend_temperature_sensor_);
  LOG_SENSOR("  ", "Heat power", this->heat_power_sensor_);
  LOG_SENSOR("  ", "Air consumption", this->air_consumption_sensor_);
  LOG_SENSOR("  ", "Air flow rate", this->air_flow_rate_sensor_);
  LOG_SENSOR("  ", "Min air flow rate", this->min_air_flow_rate_sensor_);
  LOG_SENSOR("  ", "Max air flow rate", this->max_air_flow_rate_sensor_);
}

void CGAnemComponent::read_status() {
  uint8_t status;
  if (!this->read_byte(CG_ANEM_REGISTER_STATUS, &status)) {
    ESP_LOGW(TAG, "Error reading status register");
    this->status_set_warning();
    return;
  }

  // Обработка статусов с публикацией в бинарные сенсоры
  bool unsteady = status & CG_ANEM_STATUS_UNSTEADY_PROCESS;
  bool overvoltage = status & CG_ANEM_STATUS_OVERVOLTAGE;
  bool watchdog = (status & CG_ANEM_STATUS_WATCHDOG_TIMER) == 0;

  if (unsteady) {
    ESP_LOGW(TAG, "Unsteady process detected");
    this->status_set_warning();
  } else {
    this->status_clear_warning();
  }

  if (overvoltage) {
    ESP_LOGW(TAG, "Overvoltage detected");
  }

  if (!watchdog) {
    ESP_LOGW(TAG, "Watchdog disabled");
  }

  // Публикация в бинарные сенсоры
  if (this->status_up_binary_sensor_ != nullptr)
    this->status_up_binary_sensor_->publish_state(unsteady);
  
  if (this->status_ov_binary_sensor_ != nullptr)
    this->status_ov_binary_sensor_->publish_state(overvoltage);
  
  if (this->status_wdt_binary_sensor_ != nullptr)
    this->status_wdt_binary_sensor_->publish_state(watchdog);

  // Дополнительные проверки
  if (status & CG_ANEM_STATUS_INCORRECT_TARING_RANGE) {
    ESP_LOGW(TAG, "Incorrect taring range detected");
  }

  if (status & CG_ANEM_STATUS_INCORRECT_TARING) {
    ESP_LOGW(TAG, "Incorrect taring detected");
  }
}

float CGAnemComponent::calculate_air_consumption(float speed) {
  if (duct_ <= 0.01f || speed <= -254.0f) {  // -255 - признак ошибки
    return -255.0f;
  }
  // Формула: скорость (м/с) * площадь (см²) * 0.36 = м³/ч
  return speed * duct_ * 0.36f;
}

void CGAnemComponent::update() {
  this->read_status();

  if (this->status_has_warning()) {
    ESP_LOGV(TAG, "Skipping update due to warning status");
    return;
  }

  uint16_t temp_raw, hot_raw, speed_raw, min_raw = 0, max_raw = 0;
  uint8_t power_raw, supply_raw;

  // Чтение температуры холодного спая
  if (!read_uint16(CG_ANEM_REGISTER_COLD_H, CG_ANEM_REGISTER_COLD_L, temp_raw)) {
    ESP_LOGW(TAG, "Failed to read ambient temperature");
    this->status_set_warning();
    return;
  }
  float temp = temp_raw / 10.0f;

  // Чтение температуры горячего спая
  if (!read_uint16(CG_ANEM_REGISTER_HOT_H, CG_ANEM_REGISTER_HOT_L, hot_raw)) {
    ESP_LOGW(TAG, "Failed to read hotend temperature");
    this->status_set_warning();
    return;
  }
  float hot = hot_raw / 10.0f;

  // Чтение скорости потока
  if (!read_uint16(CG_ANEM_REGISTER_WIND_H, CG_ANEM_REGISTER_WIND_L, speed_raw)) {
    ESP_LOGW(TAG, "Failed to read wind speed");
    this->status_set_warning();
    return;
  }
  float speed = speed_raw / 10.0f;

  // Чтение Min/Max (только для версии >= 1.0)
  float min_air = -255.0f;
  float max_air = -255.0f;
  
  if (version_ >= 1.0f) {
    if (read_uint16(CG_ANEM_REGISTER_WIND_MIN_H, CG_ANEM_REGISTER_WIND_MIN_L, min_raw)) {
      min_air = min_raw / 10.0f;
    } else {
      ESP_LOGW(TAG, "Failed to read min air flow rate");
    }
    
    if (read_uint16(CG_ANEM_REGISTER_WIND_MAX_H, CG_ANEM_REGISTER_WIND_MAX_L, max_raw)) {
      max_air = max_raw / 10.0f;
    } else {
      ESP_LOGW(TAG, "Failed to read max air flow rate");
    }
  } else {
    ESP_LOGV(TAG, "Min/Max air flow not available for firmware < 1.0");
  }

  // Чтение напряжения питания
  if (!this->read_byte(CG_ANEM_REGISTER_SUPPLY_V, &supply_raw)) {
    ESP_LOGW(TAG, "Failed to read supply voltage");
  } else {
    float supply_v = supply_raw / 10.0f;
    ESP_LOGV(TAG, "Supply voltage: %.1f V", supply_v);
  }

  // Чтение мощности нагрева
  if (!this->read_byte(CG_ANEM_REGISTER_HEAT_WT, &power_raw)) {
    ESP_LOGW(TAG, "Failed to read heat power");
    this->status_set_warning();
    return;
  }
  
  float power;
  if (version_ >= 1.0f) {
    // Для версии 1.0+ другая формула
    power = power_raw / 255.0f * (4.7f * 4.7f / 8.0f);
  } else {
    power = power_raw / 10.0f;
  }

  // Расчет расхода воздуха
  float consumption = calculate_air_consumption(speed);

  // Публикация всех сенсоров
  if (this->ambient_temperature_sensor_ != nullptr)
    this->ambient_temperature_sensor_->publish_state(temp);
  
  if (this->hotend_temperature_sensor_ != nullptr)
    this->hotend_temperature_sensor_->publish_state(hot);
  
  if (this->air_flow_rate_sensor_ != nullptr)
    this->air_flow_rate_sensor_->publish_state(speed);
  
  if (this->heat_power_sensor_ != nullptr)
    this->heat_power_sensor_->publish_state(power);
  
  if (this->air_consumption_sensor_ != nullptr)
    this->air_consumption_sensor_->publish_state(consumption);
  
  if (this->min_air_flow_rate_sensor_ != nullptr && min_air > -254.0f)
    this->min_air_flow_rate_sensor_->publish_state(min_air);
  
  if (this->max_air_flow_rate_sensor_ != nullptr && max_air > -254.0f)
    this->max_air_flow_rate_sensor_->publish_state(max_air);

  ESP_LOGV(TAG, "Update: T=%.1f°C, Hot=%.1f°C, Speed=%.1fm/s, Power=%.3fW, Consumption=%.3fm³/h", 
           temp, hot, speed, power, consumption);
}

}  // namespace cg_anem
}  // namespace esphome
