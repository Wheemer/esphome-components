#include "aw9310x.h"
#include "aw9310x_reg.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace aw9310x {

static const char *const TAG = "aw9310x";

void AW9310XComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AW93103 Touch Sensor...");
  
  // Проверка I2C соединения
  if (!this->read_chip_id()) {
    ESP_LOGE(TAG, "Failed to read chip ID - check I2C connection");
    this->mark_failed();
    return;
  }
  
  // Сброс чипа
  if (!this->sw_reset()) {
    ESP_LOGE(TAG, "Software reset failed");
    this->mark_failed();
    return;
  }
  
  // Загрузка конфигурации по умолчанию
  if (!this->load_default_config()) {
    ESP_LOGE(TAG, "Failed to load default configuration");
    this->mark_failed();
    return;
  }
  
  // Калибровка всех каналов
  if (!this->calibrate_all_channels()) {
    ESP_LOGW(TAG, "Auto calibration had issues, but continuing");
  }
  
  // Настройка порогов
  this->set_global_threshold(default_proximity_threshold_, default_touch_threshold_);
  
  // Применение конфигураций каналов
  for (uint8_t i = 0; i < max_channels_; i++) {
    if (channel_configs_[i].enable_in_ear_detection) {
      this->set_channel_sensitivity(i, channel_configs_[i].gain, channel_configs_[i].resolution);
      this->set_parasitic_capacitance(i, 0); // Автоопределение
    }
  }
  
  // Включение Auto-Offset-Tuning (AOT)
  this->enable_auto_offset_tuning(true);
  
  // Установка активного режима
  this->set_operation_mode(ACTIVE_MODE);
  
  ESP_LOGI(TAG, "AW93103 initialized successfully");
  ESP_LOGI(TAG, "  Proximity threshold: %d", default_proximity_threshold_);
  ESP_LOGI(TAG, "  Touch threshold: %d", default_touch_threshold_);
}

void AW9310XComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AW93103 Touch Sensor:");
  LOG_I2C_DEVICE(this);
  
  if (this->interrupt_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Interrupt Pin: GPIO%d", this->interrupt_pin_->get_pin());
  }
  
  ESP_LOGCONFIG(TAG, "  Operation Mode: %d", (int)current_mode_);
  ESP_LOGCONFIG(TAG, "  Auto Mode Switching: %s", auto_mode_switching_ ? "ON" : "OFF");
  
  for (int i = 0; i < max_channels_; i++) {
    if (channel_configs_[i].enable_in_ear_detection) {
      ESP_LOGCONFIG(TAG, "  Channel %d:", i);
      ESP_LOGCONFIG(TAG, "    Proximity Threshold: %d", channel_configs_[i].proximity_threshold);
      ESP_LOGCONFIG(TAG, "    Touch Threshold: %d", channel_configs_[i].touch_threshold);
      ESP_LOGCONFIG(TAG, "    Gain: %d, Resolution: %d", channel_configs_[i].gain, channel_configs_[i].resolution);
    }
  }
  
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Setup failed!");
  }
}

bool AW9310XComponent::read_register(uint16_t reg, uint32_t *value) {
  uint8_t reg_addr[2] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF)};
  uint8_t data[4];
  
  if (this->write(reg_addr, 2) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Failed to write register address 0x%04X", reg);
    return false;
  }
  
  delay(1); // Небольшая задержка для стабильности
  
  if (this->read(data, 4) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Failed to read data from register 0x%04X", reg);
    return false;
  }
  
  // Big-endian to host
  *value = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | 
           ((uint32_t)data[2] << 8) | (uint32_t)data[3];
  
  ESP_LOGVV(TAG, "Read register 0x%04X = 0x%08lX", reg, (unsigned long)*value);
  return true;
}

