#include "ujin.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ujin {

static const char *const TAG = "ujin";

void UjinComponent::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Ujin Dimmer Component...");
    rx_buffer_.reserve(MAX_RX_SIZE);
    request_config();
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

void UjinComponent::set_channel_brightness(uint8_t channel, uint8_t brightness_percent) {
    if (channel < 1 || channel > 2) return;
    if (brightness_percent > 100) brightness_percent = 100;
    
    std::vector<uint8_t> message = {0xC8, 0x07};
    uint8_t cmd = (channel == 1) ? 0x30 : 0x32;
    message.push_back(cmd);
    message.push_back(0x02);
    message.push_back(0x00);
    message.push_back(brightness_percent);
    
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    
    send_command(message);
    
    if (channel == 1) {
        state_.channel1_brightness = brightness_percent;
        state_.channel1_on = (brightness_percent > 0);
    } else {
        state_.channel2_brightness = brightness_percent;
        state_.channel2_on = (brightness_percent > 0);
    }
    
    update_binary_sensors();
    state_callback_->trigger(state_);
}

void UjinComponent::set_both_brightness(uint8_t brightness_percent) {
    if (brightness_percent > 100) brightness_percent = 100;
    
    std::vector<uint8_t> message = {0xC8, 0x07, 0x33, 0x02, 0x00, brightness_percent};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    
    send_command(message);
    
    state_.channel1_brightness = brightness_percent;
    state_.channel2_brightness = brightness_percent;
    state_.channel1_on = (brightness_percent > 0);
    state_.channel2_on = (brightness_percent > 0);
    
    update_binary_sensors();
    state_callback_->trigger(state_);
}

void UjinComponent::set_channel_on(uint8_t channel) {
    set_channel_brightness(channel, 100);
}

void UjinComponent::set_channel_off(uint8_t channel) {
    set_channel_brightness(channel, 0);
}

void UjinComponent::toggle_channel(uint8_t channel) {
    if (channel == 1) {
        set_channel_brightness(1, state_.channel1_on ? 0 : 100);
    } else if (channel == 2) {
        set_channel_brightness(2, state_.channel2_on ? 0 : 100);
    }
}

void UjinComponent::beep_short() {
    beep_custom(0xFA);
}

void UjinComponent::beep_long() {
    beep_custom(0xFB);
}

void UjinComponent::beep_custom(uint8_t duration) {
    if (!state_.buzzer.enabled) return;
    
    std::vector<uint8_t> message = {0xC8, 0x07, 0x0B, 0x02, 0x00, duration};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
}

void UjinComponent::start_dimming(uint8_t channel) {
    std::vector<uint8_t> message = {0xC8, 0x07};
    
    if (channel == 1) {
        message.push_back(0x34);
        message.push_back(0x02);
        message.push_back(0x00);
    } else if (channel == 2) {
        message.push_back(0x34);
        message.push_back(0x02);
        message.push_back(0x01);
    } else if (channel == 3) {
        message.push_back(0x36);
        message.push_back(0x02);
        message.push_back(0x00);
    } else {
        return;
    }
    
    message.push_back(0x64);
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    
    send_command(message);
}

void UjinComponent::stop_dimming(uint8_t channel) {
    std::vector<uint8_t> message = {0xC8, 0x07};
    
    if (channel == 1) {
        message.push_back(0x35);
        message.push_back(0x02);
        message.push_back(0x00);
    } else if (channel == 2) {
        message.push_back(0x35);
        message.push_back(0x02);
        message.push_back(0x01);
    } else if (channel == 3) {
        message.push_back(0x37);
        message.push_back(0x02);
        message.push_back(0x00);
    } else {
        return;
    }
    
    message.push_back(0x64);
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    
    send_command(message);
}

void UjinComponent::request_config() {
    std::vector<uint8_t> message = {0xC8, 0x07, 0x50, 0x02, 0x00, 0x00};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
    ESP_LOGI(TAG, "Requested configuration from device");
}

void UjinComponent::save_full_config() {
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
        0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00,
        state_.dimmer1_enabled ? 0xC5 : 0xC4,
        0xFF, 0xCA, 0x08,
        static_cast<uint8_t>(state_.min_brightness1 >> 8),
        static_cast<uint8_t>(state_.min_brightness1 & 0xFF),
        0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00,
        state_.dimmer2_enabled ? 0xC5 : 0xC4,
        0xFF, 0xF4, 0x01, 0x00, 0x00, 0x34, 0x12
    };
    
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    
    send_command(message);
    ESP_LOGI(TAG, "Full configuration saved");
}

