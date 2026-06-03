#include "LilygotBattery.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32
#include <esp_adc_cal.h>
#include <driver/adc.h>
#endif

namespace esphome {
namespace lilygo_t_battery {

static const char *const TAG = "lilygo_battery";

void LilygotBattery::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Lilygo Battery Monitor...");
  
#ifdef USE_ESP32
  // Настройка ADC для ESP-IDF
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12);  // GPIO34 = ADC1_CHANNEL_6
  
  // Калибровка ADC
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, reference_voltage_ * 1000, &adc_chars_);
  adc_calibrated_ = true;
  ESP_LOGD(TAG, "ADC calibration completed");
#endif

  // Настройка пина включения, если он задан
  if (enable_pin_ != nullptr) {
    enable_pin_->setup();
    enable_pin_->digital_write(false);
    ESP_LOGD(TAG, "Enable pin configured");
  }
}

float LilygotBattery::read_adc_voltage_() {
  float adc_voltage = 0.0f;
  
#ifdef USE_ESP32
  if (adc_sensor_ != nullptr && adc_sensor_->has_state()) {
    // Используем значение из ADC сенсора (уже отфильтрованное)
    adc_voltage = adc_sensor_->state;
  } else {
    // Прямое чтение ADC
    uint32_t adc_raw = adc1_get_raw(ADC1_CHANNEL_6);
    if (adc_calibrated_) {
      uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_raw, &adc_chars_);
      adc_voltage = voltage_mv / 1000.0f;
    } else {
      adc_voltage = (adc_raw / 4095.0f) * reference_voltage_;
    }
  }
#else
  if (adc_sensor_ != nullptr && adc_sensor_->has_state()) {
    adc_voltage = adc_sensor_->state;
  }
#endif
  
  return adc_voltage;
}

float LilygotBattery::calculate_battery_voltage_(float adc_voltage) {
  // С учетом делителя напряжения
  return adc_voltage * voltage_divider_;
}

bool LilygotBattery::is_usb_present_() {
  // Проверяем наличие USB по напряжению выше 4.5V
  float adc_voltage = read_adc_voltage_();
  float raw_voltage = calculate_battery_voltage_(adc_voltage);
  
  // Если напряжение > 4.5V, значит подключен USB (обычно 5V через делитель)
  return raw_voltage > 4.5f;
}

int LilygotBattery::calculate_battery_level_(float battery_voltage, bool is_usb_present) {
  // Типичные значения для LiPo аккумулятора 3.7V
  const float BATTERY_FULL = 4.2f;
  const float BATTERY_EMPTY = 3.25f;
  
  if (is_usb_present) {
    // При подключенном USB аккумулятор заряжается
    if (battery_voltage >= BATTERY_FULL) {
      return 100;
    }
    // Во время зарядки напряжение может быть выше реального уровня
    // Используем скорректированную формулу
    float corrected_voltage = battery_voltage - 0.1f;
    if (corrected_voltage < BATTERY_EMPTY) corrected_voltage = BATTERY_EMPTY;
    int level = (int)((corrected_voltage - BATTERY_EMPTY) / (BATTERY_FULL - BATTERY_EMPTY) * 100);
    return std::min(100, std::max(0, level));
  } else {
    // Режим разряда
    if (battery_voltage >= BATTERY_FULL) return 100;
    if (battery_voltage <= BATTERY_EMPTY) return 0;
    int level = (int)((battery_voltage - BATTERY_EMPTY) / (BATTERY_FULL - BATTERY_EMPTY) * 100);
    return std::min(100, std::max(0, level));
  }
}

void LilygotBattery::update() {
  // Включаем пин для измерения (если нужно)
  if (enable_pin_ != nullptr) {
    enable_pin_->digital_write(true);
    esp_rom_delay_us(10000);  // 10ms задержка без блокировки
  }
  
  // Читаем ADC значение
  float adc_voltage = read_adc_voltage_();
  
  // Вычисляем напряжение батареи с учетом делителя
  float battery_voltage = calculate_battery_voltage_(adc_voltage);
  
  // Проверяем наличие USB
  bool usb_connected = is_usb_present_();
  
  // Вычисляем уровень заряда
  int battery_level_percent = calculate_battery_level_(battery_voltage, usb_connected);
  
  // Вычисляем USB напряжение (если есть USB, иначе 0)
  float usb_voltage = usb_connected ? battery_voltage : 0.0f;
  
  ESP_LOGD(TAG, "ADC Voltage: %.3f V", adc_voltage);
  ESP_LOGD(TAG, "Battery Voltage: %.2f V", battery_voltage);
  ESP_LOGD(TAG, "USB Connected: %s", usb_connected ? "YES" : "NO");
  ESP_LOGD(TAG, "Battery Level: %d %%", battery_level_percent);
  
  // Публикуем значения в сенсоры
  if (this->voltage != nullptr) {
    this->voltage->publish_state(battery_voltage);
  }
  if (this->bus_voltage != nullptr) {
    this->bus_voltage->publish_state(usb_voltage);
  }
  if (this->battery_level != nullptr) {
    this->battery_level->publish_state((float)battery_level_percent);
  }
  
  // Выключаем пин
  if (enable_pin_ != nullptr) {
    enable_pin_->digital_write(false);
  }
}

void LilygotBattery::dump_config() {
  ESP_LOGCONFIG(TAG, "Lilygo Battery Monitor:");
  ESP_LOGCONFIG(TAG, "  Voltage Divider: %.1f", voltage_divider_);
  ESP_LOGCONFIG(TAG, "  Reference Voltage: %.2f V", reference_voltage_);
  LOG_SENSOR("  ", "Battery Voltage", this->voltage);
  LOG_SENSOR("  ", "USB Voltage", this->bus_voltage);
  LOG_SENSOR("  ", "Battery Level", this->battery_level);
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace lilygo_t_battery
}  // namespace esphome
