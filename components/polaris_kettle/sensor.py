import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_TYPE

from . import generate
from .const import CONF_POLARIS_ID

DEPENDENCIES = ["polaris"]

CONF_SENSOR_TYPE = "type"
SENSOR_TYPES = ["current", "target"]

CONFIG_SCHEMA = sensor.sensor_schema().extend({
    cv.GenerateID(CONF_ID): cv.declare_id(sensor.Sensor),
    cv.GenerateID(CONF_POLARIS_ID): cv.use_id(generate.PolarisKettle),
    cv.Required(CONF_SENSOR_TYPE): cv.one_of(*SENSOR_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_POLARIS_ID])
    
    sensor_type = config[CONF_SENSOR_TYPE]
    if sensor_type == "current":
        cg.add(parent.set_current_temperature_sensor(var))
    else:
        cg.add(parent.set_target_temperature_sensor(var))
