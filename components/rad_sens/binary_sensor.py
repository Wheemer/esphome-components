import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import RadSensComponent

DEPENDENCIES = ['rad_sens']

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(RadSensComponent),
    cv.Optional("hv_generator_state"): binary_sensor.binary_sensor_schema(),
})

async def to_code(config):
    parent = await cg.get_variable(config["id"])
    
    if "hv_generator_state" in config:
        bs = await binary_sensor.new_binary_sensor(config["hv_generator_state"])
        cg.add(parent.set_hv_generator_state_sensor(bs))