void UjinComponent::set_power_mode(bool phase_only) {
    state_.power_phase_only = phase_only;
    save_full_config();
    update_binary_sensors();
}

void UjinComponent::set_timer(uint8_t channel, uint16_t seconds) {
    if (seconds > 65535) seconds = 65535;
    if (channel == 1) {
        state_.timer1 = seconds;
    } else if (channel == 2) {
        state_.timer2 = seconds;
    }
    save_full_config();
}

void UjinComponent::set_brightness_limits(uint8_t channel, uint16_t min_val, uint16_t max_val) {
    if (min_val > max_val) std::swap(min_val, max_val);
    if (max_val > 10000) max_val = 10000;
    
    if (channel == 1) {
        state_.min_brightness1 = min_val;
        state_.max_brightness1 = max_val;
    } else {
        state_.min_brightness2 = min_val;
        state_.max_brightness2 = max_val;
    }
    save_full_config();
}

void UjinComponent::set_dimming_enabled(uint8_t channel, bool enabled) {
    if (channel == 1) {
        state_.dimmer1_enabled = enabled;
    } else if (channel == 2) {
        state_.dimmer2_enabled = enabled;
    } else if (channel == 3) {
        state_.dimmer1_enabled = enabled;
        state_.dimmer2_enabled = enabled;
    }
    save_full_config();
    update_binary_sensors();
}

void UjinComponent::set_operation_mode(uint8_t mode) {
    if (mode > 3) mode = 0;
    state_.operation_mode = mode;
    
    std::vector<uint8_t> message = {0xC8, 0x07, 0x40, 0x02, 0x00, mode};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
    
    save_full_config();
}

void UjinComponent::set_dimming_mode(uint8_t mode) {
    if (mode > 3) mode = 0;
    state_.dimming_mode = mode;
    
    switch(mode) {
        case 0:
            state_.dimmer1_enabled = true;
            state_.dimmer2_enabled = true;
            break;
        case 1:
            state_.dimmer1_enabled = true;
            state_.dimmer2_enabled = false;
            break;
        case 2:
            state_.dimmer1_enabled = false;
            state_.dimmer2_enabled = true;
            break;
        case 3:
            state_.dimmer1_enabled = false;
            state_.dimmer2_enabled = false;
            break;
    }
    save_full_config();
    update_binary_sensors();
}

void UjinComponent::set_touch_sensitivity(uint8_t sensitivity) {
    if (sensitivity > 2) sensitivity = 1;
    state_.touch_sensitivity = sensitivity;
    
    uint8_t threshold = (sensitivity == 0) ? 0x64 : (sensitivity == 1) ? 0xC8 : 0xFF;
    std::vector<uint8_t> message = {0xC8, 0x07, 0x41, 0x02, 0x00, threshold};
    uint8_t crc = calculate_crc(message);
    message.push_back(crc);
    send_command(message);
    
    save_full_config();
}

void UjinComponent::update_binary_sensors() {
    if (ext_input1_sensor_) ext_input1_sensor_->publish_state(get_external_input_1());
    if (ext_input2_sensor_) ext_input2_sensor_->publish_state(get_external_input_2());
    if (any_ext_input_sensor_) any_ext_input_sensor_->publish_state(get_any_external_input());
    if (dimmer1_enabled_sensor_) dimmer1_enabled_sensor_->publish_state(get_dimmer1_enabled());
    if (dimmer2_enabled_sensor_) dimmer2_enabled_sensor_->publish_state(get_dimmer2_enabled());
    if (channel1_on_sensor_) channel1_on_sensor_->publish_state(get_channel1_on());
    if (channel2_on_sensor_) channel2_on_sensor_->publish_state(get_channel2_on());
    if (power_phase_only_sensor_) power_phase_only_sensor_->publish_state(get_power_phase_only());
}

