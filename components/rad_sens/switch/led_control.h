#pragma once

#include "esphome/components/switch/switch.h"
#include "../rad_sens.h"

namespace esphome {
namespace rad_sens {

class LedControl : public switch_::Switch, public Parented<RadSensComponent> {
 public:
  LedControl() = default;
  void setup() override;

 protected:
  void write_state(bool state) override;
};

}  // namespace rad_sens
}  // namespace esphome
