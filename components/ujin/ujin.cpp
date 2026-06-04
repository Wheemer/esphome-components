#include "ujin.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ujin {

static const char *const TAG = "ujin";

void UjinComponent::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Ujin Dimmer Component...");
    rx_buffer_.reserve(MAX_RX_SIZE);
    
    // Загружаем конфигурацию
    load_configuration();
}

uint8_t UjinComponent::calculate_crc(const std::vector<uint8_t> &data) {
    uint8_t crc = 0;
    for (uint8_t byte : data) {
        crc ^= byte;
    }
    return crc;
}

void UjinComponent::send_command(const std::vector<uint8_t> &command) {
    if (!uart_) {
        ESP_LOGE(TAG, "UART not initialized!");
        return;
    }
    
    std::string hex_str;
    for (uint8_t b : command) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02X", b);
        hex_str += hex;
        hex_str += " ";
    }
    ESP_LOGD(TAG, "Sending: %s", hex_str.c_str());
    
    uart_->write_array(command.data(), command.size());
}

void UjinComponent::set_channel_brightness(uint8_t channel, uint8_t brightness) {
    if (channel < 1 || channel > 2) return;
    
    std::vector<uint8_t> message = {0xC8, 0x07};
    
    if (channel == 1) {
        message.push_back(0x30);
    } else if (channel == 2) {
        message.push_back(0x32);
    }
    
    message.push_back(0x02);
    message.push_back(0x00);
    message.push_back(brightness);
    
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    
    send_command(message);
    
    if (channel == 1) {
        state_.channel1_brightness = brightness;
        state_.channel1_on = (brightness > 0);
    } else {
        state_.channel2_brightness = brightness;
        state_.channel2_on = (brightness > 0);
    }
    
    if (state_callback_) {
        state_callback_->trigger(state_);
    }
}

void UjinComponent::set_channel_on(uint8_t channel) {
    set_channel_brightness(channel, 0x64);
}

void UjinComponent::set_channel_off(uint8_t channel) {
    set_channel_brightness(channel, 0x00);
}

void UjinComponent::toggle_channel(uint8_t channel) {
    if (channel == 1) {
        set_channel_brightness(1, state_.channel1_on ? 0x00 : 0x64);
    } else if (channel == 2) {
        set_channel_brightness(2, state_.channel2_on ? 0x00 : 0x64);
    }
}

void UjinComponent::beep(bool long_beep) {
    if (!state_.buzzer.enabled) return;
    
    uint8_t duration = long_beep ? state_.buzzer.long_beep_duration : state_.buzzer.short_beep_duration;
    std::vector<uint8_t> message = {0xC8, 0x07, 0x0B, 0x02, 0x00, duration};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
}

void UjinComponent::play_custom_beep(uint8_t duration) {
    if (!state_.buzzer.enabled) return;
    
    std::vector<uint8_t> message = {0xC8, 0x07, 0x0B, 0x02, 0x00, duration};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
}

void UjinComponent::set_buzzer_enabled(bool enabled) {
    state_.buzzer.enabled = enabled;
    save_configuration();
}

void UjinComponent::set_buzzer_volume(uint8_t volume) {
    if (volume > 100) volume = 100;
    state_.buzzer.volume = volume;
    save_configuration();
}

void UjinComponent::set_dimming_enabled(uint8_t channel, bool enabled) {
    if (channel < 1 || channel > 2) return;
    
    std::vector<uint8_t> message = {0xC8, 0x07};
    
    if (channel == 1) {
        message.push_back(enabled ? 0x34 : 0x35);
        message.push_back(0x02);
        message.push_back(0x00);
    } else if (channel == 2) {
        message.push_back(enabled ? 0x34 : 0x35);
        message.push_back(0x02);
        message.push_back(0x01);
    }
    
    message.push_back(0x64);
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    
    send_command(message);
    
    if (channel == 1) {
        state_.dimmer1_enabled = enabled;
    } else {
        state_.dimmer2_enabled = enabled;
    }
    
    if (settings_callback_) {
        settings_callback_->trigger(state_);
    }
}

