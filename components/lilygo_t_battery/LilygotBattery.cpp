#include "LilygotBattery.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32
#include "esp_adc/adc_oneshot.h"
#endif

namespace esphome {
namespace lilygo_t_battery {

static const char *const TAG = "lilygo_battery";

void LilygotBattery::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Lilygo Battery Monitor...");
  
  // Настройка пина включения
  if (enable_pin_ != nullptr) {
    enable_pin_->setup();
    enable_pin_->digital_write(false);
    ESP_LOGD(TAG, "Enable pin configured");
  }
  
#ifdef USE_ESP32
  // Используем новый ADC API (adc_oneshot)
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT_1,
  };
  esp_err_t ret = adc_oneshot_new_unit(&init_config, &adc_handle_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize ADC unit: %s", esp_err_to_name(ret));
    return;
  }
  
  // Настройка канала (GPIO34 = ADC1_CHANNEL_6)
  adc_oneshot_chan_cfg_t config = {
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ret = adc_oneshot_config_channel(adc_handle_, ADC_CHANNEL_6, &config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGD(TAG, "ADC configured for GPIO34 (Channel 6)");
  }
#endif
  
  ESP_LOGCONFIG(TAG, "Setup complete");
}

float LilygotBattery::read_adc_voltage_() {
  float adc_voltage = 0.0f;
  
  // Если есть внешний ADC сенсор, используем его
  if (adc_sensor_ != nullptr && adc_sensor_->has_state()) {
    adc_voltage = adc_sensor_->state;
    ESP_LOGV(TAG, "ADC from sensor: %.3f V", adc_voltage);
    return adc_voltage;
  }
  
#ifdef USE_ESP32
  // Прямое чтение ADC через adc_oneshot
  if (adc_handle_ != nullptr) {
    int adc_raw = 0;
    esp_err_t ret = adc_oneshot_read(adc_handle_, ADC_CHANNEL_6, &adc_raw);
    if (ret == ESP_OK && adc_raw > 0) {
      // Преобразуем в напряжение (12-bit ADC, 0-4095)
      adc_voltage = (adc_raw / 4095.0f) * reference_voltage_;
      ESP_LOGV(TAG, "ADC raw: %d, voltage: %.3f V", adc_raw, adc_voltage);
    } else {
      ESP_LOGW(TAG, "Failed to read ADC: %s", esp_err_to_name(ret));
    }
  } else {
    ESP_LOGW(TAG, "ADC handle not initialized");
  }
#else
  // Для Arduino framework
  adc_voltage = analogRead(34) / 4095.0f * reference_voltage_;
#endif
  
  return adc_voltage;
}

float LilygotBattery::calculate_usb_voltage_(float battery_voltage) {
  // Если напряжение выше 4.5V, значит подключен USB
  if (battery_voltage > 4.5f) {
    return battery_voltage;
  }
  return 0.0f;
}

int LilygotBattery::calculate_battery_level_(float battery_voltage) {
  // Типичные значения для LiPo аккумулятора 3.7V
  const float BATTERY_FULL = 4.2f;
  const float BATTERY_EMPTY = 3.25f;
  
  // Если напряжение выше максимума
  if (battery_voltage >= BATTERY_FULL) {
    return 100;
  }
  
  // Если напряжение ниже минимума
  if (battery_voltage <= BATTERY_EMPTY) {
    return 0;
  }
  
  // Линейная интерполяция
  int level = (int)((battery_voltage - BATTERY_EMPTY) / (BATTERY_FULL - BATTERY_EMPTY) * 100.0f);
  
  // Ограничиваем диапазон
  if (level > 100) level = 100;
  if (level < 0) level = 0;
  
  return level;
}

void LilygotBattery::update() {
  ESP_LOGV(TAG, "Starting battery update...");
  
  // Включаем пин для измерения (если нужен)
  if (enable_pin_ != nullptr) {
    enable_pin_->digital_write(true);
    // Небольшая задержка для стабилизации
    delay(10);
  }
  
  // Читаем напряжение с ADC
  float adc_voltage = read_adc_voltage_();
  
  // Вычисляем реальное напряжение батареи с учетом делителя
  float battery_voltage = adc_voltage * voltage_divider_;
  
  // Вычисляем напряжение USB (если подключен)
  float usb_voltage = calculate_usb_voltage_(battery_voltage);
  
  // Корректируем напряжение батареи, если USB подключен
  float corrected_battery_voltage = battery_voltage;
  if (usb_voltage > 0) {
    // Во время зарядки напряжение завышено, корректируем
    corrected_battery_voltage = battery_voltage - 0.1f;
    if (corrected_battery_voltage < 3.25f) {
      corrected_battery_voltage = 3.25f;
    }
  }
  
  // Вычисляем уровень заряда
  int battery_level = calculate_battery_level_(corrected_battery_voltage);
  
  // Логирование
  ESP_LOGD(TAG, "=== Battery Status ===");
  ESP_LOGD(TAG, "ADC Voltage: %.3f V", adc_voltage);
  ESP_LOGD(TAG, "Battery Voltage: %.2f V", battery_voltage);
  ESP_LOGD(TAG, "USB Voltage: %.2f V", usb_voltage);
  ESP_LOGD(TAG, "Battery Level: %d %%", battery_level);
  ESP_LOGD(TAG, "======================");
  
  // Публикуем значения в сенсоры
  if (voltage_sensor_ != nullptr) {
    voltage_sensor_->publish_state(battery_voltage);
  }
  
  if (bus_voltage_sensor_ != nullptr) {
    bus_voltage_sensor_->publish_state(usb_voltage);
  }
  
  if (battery_level_sensor_ != nullptr) {
    battery_level_sensor_->publish_state((float)battery_level);
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
  
  if (voltage_sensor_ != nullptr) {
    LOG_SENSOR("  ", "Battery Voltage", voltage_sensor_);
  }
  if (bus_voltage_sensor_ != nullptr) {
    LOG_SENSOR("  ", "USB Voltage", bus_voltage_sensor_);
  }
  if (battery_level_sensor_ != nullptr) {
    LOG_SENSOR("  ", "Battery Level", battery_level_sensor_);
  }
  
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace lilygo_t_battery
}  // namespace esphome