void UjinComponent::loop() {
    if (!uart_) return;
    
    while (uart_->available()) {
        uint8_t byte;
        if (uart_->read_byte(&byte)) {
            rx_buffer_.push_back(byte);
            
            if (rx_buffer_.size() > 1 && rx_buffer_[0] != 0xC8 && rx_buffer_[0] != 0x02) {
                rx_buffer_.clear();
                continue;
            }
            
            if (rx_buffer_.size() >= 7 && rx_buffer_[0] == 0xC8) {
                std::vector<uint8_t> data(rx_buffer_.begin(), rx_buffer_.end() - 1);
                uint8_t received_crc = rx_buffer_.back();
                uint8_t calc_crc = calculate_crc(data);
                
                if (calc_crc == received_crc) {
                    parse_short_response(rx_buffer_);
                    rx_buffer_.clear();
                } else if (rx_buffer_.size() > MAX_RX_SIZE) {
                    rx_buffer_.clear();
                }
            }
            else if (rx_buffer_.size() >= 10 && rx_buffer_[0] == 0x02) {
                parse_debug_log(rx_buffer_);
                rx_buffer_.clear();
            }
            else if (rx_buffer_.size() >= 41 && rx_buffer_[0] == 0xC8 && rx_buffer_[1] == 0x29) {
                if (rx_buffer_.size() == 41) {
                    parse_long_response(rx_buffer_);
                    rx_buffer_.clear();
                } else if (rx_buffer_.size() > 60) {
                    rx_buffer_.clear();
                }
            }
        }
    }
}

void UjinComponent::parse_short_response(const std::vector<uint8_t> &data) {
    if (data.size() < 7) return;
    
    ESP_LOGD(TAG, "Short response: %02X %02X %02X %02X %02X %02X %02X",
             data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
    
    uint8_t cmd = data[2];
    uint8_t arg = data[5];
    
    if (cmd == 0x30) {
        state_.channel1_brightness = arg;
        state_.channel1_on = (arg > 0);
        update_binary_sensors();
        state_callback_->trigger(state_);
    } else if (cmd == 0x32) {
        state_.channel2_brightness = arg;
        state_.channel2_on = (arg > 0);
        update_binary_sensors();
        state_callback_->trigger(state_);
    } else if (cmd == 0x33) {
        state_.channel1_brightness = arg;
        state_.channel2_brightness = arg;
        state_.channel1_on = (arg > 0);
        state_.channel2_on = (arg > 0);
        update_binary_sensors();
        state_callback_->trigger(state_);
    }
}

void UjinComponent::parse_long_response(const std::vector<uint8_t> &data) {
    ESP_LOGD(TAG, "Long response (%d bytes)", data.size());
    
    if (data.size() > 18) state_.dimmer1_enabled = (data[18] == 0xC5);
    if (data.size() > 32) state_.dimmer2_enabled = (data[32] == 0xC5);
    if (data.size() > 4) state_.power_phase_only = (data[4] == 0xF0);
    
    if (data.size() > 8) {
        state_.timer1 = (data[5] << 8) | data[6];
        state_.timer2 = (data[7] << 8) | data[8];
    }
    
    if (data.size() > 12) {
        state_.max_brightness1 = (data[9] << 8) | data[10];
        state_.min_brightness1 = (data[11] << 8) | data[12];
    }
    if (data.size() > 16) {
        state_.max_brightness2 = (data[13] << 8) | data[14];
        state_.min_brightness2 = (data[15] << 8) | data[16];
    }
    
    if (data.size() > 38 && data[38] >= 0x18 && data[38] <= 0x1E) {
        state_.external_input = data[38];
    }
    
    update_binary_sensors();
    state_callback_->trigger(state_);
}

void UjinComponent::parse_debug_log(const std::vector<uint8_t> &data) {
    if (data.size() < 10) return;
    
    ESP_LOGD(TAG, "Debug log from PIC16: %s", format_hex_pretty(data).c_str());
    
    if (data.size() > 4) {
        uint8_t input_state = data[4];
        if (input_state >= 0x18 && input_state <= 0x1E) {
            state_.external_input = input_state;
            update_binary_sensors();
            state_callback_->trigger(state_);
        }
    }
}

void UjinComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "Ujin Dimmer Component:");
    ESP_LOGCONFIG(TAG, "  Channel 1: %s, Brightness: %d%%", 
                  state_.channel1_on ? "ON" : "OFF", state_.channel1_brightness);
    ESP_LOGCONFIG(TAG, "  Channel 2: %s, Brightness: %d%%", 
                  state_.channel2_on ? "ON" : "OFF", state_.channel2_brightness);
    ESP_LOGCONFIG(TAG, "  Dimming: CH1=%s, CH2=%s", 
                  state_.dimmer1_enabled ? "enabled" : "disabled",
                  state_.dimmer2_enabled ? "enabled" : "disabled");
    ESP_LOGCONFIG(TAG, "  External input: 0x%02X", state_.external_input);
}

} // namespace ujin
} // namespace esphome