bool AW9310XComponent::write_register(uint16_t reg, uint32_t value) {
  uint8_t data[6];
  data[0] = reg >> 8;
  data[1] = reg & 0xFF;
  data[2] = value >> 24;
  data[3] = (value >> 16) & 0xFF;
  data[4] = (value >> 8) & 0xFF;
  data[5] = value & 0xFF;
  
  i2c::ErrorCode err = this->write(data, 6);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Failed to write register 0x%04X (error: %d)", reg, err);
    return false;
  }
  
  ESP_LOGVV(TAG, "Write register 0x%04X = 0x%08lX", reg, (unsigned long)value);
  return true;
}

bool AW9310XComponent::read_register_byte(uint16_t reg, uint8_t *value) {
  uint32_t val32;
  if (!read_register(reg, &val32)) {
    return false;
  }
  *value = (uint8_t)(val32 & 0xFF);
  return true;
}

bool AW9310XComponent::write_register_byte(uint16_t reg, uint8_t value) {
  return write_register(reg, (uint32_t)value);
}

bool AW9310XComponent::read_chip_id() {
  uint32_t data = 0;
  
  for (int i = 0; i < 3; i++) {
    if (this->read_register(REG_CHIP_ID, &data)) {
      uint16_t chip_id = (uint16_t)(data >> 16);
      if (chip_id == AW93103_CHIP_ID) {
        ESP_LOGI(TAG, "Chip ID verified: 0x%04X (AW93103)", chip_id);
        return true;
      } else if (chip_id == AW93105_CHIP_ID) {
        ESP_LOGI(TAG, "Chip ID: 0x%04X (AW93105 - 6 channel version)", chip_id);
        return true;
      } else {
        ESP_LOGW(TAG, "Unexpected chip ID: 0x%04X", chip_id);
      }
    }
    delay(10);
  }
  
  ESP_LOGE(TAG, "Failed to read valid chip ID");
  return false;
}

bool AW9310XComponent::sw_reset() {
  // Запись 0 в регистр HOSTCTRL2 вызывает сброс
  if (!this->write_register(REG_HOSTCTRL2, 0x00000000)) {
    ESP_LOGE(TAG, "Failed to write reset register");
    return false;
  }
  delay(20); // Время на сброс согласно документации
  return true;
}

bool AW9310XComponent::load_default_config() {
  // Включение каналов 0-2
  uint32_t scan_ctrl0 = SCANCTRL0_CH_EN_MASK; // Биты 0-2 = 1
  if (!this->write_register(REG_SCANCTRL0, scan_ctrl0)) {
    ESP_LOGE(TAG, "Failed to configure scan control");
    return false;
  }
  
  // Настройка периода сканирования в активном режиме (30ms по умолчанию)
  // SCANPERIOD = 30ms / 0.5ms = 60 (0x3C)
  uint32_t scan_ctrl1 = 0x003C0000; // Битовая структура согласно документации
  if (!this->write_register(REG_SCANCTRL1, scan_ctrl1)) {
    ESP_LOGW(TAG, "Failed to set scan period, using defaults");
  }
  
  // Включение прерываний
  uint32_t irq_en = 0x00000606; // Бит 1 и 2 для приближения, бит 0 для сканирования
  if (!this->write_register(REG_HOSTIRQEN, irq_en)) {
    ESP_LOGW(TAG, "Failed to enable interrupts");
    return false;
  }
  
  return true;
}

bool AW9310XComponent::calibrate_all_channels() {
  ESP_LOGI(TAG, "Starting auto-calibration for all channels...");
  
  // Включение калибровки для всех активных каналов
  uint32_t scan_ctrl0;
  if (!this->read_register(REG_SCANCTRL0, &scan_ctrl0)) {
    return false;
  }
  
  // Установка битов калибровки для каналов 0-2
  uint32_t cali_bits = 0x00000700; // Биты 8-10 для каналов 0-2
  scan_ctrl0 |= cali_bits;
  
  if (!this->write_register(REG_SCANCTRL0, scan_ctrl0)) {
    ESP_LOGE(TAG, "Failed to enable calibration");
    return false;
  }
  
  // Ожидание завершения калибровки (до 200ms)
  delay(150);
  
  // Проверка статуса калибровки
  uint8_t cali_status;
  if (this->read_register_byte(REG_CALI_STATUS, &cali_status)) {
    if (cali_status & 0x07) {
      ESP_LOGI(TAG, "Calibration completed for channels: 0x%02X", cali_status & 0x07);
    } else {
      ESP_LOGW(TAG, "Calibration may have failed - status: 0x%02X", cali_status);
    }
  }
  
  return true;
}