bool UjinComponent::get_dimming_enabled(uint8_t channel) {
    return (channel == 1) ? state_.dimmer1_enabled : state_.dimmer2_enabled;
}

void UjinComponent::set_operation_mode(uint8_t mode) {
    if (mode > 3) mode = 0;
    state_.settings.operation_mode = mode;
    
    std::vector<uint8_t> message = {0xC8, 0x07, 0x40, 0x02, 0x00, mode};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
    
    save_configuration();
    
    if (settings_callback_) {
        settings_callback_->trigger(state_);
    }
}

void UjinComponent::set_dimming_mode(uint8_t mode) {
    if (mode > 3) mode = 0;
    state_.settings.dimming_mode = mode;
    
    switch(mode) {
        case 0:
            set_dimming_enabled(1, true);
            set_dimming_enabled(2, true);
            break;
        case 1:
            set_dimming_enabled(1, true);
            set_dimming_enabled(2, false);
            break;
        case 2:
            set_dimming_enabled(1, false);
            set_dimming_enabled(2, true);
            break;
        case 3:
            set_dimming_enabled(1, false);
            set_dimming_enabled(2, false);
            break;
    }
    
    save_configuration();
    
    if (settings_callback_) {
        settings_callback_->trigger(state_);
    }
}

void UjinComponent::set_touch_sensitivity(uint8_t sensitivity) {
    if (sensitivity > 2) sensitivity = 1;
    state_.settings.touch_sensitivity = sensitivity;
    
    uint16_t threshold = (sensitivity == 0) ? 600 : (sensitivity == 1) ? 1000 : 1400;
    std::vector<uint8_t> message = {0xC8, 0x07, 0x41, 0x02, 
        static_cast<uint8_t>(threshold >> 8), 
        static_cast<uint8_t>(threshold & 0xFF)};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
    
    save_configuration();
    
    if (settings_callback_) {
        settings_callback_->trigger(state_);
    }
}

void UjinComponent::set_timer(uint8_t channel, uint16_t seconds) {
    if (channel == 1) {
        state_.timer1 = seconds;
    } else {
        state_.timer2 = seconds;
    }
    save_configuration();
}

void UjinComponent::set_brightness_limits(uint8_t channel, uint16_t min_val, uint16_t max_val) {
    if (min_val > max_val) std::swap(min_val, max_val);
    if (max_val > 100) max_val = 100;
    
    if (channel == 1) {
        state_.min_brightness1 = min_val;
        state_.max_brightness1 = max_val;
    } else {
        state_.min_brightness2 = min_val;
        state_.max_brightness2 = max_val;
    }
    save_configuration();
}

void UjinComponent::save_configuration() {
    std::vector<uint8_t> message = {
        0xC8, 0x29, 0x66, 0x24,
        state_.power_phase_only ? 0xF0 : 0xF1,
        static_cast<uint8_t>(state_.timer1 >> 8),
        static_cast<uint8_t>(state_.timer1 & 0xFF),
        static_cast<uint8_t>(state_.timer2 >> 8),
        static_cast<uint8_t>(state_.timer2 & 0xFF),
        static_cast<uint8_t>(state_.max_brightness1 >> 8),
        static_cast<uint8_t>(state_.max_brightness1 & 0xFF),
        static_cast<uint8_t>(state_.min_brightness1 >> 8),
        static_cast<uint8_t>(state_.min_brightness1 & 0xFF),
        static_cast<uint8_t>(state_.max_brightness2 >> 8),
        static_cast<uint8_t>(state_.max_brightness2 & 0xFF),
        static_cast<uint8_t>(state_.min_brightness2 >> 8),
        static_cast<uint8_t>(state_.min_brightness2 & 0xFF),
        state_.dimmer1_enabled ? 0xC5 : 0xC4,
        0xFF, 0xCA, 0x08,
    };
    
    // Добавляем остальную конфигурацию
    for (int i = 0; i < 16; i++) {
        message.push_back(0x00);
    }
    
    message.push_back(state_.settings.operation_mode);
    message.push_back(state_.settings.dimming_mode);
    message.push_back(state_.buzzer.enabled ? 0x01 : 0x00);
    message.push_back(state_.buzzer.volume);
    
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    
    send_command(message);
    ESP_LOGI(TAG, "Configuration saved");
}

