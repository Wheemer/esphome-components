#include "high_voltage_control.h"

namespace esphome {
namespace rad_sens {

void HighVoltageControl::setup() {
  optional<bool> initial_state = this->get_initial_state_with_restore_mode();
  if (initial_state.has_value()) {
    if (initial_state.value()) {
      this->turn_on();
    } else {
      this->turn_off();
    }
  } else {
    this->publish_state(this->parent_->get_hv_generator_state());
  }
}

void HighVoltageControl::write_state(bool state) {
  this->parent_->set_hv_generator(state);
  this->publish_state(state);
}

}  // namespace rad_sens
}  // namespace esphome
