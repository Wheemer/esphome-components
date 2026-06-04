import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID

from . import polaris_kettle_ns, PolarisKettle

DEPENDENCIES = ["polaris_kettle"]

PolarisNoKettleBinarySensor = polaris_kettle_ns.class_(
    "PolarisNoKettleBinarySensor", binary_sensor.BinarySensor, cg.Component
)

PolarisNoWaterBinarySensor = polaris_kettle_ns.class_(
    "PolarisNoWaterBinarySensor", binary_sensor.BinarySensor, cg.Component
)

CONF_POLARIS_KETTLE_ID = "polaris_kettle_id"

NO_KETTLE_SCHEMA = binary_sensor.binary_sensor_schema(
    icon="mdi:kettle-off"
).extend({
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
})

NO_WATER_SCHEMA = binary_sensor.binary_sensor_schema(
    icon="mdi:water-off"
).extend({
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
})

async def register_no_kettle_sensor(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_no_kettle_binary_sensor(var))

async def register_no_water_sensor(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_no_water_binary_sensor(var))
