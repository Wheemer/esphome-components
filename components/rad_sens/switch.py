import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID

from . import RadSensComponent

DEPENDENCIES = ['rad_sens']

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(RadSensComponent),
    cv.Optional("hv_generator_switch"): switch.switch_schema(class_=switch.Switch),
    cv.Optional("led_switch"): switch.switch_schema(class_=switch.Switch),
    cv.Optional("low_power_switch"): switch.switch_schema(class_=switch.Switch),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if "hv_generator_switch" in config:
        sw = await switch.new_switch(config["hv_generator_switch"])
        cg.add(parent.set_hv_generator_switch(sw))
    
    if "led_switch" in config:
        sw = await switch.new_switch(config["led_switch"])
        cg.add(parent.set_led_switch(sw))
    
    if "low_power_switch" in config:
        sw = await switch.new_switch(config["low_power_switch"])
        cg.add(parent.set_low_power_switch(sw))
