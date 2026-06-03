import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID

from . import RadSensComponent

DEPENDENCIES = ['rad_sens']

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if "sensitivity" in config:
        sens_config = config["sensitivity"]
        
        # Создаём number с авто-генерируемым ID
        num = cg.new_Pvariable(cg.global_id("rad_sens_sens"))
        cg.add(num.set_min_value(sens_config["min_value"]))
        cg.add(num.set_max_value(sens_config["max_value"]))
        cg.add(num.set_step(sens_config["step"]))
        cg.add(num.set_unit_of_measurement(sens_config["unit_of_measurement"]))
        cg.add(num.set_icon(sens_config["icon"]))
        
        await number.register_number(num, sens_config)
        cg.add(parent.set_sensitivity_number(num))

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(RadSensComponent),
    cv.Optional("sensitivity"): cv.Schema({
        cv.Optional("name"): cv.string,
        cv.Optional("min_value", default=100): cv.float_,
        cv.Optional("max_value", default=1100): cv.float_,
        cv.Optional("step", default=1): cv.float_,
        cv.Optional("unit_of_measurement", default="imp/µR"): cv.string,
        cv.Optional("icon", default="mdi:tune"): cv.icon,
    }),
})
