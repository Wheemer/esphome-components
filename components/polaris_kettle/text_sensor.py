import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID

from . import polaris_kettle_ns, PolarisKettle

DEPENDENCIES = ["polaris_kettle"]

PolarisModeTextSensor = polaris_kettle_ns.class_(
    "PolarisModeTextSensor", text_sensor.TextSensor, cg.Component
)

CONF_POLARIS_KETTLE_ID = "polaris_kettle_id"

MODE_SCHEMA = text_sensor.text_sensor_schema(
    icon="mdi:kettle"
).extend({
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
})

async def register_mode_text_sensor(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_mode_text_sensor(var))
