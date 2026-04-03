#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/gpio.h"
#include <vector>

namespace esphome {
namespace aw9310x {

// Режимы работы чипа
enum OperationMode {
  DEEPSLEEP_MODE = 4,
  SLEEP_MODE = 2,
  DOZE_MODE = 3,
  ACTIVE_MODE = 1
};

// Конфигурация канала
struct ChannelConfig {
  uint8_t channel;
  bool enable_in_ear_detection;
  uint32_t proximity_threshold;
  uint32_t touch_threshold;
  uint8_t gain;          // 0-7: усиление AFE
  uint8_t resolution;    // 0-7: разрешение ADC (6 = 1fF)
  uint8_t freq;          // 0-7: частота сканирования
  uint16_t doze_period_ms;  // Период в Doze режиме
};

class AW9310XChannel;

class AW9310XComponent : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  
  // Основные настройки
  void set_interrupt_pin(InternalGPIOPin *pin) { interrupt_pin_ = pin; }
  void set_operation_mode(OperationMode mode);
  void set_auto_mode_switching(bool enable) { auto_mode_switching_ = enable; }
  
  // Настройка каналов
  void set_channel_config(uint8_t channel, const ChannelConfig &config);
  void register_channel(AW9310XChannel *sensor, uint8_t channel);
  
  // Датчики сырых значений
  void set_raw_sensor(sensor::Sensor *sensor, uint8_t channel);
  
  // Калибровка и компенсация
  bool calibrate_all_channels();
  bool calibrate_channel(uint8_t channel);
  bool enable_auto_offset_tuning(bool enable);
  bool set_parasitic_capacitance(uint8_t channel, uint16_t cp_value_pf); // 0-150pF
  
  // Настройка чувствительности
  void set_global_threshold(uint32_t proximity_th, uint32_t touch_th);
  void set_channel_sensitivity(uint8_t channel, uint8_t gain, uint8_t resolution);
  
  // Бинарные сенсоры
  binary_sensor::BinarySensor *touch_sensors_[3] = {nullptr};
  binary_sensor::BinarySensor *proximity_sensors_[3] = {nullptr};

 protected:
  // I2C операции
  bool read_register(uint16_t reg, uint32_t *value);
  bool write_register(uint16_t reg, uint32_t value);
  bool read_register_byte(uint16_t reg, uint8_t *value);
  bool write_register_byte(uint16_t reg, uint8_t value);
  
  // Инициализация
  bool read_chip_id();
  bool sw_reset();
  bool load_default_config();
  
  // Чтение данных
  bool read_diff_values(int32_t *diff);
  bool read_raw_counts(int32_t *raw, uint8_t channel);
  
  // Обработка касаний
  void update_touch_states(int32_t *diff);
  void update_proximity_states(int32_t *diff);
  
  // Обработка прерываний
  void handle_interrupt();
  uint32_t read_interrupt_source();
  
 private:
  InternalGPIOPin *interrupt_pin_{nullptr};
  OperationMode current_mode_{ACTIVE_MODE};
  bool auto_mode_switching_{true};
  
  // Конфигурация каналов
  ChannelConfig channel_configs_[3];
  std::vector<uint8_t> enabled_channels_;
  
  // Состояния каналов
  bool touch_states_[3] = {false};
  bool proximity_states_[3] = {false};
  
  // Датчики сырых значений
  sensor::Sensor *raw_sensors_[3] = {nullptr};
  
  // Пороговые значения по умолчанию
  uint32_t default_proximity_threshold_{50000};
  uint32_t default_touch_threshold_{80000};
  
  // Для обработки прерываний
  bool last_interrupt_state_{false};
  uint32_t last_interrupt_time_{0};
  
  static constexpr uint8_t max_channels_ = 3;
  static constexpr uint16_t I2C_TIMEOUT_MS_ = 100;
};

// Класс для бинарного сенсора касания
class AW9310XChannel : public binary_sensor::BinarySensor {
 public:
  void set_channel(uint8_t channel) { channel_ = channel; }
  uint8_t get_channel() const { return channel_; }
  
  void set_sensor_type(const std::string &type) { 
    is_touch_ = (type == "touch"); 
    is_proximity_ = (type == "proximity");
  }
  
  bool is_touch() const { return is_touch_; }
  bool is_proximity() const { return is_proximity_; }
  
 private:
  uint8_t channel_{0};
  bool is_touch_{true};
  bool is_proximity_{false};
};

}  // namespace aw9310x
}  // namespace esphome
