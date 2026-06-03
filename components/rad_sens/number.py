import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID

from . import RadSensComponent

DEPENDENCIES = ['rad_sens']

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(RadSensComponent),
    cv.Optional("sensitivity"): number.NUMBER_SCHEMA.extend({
        cv.Optional("min_value", default=100): cv.float_,
        cv.Optional("max_value", default=1100): cv.float_,
        cv.Optional("step", default=1): cv.float_,
        cv.Optional("unit_of_measurement", default="imp/µR"): cv.string_strict,
        cv.Optional("icon", default="mdi:tune"): cv.icon,
    }),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if "sensitivity" in config:
        sens_config = config["sensitivity"]
        
        # Создаём number
        num = await number.new_number(
            sens_config,
            min_value=sens_config["min_value"],
            max_value=sens_config["max_value"],
            step=sens_config["step"]
        )
        
        cg.add(parent.set_sensitivity_number(num))
        )
        
        cg.add(parent.set_sensitivity_number(num))
