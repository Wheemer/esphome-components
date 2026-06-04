#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include <vector>
#include <functional>
#include <memory>

namespace esphome {
namespace ujin {

struct DeviceState {
    uint8_t channel1_brightness = 0;
    uint8_t channel2_brightness = 0;
    bool dimmer1_enabled = true;
    bool dimmer2_enabled = true;
    bool channel1_on = false;
    bool channel2_on = false;
    uint8_t power_mode = 0xF1;
    bool power_phase_only = false;
    
    struct {
        bool enabled = true;
        uint8_t short_beep_duration = 0xFA;
        uint8_t long_beep_duration = 0xFB;
        uint8_t volume = 100;
    } buzzer;
    
    struct {
        uint8_t operation_mode = 0;
        uint8_t dimming_mode = 0;
        uint8_t touch_sensitivity = 1;
    } settings;
    
    uint16_t timer1 = 0;
    uint16_t timer2 = 0;
    uint16_t min_brightness1 = 0;
    uint16_t max_brightness1 = 100;
    uint16_t min_brightness2 = 0;
    uint16_t max_brightness2 = 100;
    uint8_t external_input_state = 0x18;
};

class UjinComponent : public Component, public uart::UARTDevice {
public:
    void setup() override;
    void loop() override;
    void dump_config() override;
    
    void set_uart_parent(uart::UARTComponent *uart) { uart_ = uart; }
    
    // Команды для диммеров
    void set_channel_brightness(uint8_t channel, uint8_t brightness);
    void set_channel_on(uint8_t channel);
    void set_channel_off(uint8_t channel);
    void toggle_channel(uint8_t channel);
    
    // Зуммер
    void beep(bool long_beep = false);
    void set_buzzer_enabled(bool enabled);
    void set_buzzer_volume(uint8_t volume);
    void play_custom_beep(uint8_t duration);
    
    // Настройки
    void set_operation_mode(uint8_t mode);
    void set_dimming_mode(uint8_t mode);
    void set_touch_sensitivity(uint8_t sensitivity);
    void set_dimming_enabled(uint8_t channel, bool enabled);
    bool get_dimming_enabled(uint8_t channel);
    
    // Таймеры и границы
    void set_timer(uint8_t channel, uint16_t seconds);
    void set_brightness_limits(uint8_t channel, uint16_t min_val, uint16_t max_val);
    
    // Сохранение/загрузка конфигурации
    void save_configuration();
    void load_configuration();
    
    // Получение состояния
    const DeviceState &get_state() const { return state_; }
    
    // Callbacks
    Trigger<const DeviceState &> *get_state_callback() const { return state_callback_; }
    Trigger<const DeviceState &> *get_settings_callback() const { return settings_callback_; }
    
protected:
    uart::UARTComponent *uart_{nullptr};
    DeviceState state_;
    Trigger<const DeviceState &> *state_callback_{new Trigger<const DeviceState &>()};
    Trigger<const DeviceState &> *settings_callback_{new Trigger<const DeviceState &>()};
    
    uint8_t calculate_crc(const std::vector<uint8_t> &data);
    void send_command(const std::vector<uint8_t> &command);
    void parse_response(const std::vector<uint8_t> &data);
    
    std::vector<uint8_t> rx_buffer_;
    static constexpr size_t MAX_RX_SIZE = 128;
};

} // namespace ujin
} // namespace esphome
