import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_TYPE

from . import generate
from .const import CONF_POLARIS_ID

DEPENDENCIES = ["polaris"]

CONF_SENSOR_TYPE = "type"
SENSOR_TYPES = ["no_kettle", "no_water"]

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema().extend({
    cv.GenerateID(CONF_ID): cv.declare_id(binary_sensor.BinarySensor),
    cv.GenerateID(CONF_POLARIS_ID): cv.use_id(generate.PolarisKettle),
    cv.Required(CONF_SENSOR_TYPE): cv.one_of(*SENSOR_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_POLARIS_ID])
    
    sensor_type = config[CONF_SENSOR_TYPE]
    if sensor_type == "no_kettle":
        cg.add(parent.set_no_kettle_binary_sensor(var))
    else:
        cg.add(parent.set_no_water_binary_sensor(var))
