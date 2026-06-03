import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    ICON_RADIOACTIVE,
    STATE_CLASS_MEASUREMENT,
)

from . import RadSensComponent
from .const import (
    UNIT_MICROROENTGEN_PER_HOUR,
    UNIT_COUNT_PER_MINUTE,
    CONF_DYNAMIC_INTENSITY,
    CONF_STATIC_INTENSITY,
    CONF_COUNTS_PER_MINUTE,
)

DEPENDENCIES = ['rad_sens']

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(RadSensComponent),
    cv.Optional(CONF_DYNAMIC_INTENSITY): sensor.sensor_schema(
        unit_of_measurement=UNIT_MICROROENTGEN_PER_HOUR,
        icon=ICON_RADIOACTIVE,
        accuracy_decimals=3,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_STATIC_INTENSITY): sensor.sensor_schema(
        unit_of_measurement=UNIT_MICROROENTGEN_PER_HOUR,
        icon=ICON_RADIOACTIVE,
        accuracy_decimals=3,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_COUNTS_PER_MINUTE): sensor.sensor_schema(
        unit_of_measurement=UNIT_COUNT_PER_MINUTE,
        icon=ICON_RADIOACTIVE,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if CONF_DYNAMIC_INTENSITY in config:
        sens = await sensor.new_sensor(config[CONF_DYNAMIC_INTENSITY])
        cg.add(parent.set_dynamic_intensity_sensor(sens))
    
    if CONF_STATIC_INTENSITY in config:
        sens = await sensor.new_sensor(config[CONF_STATIC_INTENSITY])
        cg.add(parent.set_static_intensity_sensor(sens))
    
    if CONF_COUNTS_PER_MINUTE in config:
        sens = await sensor.new_sensor(config[CONF_COUNTS_PER_MINUTE])
        cg.add(parent.set_pulses_sensor(sens))