bool AW9310XComponent::calibrate_channel(uint8_t channel) {
  if (channel >= max_channels_) {
    ESP_LOGW(TAG, "Invalid channel %d for calibration", channel);
    return false;
  }
  
  ESP_LOGI(TAG, "Calibrating channel %d...", channel);
  
  uint32_t scan_ctrl0;
  if (!this->read_register(REG_SCANCTRL0, &scan_ctrl0)) {
    return false;
  }
  
  // Установка бита калибровки для конкретного канала
  scan_ctrl0 |= (1 << (8 + channel));
  
  if (!this->write_register(REG_SCANCTRL0, scan_ctrl0)) {
    return false;
  }
  
  delay(100);
  
  return true;
}

bool AW9310XComponent::enable_auto_offset_tuning(bool enable) {
  // AOT включается через специальные биты в конфигурации AFE
  for (uint8_t ch = 0; ch < max_channels_; ch++) {
    uint32_t afecfg1;
    uint16_t reg_addr = REG_AFECFG1_CH0 + (ch * 0x3C); // Шаг 60 байт между каналами
    
    if (!this->read_register(reg_addr, &afecfg1)) {
      return false;
    }
    
    if (enable) {
      afecfg1 |= 0x00000100; // Бит AOT enable
    } else {
      afecfg1 &= ~0x00000100;
    }
    
    if (!this->write_register(reg_addr, afecfg1)) {
      return false;
    }
  }
  
  ESP_LOGI(TAG, "Auto-Offset-Tuning %s", enable ? "enabled" : "disabled");
  return true;
}

bool AW9310XComponent::set_parasitic_capacitance(uint8_t channel, uint16_t cp_value_pf) {
  if (channel >= max_channels_) {
    return false;
  }
  
  // Максимальная компенсация 150pF согласно документации
  if (cp_value_pf > 150) {
    ESP_LOGW(TAG, "Parasitic capacitance %d pF exceeds maximum 150 pF", cp_value_pf);
    cp_value_pf = 150;
  }
  
  // Преобразование pF в значение регистра (0-150 -> 0-255)
  uint8_t reg_value = (cp_value_pf * 255) / 150;
  
  uint16_t reg_addr = REG_CPARA_CH0 + (channel * 0x3C);
  if (!this->write_register_byte(reg_addr, reg_value)) {
    ESP_LOGW(TAG, "Failed to set parasitic capacitance for channel %d", channel);
    return false;
  }
  
  ESP_LOGD(TAG, "Set parasitic capacitance for channel %d: %d pF (reg: 0x%02X)", 
           channel, cp_value_pf, reg_value);
  return true;
}

void AW9310XComponent::set_global_threshold(uint32_t proximity_th, uint32_t touch_th) {
  default_proximity_threshold_ = proximity_th;
  default_touch_threshold_ = touch_th;
  
  // Установка порогов для всех каналов
  uint16_t prox_regs[] = {REG_PROXTH0_CH0, REG_PROXTH0_CH1, REG_PROXTH0_CH2};
  uint16_t touch_regs[] = {REG_TOUCHTH0_CH0, REG_TOUCHTH0_CH1, REG_TOUCHTH0_CH2};
  
  for (int i = 0; i < max_channels_; i++) {
    if (!this->write_register(prox_regs[i], proximity_th)) {
      ESP_LOGW(TAG, "Failed to set proximity threshold for channel %d", i);
    }
    if (!this->write_register(touch_regs[i], touch_th)) {
      ESP_LOGW(TAG, "Failed to set touch threshold for channel %d", i);
    }
  }
  
  ESP_LOGI(TAG, "Set global thresholds - Proximity: %d, Touch: %d", proximity_th, touch_th);
}

