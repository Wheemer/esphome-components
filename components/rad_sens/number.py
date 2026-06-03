import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID

from . import RadSensComponent

DEPENDENCIES = ['rad_sens']

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(RadSensComponent),
    cv.Optional("sensitivity"): number._NUMBER_SCHEMA,
    cv.Optional("min_value", default=100): cv.float_,
    cv.Optional("max_value", default=1100): cv.float_,
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if "sensitivity" in config:
        sens_config = config["sensitivity"]
        
        # Используем стандартную функцию new_number
        # Она сама создаст ID, если его нет
        num = await number.new_number(
            sens_config,
            min_value=100,
            max_value=1100,
            step=1
        )
        
        cg.add(parent.set_sensitivity_number(num))
