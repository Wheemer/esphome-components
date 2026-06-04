#include "esphome.h"
#include "esphome/components/water_heater/water_heater.h"
#include "esphome/components/uart/uart.h"
#include <vector>

namespace esphome {
namespace polaris_kettle {

class PolarisKettle : public water_heater::WaterHeater, public Component, public uart::UARTDevice {
 public:
  PolarisKettle() = default;
  
  void set_uart_parent(uart::UARTComponent *parent) {
    this->parent_ = parent;
    this->set_uart(parent);
  }
  
  void setup() override {
    ESP_LOGI("polaris", "Polaris Kettle Component Started");
    
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
  
  void loop() override {
    while (this->available()) {
      uint8_t data;
      this->read_byte(&data);
      buffer_.push_back(data);
      
      // Ищем префикс 0x81
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
  
  void control(const water_heater::WaterHeaterCall &call) override {
    bool updated = false;
    
    if (call.get_target_temperature().has_value()) {
      float new_temp = *call.get_target_temperature();
      this->target_temperature = new_temp;
      send_target_temperature((uint8_t)new_temp);
      updated = true;
    }
    
    if (call.get_mode().has_value()) {
      this->mode = *call.get_mode();
      send_mode(this->mode);
      updated = true;
    }
    
    if (updated) {
      this->publish_state();
    }
  }
  
  void publish_state() override {
    this->current_temperature = current_temp_;
    this->water_heater::WaterHeater::publish_state();
  }
  
  // Команды для чайной церемонии
  void black_tea() { send_preset(95, 0x3D); }      // черный чай / кофе / пауэр 95
  void mix_tea()   { send_preset(40, 0x3B); }      // смесь 40
  void white_tea() { send_preset(65, 0x3C); }      // белый 65
  void green_tea() { send_preset(80, 0x7C); }      // зеленый цветочный 80
  void oolong_tea(){ send_preset(90, 0x7D); }      // красный улун травяной 90
  void bag_tea()   { send_preset(100, 0x7E); }     // пакетированный 100
  void boil()      { send_preset(100, 0x3B); }     // кипячение
  void keep_warm() { send_preset(40, 0x63); }      // разогрев с удержанием
  
  // Вкл/Выкл
  void turn_on()  { send_power(0x00, 0x01, 70, 0x3C); }
  void turn_off() { send_power(0x00, 0x00, 0x00, 0x00); }

  // Дополнительные методы для Home Assistant
  void set_temperature(float temp) {
    send_target_temperature((uint8_t)temp);
  }
  
  void start_boil() {
    boil();
  }
  
  void start_keep_warm() {
    keep_warm();
  }

 protected:
  uart::UARTComponent *parent_{nullptr};
  std::vector<uint8_t> buffer_;
  float current_temp_ = 0.0;
  bool has_error_ = false;
  bool no_water_ = false;
  
  void process_frame() {
    if (buffer_.size() < 8) return;
    uint8_t *frame = buffer_.data();
    
    if (frame[0] != 0x81) return;
    
    // Проверка контрольной суммы (сумма байт 0-5)
    uint16_t calc_sum = 0;
    for (int i = 0; i < 6; i++) calc_sum += frame[i];
    uint16_t recv_sum = (frame[6] << 8) | frame[7];
    
    if (calc_sum != recv_sum) {
      ESP_LOGW("polaris", "Checksum error: calc=0x%04X recv=0x%04X", calc_sum, recv_sum);
      return;
    }
    
    uint8_t status = frame[1];   // 0x00=норма, 0x01=вкл?, 0xFF=ошибка
    uint8_t mode = frame[2];     // 0x00=сон, 0x01=ожидание, 0x03=сопряжение
    uint8_t target_temp = frame[3];
    uint8_t current_temp = frame[4];
    
    current_temp_ = (float)current_temp;
    this->target_temperature = (float)target_temp;
    
    // Обработка ошибок
    if (status == 0xFF) {
      has_error_ = true;
      if (frame[3] == 0x64 && frame[4] == 0x55) {
        no_water_ = true;
        ESP_LOGW("polaris", "Error: No water in kettle!");
      } else if (frame[3] == 0x64 && frame[4] == 0x00) {
        ESP_LOGW("polaris", "Error: No kettle detected!");
      } else {
        ESP_LOGW("polaris", "Error: Unknown error (status=0xFF)");
      }
      this->mode = water_heater::WATER_HEATER_MODE_OFF;
    } else {
      has_error_ = false;
      no_water_ = false;
      
      // Определяем режим работы
      if (mode == 0x00) {
        this->mode = water_heater::WATER_HEATER_MODE_OFF;
      } else if (mode == 0x01) {
        // В режиме ожидания определяем подрежим по байту 4
        if (frame[4] == 0x3B || frame[4] == 0x7B) {
          this->mode = water_heater::WATER_HEATER_MODE_PERFORMANCE; // кипячение
        } else if (frame[4] == 0x63 || frame[4] == 0x3C || frame[4] == 0x3D) {
          this->mode = water_heater::WATER_HEATER_MODE_ECO; // подогрев
        } else if (frame[4] == 0x7C || frame[4] == 0x7D || frame[4] == 0x7E) {
          this->mode = water_heater::WATER_HEATER_MODE_GAS; // чайная церемония
        } else {
          this->mode = water_heater::WATER_HEATER_MODE_GAS;
        }
      } else {
        this->mode = water_heater::WATER_HEATER_MODE_GAS;
      }
    }
    
    this->publish_state();
    
    ESP_LOGD("polaris", "Status: %s | Mode: %d | Temp: %.1f°C / %.1f°C | Error: %s",
             status == 0xFF ? "ERROR" : "OK", mode, current_temp_, target_temp,
             no_water_ ? "NO WATER" : (has_error_ ? "NO KETTLE" : "OK"));
  }
  
  uint16_t calculate_checksum(uint8_t *data, int len) {
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) sum += data[i];
    return sum;
  }
  
  void send_command(uint8_t byte1, uint8_t byte2, uint8_t byte3, 
                    uint8_t byte4, uint8_t byte5) {
    uint8_t cmd[8] = {0x81, byte1, byte2, byte3, byte4, byte5, 0x00, 0x00};
    uint16_t sum = calculate_checksum(cmd, 6);
    cmd[6] = (sum >> 8) & 0xFF;
    cmd[7] = sum & 0xFF;
    this->write_array(cmd, 8);
    
    ESP_LOGD("polaris", "Send: %02X %02X %02X %02X %02X %02X %02X %02X",
             cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
  }
  
  void send_preset(uint8_t temp, uint8_t mode_byte) {
    // byte1: 0x00=норма, byte2: 0x01=ожидание
    send_command(0x00, 0x01, temp, mode_byte, 0x00);
    ESP_LOGI("polaris", "Send preset: temp=%d°C mode_byte=0x%02X", temp, mode_byte);
  }
  
  void send_power(uint8_t status, uint8_t mode, uint8_t temp, uint8_t flag) {
    send_command(status, mode, temp, flag, 0x00);
  }
  
  void send_target_temperature(uint8_t temp) {
    send_command(0x00, 0x01, temp, 0x3C, 0x00);
    ESP_LOGI("polaris", "Set target temperature: %d°C", temp);
  }
  
  void send_mode(water_heater::WaterHeaterMode mode) {
    switch (mode) {
      case water_heater::WATER_HEATER_MODE_OFF:
        send_command(0x00, 0x00, 0x00, 0x00, 0x00);
        ESP_LOGI("polaris", "Turn OFF");
        break;
      case water_heater::WATER_HEATER_MODE_PERFORMANCE:
        send_preset(100, 0x3B);  // кипячение
        ESP_LOGI("polaris", "Mode: PERFORMANCE (Boil)");
        break;
      case water_heater::WATER_HEATER_MODE_ECO:
        send_preset(40, 0x63);   // подогрев
        ESP_LOGI("polaris", "Mode: ECO (Keep warm)");
        break;
      case water_heater::WATER_HEATER_MODE_GAS:
        send_preset((uint8_t)this->target_temperature, 0x3C);
        ESP_LOGI("polaris", "Mode: GAS (Tea ceremony, %.1f°C)", this->target_temperature);
        break;
      default:
        send_preset((uint8_t)this->target_temperature, 0x3C);
        break;
    }
  }
};

}  // namespace polaris_kettle
}  // namespace esphome
