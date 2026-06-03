#include "cg_anem.h"
#include "esphome/core/log.h"

namespace esphome {
namespace cg_anem {

static const char *const TAG = "cg_anem.sensor";

// Регистры устройства
static const uint8_t I2C_REG_VERSION = 0x04;
static const uint8_t I2C_REG_WHO_I_AM = 0x05;
static const uint8_t I2C_REG_STATUS = 0x06;
static const uint8_t I2C_REG_WIND_H = 0x07;
static const uint8_t I2C_REG_WIND_L = 0x08;
static const uint8_t I2C_REG_SUPPLY_V = 0x0D;
static const uint8_t I2C_REG_PWR_WT = 0x0E;
static const uint8_t I2C_REG_TEMP_COLD_H = 0x10;
static const uint8_t I2C_REG_TEMP_COLD_L = 0x11;
static const uint8_t I2C_REG_TEMP_HOT_H = 0x12;
static const uint8_t I2C_REG_TEMP_HOT_L = 0x13;
static const uint8_t I2C_REG_DT_H = 0x14;
static const uint8_t I2C_REG_DT_L = 0x15;
static const uint8_t I2C_REG_WIND_MAX_H = 0x21;
static const uint8_t I2C_REG_WIND_MAX_L = 0x22;
static const uint8_t I2C_REG_WIND_MIN_H = 0x23;
static const uint8_t I2C_REG_WIND_MIN_L = 0x24;

// Биты статуса
static const uint8_t STATUS_UNSTEADY_PROCESS = 0x01;
static const uint8_t STATUS_OVERVOLTAGE = 0x02;
static const uint8_t STATUS_WATCHDOG_TIMER = 0x20;
static const uint8_t STATUS_INCORRECT_TARING = 0x40;
static const uint8_t STATUS_INCORRECT_TARING_RANGE = 0x80;

void CGAnemComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CG Anem sensor...");

  if (sleep_pin_ != nullptr) {
    sleep_pin_->setup();
    sleep_pin_->digital_write(false);
    ESP_LOGI(TAG, "Sleep pin configured: enable_sleep=%s", enable_sleep_ ? "true" : "false");
  }

  // Проверка связи
  uint8_t version_raw = 0;
  if (!this->read_byte(I2C_REG_VERSION, &version_raw)) {
    ESP_LOGE(TAG, "Failed to communicate with CG Anem");
    this->mark_failed();
    return;
  }

  firmware_version_ = version_raw / 10.0f;
  ESP_LOGI(TAG, "CG Anem firmware version: %.1f", firmware_version_);
  
  if (firmware_version_sensor_ != nullptr) {
    firmware_version_sensor_->publish_state(firmware_version_);
  }

  // Логирование информации о диаметре
  if (diameter_mm_ > 0) {
    ESP_LOGI(TAG, "Duct diameter: %.1f mm, Calculated area: %.1f cm²", 
             diameter_mm_, duct_area_cm2_);
  }

  ESP_LOGI(TAG, "CG Anem sensor initialized");
}

void CGAnemComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "CG Anem Sensor:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Firmware version: %.1f", firmware_version_);
  
  if (diameter_mm_ > 0) {
    ESP_LOGCONFIG(TAG, "  Duct diameter: %.1f mm", diameter_mm_);
    ESP_LOGCONFIG(TAG, "  Calculated duct area: %.1f cm²", duct_area_cm2_);
  }
  
  if (sleep_pin_ != nullptr) {
    // ИСПРАВЛЕНИЕ: Убрали вызов несуществующего метода get_pin()
    ESP_LOGCONFIG(TAG, "  Sleep pin: configured, Enabled: %s", enable_sleep_ ? "yes" : "no");
  }
  
  LOG_UPDATE_INTERVAL(this);

  LOG_SENSOR("  ", "Ambient temperature", ambient_temperature_sensor_);
  LOG_SENSOR("  ", "Hotend temperature", hotend_temperature_sensor_);
  LOG_SENSOR("  ", "Temperature difference", temperature_difference_sensor_);
  LOG_SENSOR("  ", "Heat power", heat_power_sensor_);
  LOG_SENSOR("  ", "Air flow rate", air_flow_rate_sensor_);
  LOG_SENSOR("  ", "Air consumption", air_consumption_sensor_);
  LOG_SENSOR("  ", "Min air flow rate", min_air_flow_rate_sensor_);
  LOG_SENSOR("  ", "Max air flow rate", max_air_flow_rate_sensor_);
  LOG_SENSOR("  ", "Supply voltage", supply_voltage_sensor_);
  LOG_SENSOR("  ", "Firmware version", firmware_version_sensor_);
}

void CGAnemComponent::publish_binary_sensor_state(binary_sensor::BinarySensor *sensor, bool state) {
  if (sensor != nullptr) {
    sensor->publish_state(state);
  }
}

// ИСПРАВЛЕНИЕ: Правильная формула расчета расхода воздуха из оригинальной библиотеки
float CGAnemComponent::calculate_air_consumption(float wind_speed_mps) {
  if (diameter_mm_ <= 0 || wind_speed_mps < 0) {
    return -255.0f;
  }
  
  // Формула из оригинальной библиотеки: 6 * speed * area * 0.06
  // Константы объединены: 6 * 0.06 = 0.36
  return wind_speed_mps * duct_area_cm2_ * 0.36f;
}

