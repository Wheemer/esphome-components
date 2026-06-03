#include "led_control.h"

namespace esphome {
namespace rad_sens {

void LedControl::setup() {
  optional<bool> initial_state = this->get_initial_state_with_restore_mode();
  if (initial_state.has_value()) {
    if (initial_state.value()) {
      this->turn_on();
    } else {
      this->turn_off();
    }
  } else {
    this->publish_state(this->parent_->get_led_state());
  }
}

void LedControl::write_state(bool state) {
  this->parent_->set_led(state);
  this->publish_state(state);
}

}  // namespace rad_sens
}  // namespace esphome
