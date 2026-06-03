#pragma once

#include "esphome/components/switch/switch.h"
#include "../rad_sens.h"

namespace esphome {
namespace rad_sens {

class HighVoltageControl : public switch_::Switch, public Parented<RadSensComponent> {
 public:
  HighVoltageControl() = default;
  void setup();

 protected:
  void write_state(bool state) override;
};

}  // namespace rad_sens
}  // namespace esphome