void AW9310XComponent::set_channel_sensitivity(uint8_t channel, uint8_t gain, uint8_t resolution) {
  if (channel >= max_channels_) {
    return;
  }
  
  uint16_t reg_addr = REG_AFECFG2_CH0 + (channel * 0x3C);
  uint32_t afecfg2 = 0;
  
  if (!this->read_register(reg_addr, &afecfg2)) {
    ESP_LOGW(TAG, "Failed to read AFE config for channel %d", channel);
    return;
  }
  
  // Настройка усиления (0-7)
  afecfg2 &= ~AFECFG_GAIN_MASK;
  afecfg2 |= (gain & 0x07);
  
  // Настройка разрешения (0-7, 6 = 1fF согласно документации)
  afecfg2 &= ~AFECFG_RESOLUTION_MASK;
  afecfg2 |= ((resolution & 0x07) << 3);
  
  // Настройка частоты сканирования
  afecfg2 &= ~AFECFG_FREQ_MASK;
  afecfg2 |= ((channel_configs_[channel].freq & 0x07) << 6);
  
  if (!this->write_register(reg_addr, afecfg2)) {
    ESP_LOGW(TAG, "Failed to set sensitivity for channel %d", channel);
    return;
  }
  
  ESP_LOGI(TAG, "Channel %d sensitivity: gain=%d, resolution=%d, freq=%d", 
           channel, gain, resolution, channel_configs_[channel].freq);
}

bool AW9310XComponent::read_diff_values(int32_t *diff) {
  uint16_t diff_regs[] = {REG_DIFF_CH0, REG_DIFF_CH1, REG_DIFF_CH2};
  
  for (uint8_t i = 0; i < max_channels_; i++) {
    uint32_t value = 0;
    if (!this->read_register(diff_regs[i], &value)) {
      ESP_LOGW(TAG, "Failed to read diff for channel %d", i);
      return false;
    }
    
    // Преобразование в знаковое значение (32-bit signed)
    diff[i] = (int32_t)value;
    
    // Обновление raw сенсора если есть
    if (raw_sensors_[i] != nullptr) {
      raw_sensors_[i]->publish_state((float)diff[i]);
    }
  }
  
  return true;
}

void AW9310XComponent::set_operation_mode(OperationMode mode) {
  uint32_t cmd_value;
  
  switch (mode) {
    case DEEPSLEEP_MODE:
      cmd_value = CMD_DEEPSLEEP_MODE;
      break;
    case SLEEP_MODE:
      cmd_value = CMD_SLEEP_MODE;
      break;
    case DOZE_MODE:
      cmd_value = CMD_DOZE_MODE;
      break;
    case ACTIVE_MODE:
    default:
      cmd_value = CMD_ACTIVE_MODE;
      break;
  }
  
  if (!this->write_register(REG_CMD, cmd_value)) {
    ESP_LOGE(TAG, "Failed to set operation mode");
    return;
  }
  
  current_mode_ = mode;
  ESP_LOGI(TAG, "Switched to mode: %d", (int)mode);
}

void AW9310XComponent::loop() {
  if (this->interrupt_pin_ != nullptr) {
    // INTN активный LOW (открытый сток)
    bool current_state = this->interrupt_pin_->digital_read();
    
    if (!current_state && last_interrupt_state_) {
      // Прерывание произошло (пин ушел в LOW)
      this->handle_interrupt();
    }
    
    last_interrupt_state_ = current_state;
  } else {
    // Опросный режим
    static uint32_t last_poll = 0;
    uint32_t now = millis();
    uint32_t poll_interval = (current_mode_ == ACTIVE_MODE) ? 30 : 100;
    
    if (now - last_poll > poll_interval) {
      last_poll = now;
      
      int32_t diff[3] = {0};
      if (this->read_diff_values(diff)) {
        this->update_touch_states(diff);
        this->update_proximity_states(diff);
      }
    }
  }
}

