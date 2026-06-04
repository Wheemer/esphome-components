#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/automation.h"
#include <vector>
#include <functional>
#include <memory>

namespace esphome {
namespace ujin {

struct DeviceState {
    // Состояние каналов
    uint8_t channel1_brightness = 0;
    uint8_t channel2_brightness = 0;
    bool channel1_on = false;
    bool channel2_on = false;
    
    // Состояние диммирования
    bool dimmer1_enabled = true;
    bool dimmer2_enabled = true;
    
    // Режимы
    bool power_phase_only = false;
    uint8_t operation_mode = 0;
    uint8_t dimming_mode = 0;
    uint8_t touch_sensitivity = 1;
    
    // Настройки зуммера
    struct {
        bool enabled = true;
        uint8_t short_beep = 0xFA;
        uint8_t long_beep = 0xFB;
    } buzzer;
    
    // Пределы яркости
    uint16_t min_brightness1 = 0;
    uint16_t max_brightness1 = 100;
    uint16_t min_brightness2 = 0;
    uint16_t max_brightness2 = 100;
    
    // Таймеры
    uint16_t timer1 = 0;
    uint16_t timer2 = 0;
    
    // Внешние входы: 0x18-ничего, 0x1A-вход1, 0x1C-вход2, 0x1E-оба
    uint8_t external_input = 0x18;
};

class UjinComponent : public Component, public uart::UARTDevice {
public:
    void setup() override;
    void loop() override;
    void dump_config() override;
    
    void set_uart_parent(uart::UARTComponent *uart) { uart_ = uart; }
    
    // Короткие команды (7 байт)
    void set_channel_brightness(uint8_t channel, uint8_t brightness_percent);
    void set_channel_on(uint8_t channel);
    void set_channel_off(uint8_t channel);
    void toggle_channel(uint8_t channel);
    void set_both_brightness(uint8_t brightness_percent);
    
    // Зуммер
    void beep_short();
    void beep_long();
    void beep_custom(uint8_t duration);
    
    // Управление диммированием
    void start_dimming(uint8_t channel);
    void stop_dimming(uint8_t channel);
    
    // Длинные команды
    void save_full_config();
    void set_power_mode(bool phase_only);
    void set_timer(uint8_t channel, uint16_t seconds);
    void set_brightness_limits(uint8_t channel, uint16_t min_val, uint16_t max_val);
    void set_dimming_enabled(uint8_t channel, bool enabled);
    void set_operation_mode(uint8_t mode);
    void set_dimming_mode(uint8_t mode);
    void set_touch_sensitivity(uint8_t sensitivity);
    void request_config();
    
    // Получение состояния
    const DeviceState &get_state() const { return state_; }
    
    // Для бинарных сенсоров
    bool get_external_input_1() const { return state_.external_input == 0x1A || state_.external_input == 0x1E; }
    bool get_external_input_2() const { return state_.external_input == 0x1C || state_.external_input == 0x1E; }
    bool get_any_external_input() const { return state_.external_input != 0x18; }
    bool get_dimmer1_enabled() const { return state_.dimmer1_enabled; }
    bool get_dimmer2_enabled() const { return state_.dimmer2_enabled; }
    bool get_channel1_on() const { return state_.channel1_on; }
    bool get_channel2_on() const { return state_.channel2_on; }
    bool get_power_phase_only() const { return state_.power_phase_only; }
    
    // Регистрация сенсоров
    void register_external_input_1_sensor(binary_sensor::BinarySensor *sensor) { ext_input1_sensor_ = sensor; }
    void register_external_input_2_sensor(binary_sensor::BinarySensor *sensor) { ext_input2_sensor_ = sensor; }
    void register_any_external_input_sensor(binary_sensor::BinarySensor *sensor) { any_ext_input_sensor_ = sensor; }
    void register_dimmer1_enabled_sensor(binary_sensor::BinarySensor *sensor) { dimmer1_enabled_sensor_ = sensor; }
    void register_dimmer2_enabled_sensor(binary_sensor::BinarySensor *sensor) { dimmer2_enabled_sensor_ = sensor; }
    void register_channel1_on_sensor(binary_sensor::BinarySensor *sensor) { channel1_on_sensor_ = sensor; }
    void register_channel2_on_sensor(binary_sensor::BinarySensor *sensor) { channel2_on_sensor_ = sensor; }
    void register_power_phase_only_sensor(binary_sensor::BinarySensor *sensor) { power_phase_only_sensor_ = sensor; }
    
    // Callback
    Trigger<const DeviceState &> *get_state_callback() const { return state_callback_; }
    
protected:
    uart::UARTComponent *uart_{nullptr};
    DeviceState state_;
    Trigger<const DeviceState &> *state_callback_{new Trigger<const DeviceState &>()};
    
    // Сенсоры
    binary_sensor::BinarySensor *ext_input1_sensor_{nullptr};
    binary_sensor::BinarySensor *ext_input2_sensor_{nullptr};
    binary_sensor::BinarySensor *any_ext_input_sensor_{nullptr};
    binary_sensor::BinarySensor *dimmer1_enabled_sensor_{nullptr};
    binary_sensor::BinarySensor *dimmer2_enabled_sensor_{nullptr};
    binary_sensor::BinarySensor *channel1_on_sensor_{nullptr};
    binary_sensor::BinarySensor *channel2_on_sensor_{nullptr};
    binary_sensor::BinarySensor *power_phase_only_sensor_{nullptr};
    
    uint8_t calculate_crc(const std::vector<uint8_t> &data);
    void send_command(const std::vector<uint8_t> &command);
    void parse_short_response(const std::vector<uint8_t> &data);
    void parse_long_response(const std::vector<uint8_t> &data);
    void parse_debug_log(const std::vector<uint8_t> &data);
    void update_binary_sensors();
    
    std::vector<uint8_t> rx_buffer_;
    static constexpr size_t MAX_RX_SIZE = 128;
};

} // namespace ujin
} // namespace esphome