void CGAnemComponent::read_status() {
  uint8_t status = 0;
  if (!this->read_byte(I2C_REG_STATUS, &status)) {
    ESP_LOGW(TAG, "Failed to read status register");
    return;
  }

  publish_binary_sensor_state(status_up_binary_sensor_, (status & STATUS_UNSTEADY_PROCESS) != 0);
  publish_binary_sensor_state(status_ov_binary_sensor_, (status & STATUS_OVERVOLTAGE) != 0);
  publish_binary_sensor_state(status_wdt_binary_sensor_, (status & STATUS_WATCHDOG_TIMER) != 0);
  publish_binary_sensor_state(status_incorrect_taring_binary_sensor_, (status & STATUS_INCORRECT_TARING) != 0);
  publish_binary_sensor_state(status_incorrect_taring_range_binary_sensor_, 
                            (status & STATUS_INCORRECT_TARING_RANGE) != 0);

  if (status & STATUS_UNSTEADY_PROCESS) {
    ESP_LOGW(TAG, "Sensor is in unsteady process");
  }
}

bool CGAnemComponent::read_data() {
  bool success = true;
  
  // Чтение температуры холодного щупа
  uint8_t temp_h, temp_l;
  float ambient_temp = -255.0f;
  if (this->read_byte(I2C_REG_TEMP_COLD_H, &temp_h) && 
      this->read_byte(I2C_REG_TEMP_COLD_L, &temp_l)) {
    int16_t temp_raw = (temp_h << 8) | temp_l;
    ambient_temp = temp_raw / 10.0f;
    if (ambient_temperature_sensor_ != nullptr) {
      ambient_temperature_sensor_->publish_state(ambient_temp);
    }
  } else {
    success = false;
  }
  
  // Чтение температуры горячего щупа
  uint8_t hot_h, hot_l;
  float hotend_temp = -255.0f;
  if (this->read_byte(I2C_REG_TEMP_HOT_H, &hot_h) && 
      this->read_byte(I2C_REG_TEMP_HOT_L, &hot_l)) {
    int16_t hot_raw = (hot_h << 8) | hot_l;
    hotend_temp = hot_raw / 10.0f;
    if (hotend_temperature_sensor_ != nullptr) {
      hotend_temperature_sensor_->publish_state(hotend_temp);
    }
  } else {
    success = false;
  }
  
  // Чтение разницы температур (ДОБАВЛЕНО согласно оригинальной библиотеке)
  uint8_t dt_h, dt_l;
  float temp_diff = -255.0f;
  if (this->read_byte(I2C_REG_DT_H, &dt_h) && 
      this->read_byte(I2C_REG_DT_L, &dt_l)) {
    int16_t dt_raw = (dt_h << 8) | dt_l;
    temp_diff = dt_raw / 10.0f;
    if (temperature_difference_sensor_ != nullptr) {
      temperature_difference_sensor_->publish_state(temp_diff);
    }
  }
  
  // Чтение скорости ветра
  uint8_t wind_h, wind_l;
  float wind_speed = -255.0f;
  if (this->read_byte(I2C_REG_WIND_H, &wind_h) && 
      this->read_byte(I2C_REG_WIND_L, &wind_l)) {
    int16_t wind_raw = (wind_h << 8) | wind_l;
    wind_speed = wind_raw / 10.0f;
    if (air_flow_rate_sensor_ != nullptr) {
      air_flow_rate_sensor_->publish_state(wind_speed);
    }
  } else {
    success = false;
  }
  
  // Чтение минимальной и максимальной скорости
  if (firmware_version_ >= 1.0f) {
    uint8_t min_h, min_l, max_h, max_l;
    
    if (this->read_byte(I2C_REG_WIND_MIN_H, &min_h) && 
        this->read_byte(I2C_REG_WIND_MIN_L, &min_l)) {
      int16_t min_raw = (min_h << 8) | min_l;
      float min_speed = min_raw / 10.0f;
      if (min_air_flow_rate_sensor_ != nullptr) {
        min_air_flow_rate_sensor_->publish_state(min_speed);
      }
    }
    
    if (this->read_byte(I2C_REG_WIND_MAX_H, &max_h) && 
        this->read_byte(I2C_REG_WIND_MAX_L, &max_l)) {
      int16_t max_raw = (max_h << 8) | max_l;
      float max_speed = max_raw / 10.0f;
      if (max_air_flow_rate_sensor_ != nullptr) {
        max_air_flow_rate_sensor_->publish_state(max_speed);
      }
    }
  }
  
  // Чтение напряжения питания
  uint8_t supply_v;
  float supply_voltage = -255.0f;
  if (this->read_byte(I2C_REG_SUPPLY_V, &supply_v)) {
    supply_voltage = supply_v / 10.0f;
    if (supply_voltage_sensor_ != nullptr) {
      supply_voltage_sensor_->publish_state(supply_voltage);
    }
  }
  
  // Чтение мощности нагрева (ИСПРАВЛЕНО согласно оригинальной библиотеке)
  uint8_t power;
  float heat_power = -255.0f;
  if (this->read_byte(I2C_REG_PWR_WT, &power)) {
    // Формула из оригинальной библиотеки: (power * 1.36125) / 255
    heat_power = (power * 1.36125f) / 255.0f;
    if (heat_power_sensor_ != nullptr) {
      heat_power_sensor_->publish_state(heat_power);
    }
  }
  
  // Расчет расхода воздуха
  if (air_consumption_sensor_ != nullptr && wind_speed != -255.0f) {
    float consumption = calculate_air_consumption(wind_speed);
    if (consumption != -255.0f) {
      air_consumption_sensor_->publish_state(consumption);
    }
  }
  
  return success;
}

void CGAnemComponent::update() {
  read_status();
  
  if (!read_data()) {
    ESP_LOGW(TAG, "Failed to read sensor data");
    this->status_set_warning();
  } else {
    this->status_clear_warning();
  }
}

}  // namespace cg_anem
}  // namespace esphome