void AW9310XComponent::handle_interrupt() {
  uint32_t irq_src = this->read_interrupt_source();
  
  if (irq_src == 0) {
    return;
  }
  
  ESP_LOGD(TAG, "Interrupt triggered: 0x%08lX", (unsigned long)irq_src);
  
  // Чтение данных при любом прерывании
  int32_t diff[3] = {0};
  if (this->read_diff_values(diff)) {
    if (irq_src & 0x06) { // Биты приближения
      this->update_proximity_states(diff);
    }
    this->update_touch_states(diff);
  }
  
  // Автоматическое переключение режимов
  if (auto_mode_switching_) {
    bool any_active = false;
    for (int i = 0; i < max_channels_; i++) {
      if (touch_states_[i] || proximity_states_[i]) {
        any_active = true;
        break;
      }
    }
    
    if (any_active && current_mode_ != ACTIVE_MODE) {
      this->set_operation_mode(ACTIVE_MODE);
    } else if (!any_active && current_mode_ == ACTIVE_MODE) {
      this->set_operation_mode(DOZE_MODE);
    }
  }
}

uint32_t AW9310XComponent::read_interrupt_source() {
  uint32_t irq_src = 0;
  if (!this->read_register(REG_HOSTIRQSRC, &irq_src)) {
    return 0;
  }
  
  // Очистка прерывания происходит при чтении регистра
  return irq_src;
}

void AW9310XComponent::update_touch_states(int32_t *diff) {
  for (uint8_t ch = 0; ch < max_channels_; ch++) {
    if (this->touch_sensors_[ch] != nullptr) {
      bool new_state = (diff[ch] > channel_configs_[ch].touch_threshold);
      
      if (new_state != touch_states_[ch]) {
        touch_states_[ch] = new_state;
        this->touch_sensors_[ch]->publish_state(new_state);
        
        if (new_state) {
          ESP_LOGI(TAG, "Channel %d TOUCH detected (diff=%d, th=%d)", 
                   ch, diff[ch], channel_configs_[ch].touch_threshold);
        } else {
          ESP_LOGD(TAG, "Channel %d released", ch);
        }
      }
    }
  }
}

void AW9310XComponent::update_proximity_states(int32_t *diff) {
  for (uint8_t ch = 0; ch < max_channels_; ch++) {
    if (this->proximity_sensors_[ch] != nullptr) {
      bool new_state = (diff[ch] > channel_configs_[ch].proximity_threshold);
      
      if (new_state != proximity_states_[ch]) {
        proximity_states_[ch] = new_state;
        this->proximity_sensors_[ch]->publish_state(new_state);
        
        if (new_state) {
          ESP_LOGI(TAG, "Channel %d PROXIMITY (diff=%d, th=%d)", 
                   ch, diff[ch], channel_configs_[ch].proximity_threshold);
        }
      }
    }
  }
}

void AW9310XComponent::set_channel_config(uint8_t channel, const ChannelConfig &config) {
  if (channel < max_channels_) {
    channel_configs_[channel] = config;
    enabled_channels_.push_back(channel);
  }
}

void AW9310XComponent::register_channel(AW9310XChannel *sensor, uint8_t channel) {
  if (channel < max_channels_) {
    if (sensor->is_touch()) {
      touch_sensors_[channel] = static_cast<binary_sensor::BinarySensor*>(sensor);
    } else if (sensor->is_proximity()) {
      proximity_sensors_[channel] = static_cast<binary_sensor::BinarySensor*>(sensor);
    }
    ESP_LOGI(TAG, "Registered %s sensor for channel %d", 
             sensor->is_touch() ? "touch" : "proximity", channel);
  }
}

void AW9310XComponent::set_raw_sensor(sensor::Sensor *sensor, uint8_t channel) {
  if (channel < max_channels_) {
    raw_sensors_[channel] = sensor;
  }
}

}  // namespace aw9310x
}  // namespace esphome
