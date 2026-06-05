import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_TYPE

from . import polaris_kettle_ns, PolarisKettle, CONF_POLARIS_KETTLE_ID

DEPENDENCIES = ["polaris_kettle"]

PolarisBinarySensor = polaris_kettle_ns.class_(
    "PolarisBinarySensor", binary_sensor.BinarySensor, cg.Component
)

CONF_SENSOR_TYPE = "type"
SENSOR_TYPES = {
    "no_kettle": "no_kettle",
    "no_water": "no_water",
}

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema().extend({
    cv.GenerateID(CONF_ID): cv.declare_id(PolarisBinarySensor),
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
    cv.Required(CONF_SENSOR_TYPE): cv.enum(SENSOR_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    
    sensor_type = config[CONF_SENSOR_TYPE]
    if sensor_type == "no_kettle":
        cg.add(parent.register_no_kettle_sensor(var))
    else:
        cg.add(parent.register_no_water_sensor(var))
