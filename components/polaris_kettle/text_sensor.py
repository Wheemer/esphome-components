import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID

from . import generate
from .const import CONF_POLARIS_ID

DEPENDENCIES = ["polaris"]

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend({
    cv.GenerateID(CONF_ID): cv.declare_id(text_sensor.TextSensor),
    cv.GenerateID(CONF_POLARIS_ID): cv.use_id(generate.PolarisKettle),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_POLARIS_ID])
    cg.add(parent.set_mode_text_sensor(var))
