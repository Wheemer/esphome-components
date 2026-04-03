#include "hts221.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hts221 {

static const char *const TAG = "hts221.sensor";

// Register addresses (Table 15 in datasheet)
static const uint8_t HTS221_WHO_AM_I_REG       = 0x0F;
static const uint8_t HTS221_WHO_AM_I_VALUE     = 0xBC;  // Section 7.1
static const uint8_t HTS221_AV_CONF_REG        = 0x10;  // Section 7.2
static const uint8_t HTS221_CTRL_REG1          = 0x20;  // Section 7.3
static const uint8_t HTS221_CTRL_REG2          = 0x21;  // Section 7.4
static const uint8_t HTS221_CTRL_REG3          = 0x22;  // Section 7.5
static const uint8_t HTS221_STATUS_REG         = 0x27;  // Section 7.6
static const uint8_t HTS221_HUMIDITY_OUT_L_REG = 0x28;  // Section 7.7
static const uint8_t HTS221_HUMIDITY_OUT_H_REG = 0x29;  // Section 7.8
static const uint8_t HTS221_TEMP_OUT_L_REG     = 0x2A;  // Section 7.9
static const uint8_t HTS221_TEMP_OUT_H_REG     = 0x2B;  // Section 7.10

// Calibration registers (Table 19 in datasheet)
static const uint8_t HTS221_H0_rH_x2_REG       = 0x30;
static const uint8_t HTS221_H1_rH_x2_REG       = 0x31;
static const uint8_t HTS221_T0_degC_x8_REG     = 0x32;
static const uint8_t HTS221_T1_degC_x8_REG     = 0x33;
static const uint8_t HTS221_T1_T0_MSB_REG      = 0x35;
static const uint8_t HTS221_H0_T0_OUT_REG      = 0x36;  // 16-bit, LSB first
static const uint8_t HTS221_H1_T0_OUT_REG      = 0x3A;  // 16-bit, LSB first
static const uint8_t HTS221_T0_OUT_REG         = 0x3C;  // 16-bit, LSB first
static const uint8_t HTS221_T1_OUT_REG         = 0x3E;  // 16-bit, LSB first

// CTRL_REG1 bits (Section 7.3)
static const uint8_t HTS221_CTRL1_PD           = 0x80;  // Power down control
static const uint8_t HTS221_CTRL1_BDU          = 0x04;  // Block data update
static const uint8_t HTS221_CTRL1_ODR_MASK     = 0x03;  // Output data rate mask

// Output Data Rates (Table 17)
static const uint8_t HTS221_ODR_ONE_SHOT       = 0x00;
static const uint8_t HTS221_ODR_1HZ            = 0x01;
static const uint8_t HTS221_ODR_7HZ            = 0x02;
static const uint8_t HTS221_ODR_12_5HZ         = 0x03;

// CTRL_REG2 bits (Section 7.4)
static const uint8_t HTS221_CTRL2_BOOT         = 0x80;  // Reboot memory content
static const uint8_t HTS221_CTRL2_HEATER       = 0x02;  // Heater enable
static const uint8_t HTS221_CTRL2_ONE_SHOT     = 0x01;  // One-shot conversion

// STATUS_REG bits (Section 7.6)
static const uint8_t HTS221_STATUS_HUMIDITY_READY = 0x02;  // H_DA bit
static const uint8_t HTS221_STATUS_TEMPERATURE_READY = 0x01;  // T_DA bit

// Default averaging configuration (Table 16) - 16 samples for both
static const uint8_t HTS221_AVG_DEFAULT = 0x1B;  // 16 temp + 16 humidity

