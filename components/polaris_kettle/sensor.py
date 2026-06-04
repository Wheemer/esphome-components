import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID

from . import polaris_kettle_ns, PolarisKettle

DEPENDENCIES = ["polaris_kettle"]

PolarisCurrentTemperatureSensor = polaris_kettle_ns.class_(
    "PolarisCurrentTemperatureSensor", sensor.Sensor, cg.Component
)

PolarisTargetTemperatureSensor = polaris_kettle_ns.class_(
    "PolarisTargetTemperatureSensor", sensor.Sensor, cg.Component
)

CONF_POLARIS_KETTLE_ID = "polaris_kettle_id"

CURRENT_TEMP_SCHEMA = sensor.sensor_schema(
    unit_of_measurement="°C",
    accuracy_decimals=1,
    icon="mdi:thermometer"
).extend({
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
})

TARGET_TEMP_SCHEMA = sensor.sensor_schema(
    unit_of_measurement="°C",
    accuracy_decimals=1,
    icon="mdi:thermometer"
).extend({
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
})

async def register_current_temp_sensor(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_current_temperature_sensor(var))

async def register_target_temp_sensor(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_target_temperature_sensor(var))
