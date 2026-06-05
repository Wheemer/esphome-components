import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID

from . import polaris_kettle_ns, PolarisKettle, CONF_POLARIS_KETTLE_ID

DEPENDENCIES = ["polaris_kettle"]

PolarisTextSensor = polaris_kettle_ns.class_(
    "PolarisTextSensor", text_sensor.TextSensor, cg.Component
)

CONFIG_SCHEMA = text_sensor.text_sensor_schema(
    icon="mdi:kettle"
).extend({
    cv.GenerateID(CONF_ID): cv.declare_id(PolarisTextSensor),
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_mode_text_sensor(var))