void hts221Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HTS221...");
  
  // Reset component state if it was failed
  if ((this->component_state_ & COMPONENT_STATE_MASK) == COMPONENT_STATE_FAILED) {
    this->component_state_ &= ~COMPONENT_STATE_MASK;
    this->component_state_ |= COMPONENT_STATE_CONSTRUCTION;
  }

  // Check WHO_AM_I register (Section 7.1)
  uint8_t who_am_i;
  if (!this->read_byte(HTS221_WHO_AM_I_REG, &who_am_i)) {
    ESP_LOGE(TAG, "Failed to read WHO_AM_I register");
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  
  if (who_am_i != HTS221_WHO_AM_I_VALUE) {
    ESP_LOGE(TAG, "WHO_AM_I value mismatch: expected 0x%02X, got 0x%02X", 
             HTS221_WHO_AM_I_VALUE, who_am_i);
    this->error_code_ = WHO_AM_I_MISMATCH;
    this->mark_failed();
    return;
  }
  
  // Reboot the device to ensure clean state (Section 7.4)
  if (!this->write_byte(HTS221_CTRL_REG2, HTS221_CTRL2_BOOT)) {
    ESP_LOGE(TAG, "Failed to reboot device");
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  
  delay(50);  // Wait for boot to complete (from ST example code)
  
  // Configure averaging (Section 7.2) - reduces noise (Table 16)
  if (!this->write_byte(HTS221_AV_CONF_REG, HTS221_AVG_DEFAULT)) {
    ESP_LOGW(TAG, "Failed to set averaging configuration, continuing anyway");
  }
  
  // Read calibration data (Section 8)
  if (!this->read_calibration_()) {
    ESP_LOGE(TAG, "Failed to read calibration data");
    this->error_code_ = CALIBRATION_FAILED;
    this->mark_failed();
    return;
  }
  
  // Configure device: power on, block data update, 1 Hz output data rate
  // Section 7.3: PD=1 (active), BDU=1 (block update), ODR=01 (1 Hz)
  uint8_t ctrl1 = HTS221_CTRL1_PD | HTS221_CTRL1_BDU | HTS221_ODR_1HZ;
  if (!this->write_byte(HTS221_CTRL_REG1, ctrl1)) {
    ESP_LOGE(TAG, "Failed to configure CTRL_REG1");
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  
  // Optionally enable Data Ready on pin 3 (Section 7.5)
  // Not required for normal operation, but useful for external interrupts
  // uint8_t ctrl3 = 0x04;  // DRDY_EN = 1
  // this->write_byte(HTS221_CTRL_REG3, ctrl3);
  
  calibrated_ = true;
  current_odr_ = HTS221_ODR_1HZ;
  ESP_LOGI(TAG, "HTS221 initialized successfully");
}

bool hts221Component::read_calibration_() {
  uint8_t h0_rh_x2, h1_rh_x2;
  uint8_t t0_degc_x8, t1_degc_x8;
  uint8_t t0_t1_msb;
  uint8_t h0_t0_out_l, h0_t0_out_h;
  uint8_t h1_t0_out_l, h1_t0_out_h;
  uint8_t t0_out_l, t0_out_h;
  uint8_t t1_out_l, t1_out_h;
  
  // Read humidity calibration (Section 8, Table 19)
  if (!this->read_byte(HTS221_H0_rH_x2_REG, &h0_rh_x2) ||
      !this->read_byte(HTS221_H1_rH_x2_REG, &h1_rh_x2)) {
    ESP_LOGE(TAG, "Failed to read humidity calibration registers");
    return false;
  }
  
  // Read temperature calibration (Section 8, Table 19)
  if (!this->read_byte(HTS221_T0_degC_x8_REG, &t0_degc_x8) ||
      !this->read_byte(HTS221_T1_degC_x8_REG, &t1_degc_x8) ||
      !this->read_byte(HTS221_T1_T0_MSB_REG, &t0_t1_msb)) {
    ESP_LOGE(TAG, "Failed to read temperature calibration registers");
    return false;
  }
  
  // Read calibration output values (Section 8, Table 19)
  if (!this->read_byte(HTS221_H0_T0_OUT_REG, &h0_t0_out_l) ||
      !this->read_byte(HTS221_H0_T0_OUT_REG + 1, &h0_t0_out_h) ||
      !this->read_byte(HTS221_H1_T0_OUT_REG, &h1_t0_out_l) ||
      !this->read_byte(HTS221_H1_T0_OUT_REG + 1, &h1_t0_out_h) ||
      !this->read_byte(HTS221_T0_OUT_REG, &t0_out_l) ||
      !this->read_byte(HTS221_T0_OUT_REG + 1, &t0_out_h) ||
      !this->read_byte(HTS221_T1_OUT_REG, &t1_out_l) ||
      !this->read_byte(HTS221_T1_OUT_REG + 1, &t1_out_h)) {
    ESP_LOGE(TAG, "Failed to read calibration output registers");
    return false;
  }
  
  // Process humidity calibration (Section 8, Step 2)
  // H0_rH_x2 and H1_rH_x2 are unsigned 8-bit, need to divide by 2
  float h0_rh = h0_rh_x2 / 2.0f;
  float h1_rh = h1_rh_x2 / 2.0f;
  int16_t h0_t0_out = (int16_t)((h0_t0_out_h << 8) | h0_t0_out_l);
  int16_t h1_t0_out = (int16_t)((h1_t0_out_h << 8) | h1_t0_out_l);
  
  // Process temperature calibration (Section 8, Step 1)
  // T0 and T1 are 10-bit values: 2 MSB in T1_T0_MSB, 8 LSB in T0_degC_x8/T1_degC_x8
  uint16_t t0_degc_raw = t0_degc_x8 | ((t0_t1_msb & 0x03) << 8);
  uint16_t t1_degc_raw = t1_degc_x8 | ((t0_t1_msb & 0x0C) << 6);
  float t0_temp = t0_degc_raw / 8.0f;  // Divide by 8 as per datasheet
  float t1_temp = t1_degc_raw / 8.0f;
  int16_t t0_out = (int16_t)((t0_out_h << 8) | t0_out_l);
  int16_t t1_out = (int16_t)((t1_out_h << 8) | t1_out_l);
  
  // Calculate slopes and offsets (linear interpolation from Section 8)
  // Temperature: T_degC = T_slope * T_OUT + T_zero
  // Humidity: H_rH = H_slope * H_OUT + H_zero
  temperature_slope_ = (t1_temp - t0_temp) / (t1_out - t0_out);
  temperature_zero_ = t0_temp - temperature_slope_ * t0_out;
  
  humidity_slope_ = (h1_rh - h0_rh) / (h1_t0_out - h0_t0_out);
  humidity_zero_ = h0_rh - humidity_slope_ * h0_t0_out;
  
  ESP_LOGV(TAG, "Calibration: T_slope=%.6f, T_zero=%.2f, H_slope=%.6f, H_zero=%.2f",
           temperature_slope_, temperature_zero_, humidity_slope_, humidity_zero_);
  
  return true;
}

bool hts221Component::is_data_ready_() {
  uint8_t status;
  if (!this->read_byte(HTS221_STATUS_REG, &status)) {
    return false;
  }
  // Check both humidity and temperature data available (Section 7.6)
  return (status & HTS221_STATUS_TEMPERATURE_READY) && 
         (status & HTS221_STATUS_HUMIDITY_READY);
}

bool hts221Component::wait_for_data_ready_(int timeout_ms) {
  int attempts = timeout_ms / 5;  // Check every 5ms
  while (attempts-- > 0) {
    if (is_data_ready_()) {
      return true;
    }
    delay(5);
  }
  return false;
}

void hts221Component::set_power_mode_(bool enable) {
  uint8_t ctrl1;
  if (this->read_byte(HTS221_CTRL_REG1, &ctrl1)) {
    if (enable) {
      ctrl1 |= HTS221_CTRL1_PD;
    } else {
      ctrl1 &= ~HTS221_CTRL1_PD;
    }
    this->write_byte(HTS221_CTRL_REG1, ctrl1);
  }
}

bool hts221Component::read_measurements_(float &temperature, float &humidity) {
  // For one-shot mode, we need to trigger conversion (Section 7.4)
  // Note: In continuous mode (ODR != 0), this step is not needed
  if (current_odr_ == HTS221_ODR_ONE_SHOT) {
    // Trigger one-shot conversion
    if (!this->write_byte(HTS221_CTRL_REG2, HTS221_CTRL2_ONE_SHOT)) {
      ESP_LOGW(TAG, "Failed to trigger one-shot conversion");
      return false;
    }
    
    // Wait for data ready (Section 7.4 says ONE_SHOT bit resets when done)
    if (!wait_for_data_ready_(50)) {  // Max 50ms per datasheet
      ESP_LOGW(TAG, "Data not ready after one-shot trigger");
      return false;
    }
  } else {
    // In continuous mode, just wait for next sample
    if (!wait_for_data_ready_(100)) {
      ESP_LOGW(TAG, "Data not ready in continuous mode");
      return false;
    }
  }
  
  // Read temperature (16-bit, 2's complement) (Sections 7.9, 7.10)
  uint8_t temp_l, temp_h;
  if (!this->read_byte(HTS221_TEMP_OUT_L_REG, &temp_l) ||
      !this->read_byte(HTS221_TEMP_OUT_H_REG, &temp_h)) {
    return false;
  }
  int16_t temp_raw = (int16_t)((temp_h << 8) | temp_l);
  
  // Read humidity (16-bit, 2's complement) (Sections 7.7, 7.8)
  uint8_t hum_l, hum_h;
  if (!this->read_byte(HTS221_HUMIDITY_OUT_L_REG, &hum_l) ||
      !this->read_byte(HTS221_HUMIDITY_OUT_H_REG, &hum_h)) {
    return false;
  }
  int16_t hum_raw = (int16_t)((hum_h << 8) | hum_l);
  
  // Calculate values using linear interpolation (Section 8)
  temperature = temp_raw * temperature_slope_ + temperature_zero_;
  humidity = hum_raw * humidity_slope_ + humidity_zero_;
  
  // Clamp humidity to valid range (0-100% as per datasheet Table 3)
  if (humidity < 0.0f) humidity = 0.0f;
  if (humidity > 100.0f) humidity = 100.0f;
  
  return true;
}

void hts221Component::update() {
  if (!calibrated_) {
    ESP_LOGW(TAG, "Device not calibrated, skipping update");
    this->status_set_warning();
    return;
  }
  
  float temperature = 0.0f, humidity = 0.0f;
  
  if (!read_measurements_(temperature, humidity)) {
    ESP_LOGW(TAG, "Failed to read measurements");
    this->status_set_warning();
    return;
  }
  
  // Publish values
  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state(temperature);
  }
  
  if (this->humidity_sensor_ != nullptr) {
    this->humidity_sensor_->publish_state(humidity);
  }
  
  ESP_LOGD(TAG, "Got temperature=%.2f°C, humidity=%.2f%%", temperature, humidity);
  this->status_clear_warning();
}

void hts221Component::dump_config() {
  ESP_LOGCONFIG(TAG, "HTS221:");
  LOG_I2C_DEVICE(this);
  
  switch (this->error_code_) {
    case COMMUNICATION_FAILED:
      ESP_LOGE(TAG, "  Communication failed!");
      break;
    case WHO_AM_I_MISMATCH:
      ESP_LOGE(TAG, "  WHO_AM_I register mismatch!");
      break;
    case CALIBRATION_FAILED:
      ESP_LOGE(TAG, "  Failed to read calibration data!");
      break;
    case NONE:
    default:
      break;
  }
  
  if (calibrated_) {
    ESP_LOGCONFIG(TAG, "  Calibrated: Yes");
    ESP_LOGCONFIG(TAG, "  Output Data Rate: %s", 
                  current_odr_ == HTS221_ODR_1HZ ? "1 Hz" :
                  current_odr_ == HTS221_ODR_7HZ ? "7 Hz" :
                  current_odr_ == HTS221_ODR_12_5HZ ? "12.5 Hz" : "One-shot");
  }
  
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
}

}  // namespace hts221
}  // namespace esphome