void UjinComponent::load_configuration() {
    std::vector<uint8_t> message = {0xC8, 0x07, 0x50, 0x02, 0x00, 0x00};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
    ESP_LOGI(TAG, "Requesting configuration");
}

void UjinComponent::loop() {
    if (!uart_) return;
    
    while (uart_->available()) {
        uint8_t byte;
        if (uart_->read_byte(&byte)) {
            rx_buffer_.push_back(byte);
            
            if (rx_buffer_.size() > 1 && rx_buffer_[0] != 0xC8) {
                rx_buffer_.clear();
                continue;
            }
            
            if (rx_buffer_.size() >= 7) {
                std::vector<uint8_t> data(rx_buffer_.begin(), rx_buffer_.end() - 1);
                uint8_t received_crc = rx_buffer_.back();
                uint8_t calc_crc = calculate_crc(data);
                
                if (calc_crc == received_crc) {
                    parse_response(rx_buffer_);
                    rx_buffer_.clear();
                } else if (rx_buffer_.size() > MAX_RX_SIZE) {
                    ESP_LOGW(TAG, "RX buffer overflow, clearing");
                    rx_buffer_.clear();
                }
            }
        }
    }
}

void UjinComponent::parse_response(const std::vector<uint8_t> &data) {
    if (data.size() < 7) return;
    
    std::string hex_str;
    for (uint8_t b : data) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02X", b);
        hex_str += hex;
        hex_str += " ";
    }
    ESP_LOGD(TAG, "Received: %s", hex_str.c_str());
    
    bool state_changed = false;
    bool settings_changed = false;
    
    if (data.size() >= 42) {
        if (data.size() > 17) {
            bool dimmer1 = (data[17] == 0xC5);
            if (state_.dimmer1_enabled != dimmer1) {
                state_.dimmer1_enabled = dimmer1;
                settings_changed = true;
            }
        }
        
        if (data.size() > 32) {
            bool dimmer2 = (data[32] == 0xC5);
            if (state_.dimmer2_enabled != dimmer2) {
                state_.dimmer2_enabled = dimmer2;
                settings_changed = true;
            }
        }
        
        if (data.size() > 3) {
            bool phase_only = (data[3] == 0xF0);
            if (state_.power_phase_only != phase_only) {
                state_.power_phase_only = phase_only;
                settings_changed = true;
            }
        }
        
        if (data.size() > 38) {
            if (state_.external_input_state != data[38]) {
                state_.external_input_state = data[38];
                state_changed = true;
            }
        }
        
        if (settings_changed && settings_callback_) {
            settings_callback_->trigger(state_);
        }
    }
    
    if (state_changed && state_callback_) {
        state_callback_->trigger(state_);
    }
}

void UjinComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "Ujin Dimmer Component:");
    ESP_LOGCONFIG(TAG, "  Channel 1: %s, Brightness: %d%%", 
                  state_.channel1_on ? "ON" : "OFF", 
                  (state_.channel1_brightness * 100) / 100);
    ESP_LOGCONFIG(TAG, "  Channel 2: %s, Brightness: %d%%", 
                  state_.channel2_on ? "ON" : "OFF", 
                  (state_.channel2_brightness * 100) / 100);
    ESP_LOGCONFIG(TAG, "  Buzzer: %s, Volume: %d%%", 
                  state_.buzzer.enabled ? "enabled" : "disabled",
                  state_.buzzer.volume);
}

} // namespace ujin
} // namespace esphome
