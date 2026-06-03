import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    ICON_RADIOACTIVE,
    ICON_CHIP,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_NONE,
)

from . import RadSensComponent

DEPENDENCIES = ['rad_sens']

CONF_DYNAMIC_INTENSITY = "dynamic_intensity"
CONF_STATIC_INTENSITY = "static_intensity"
CONF_PULSES = "pulses"
CONF_FIRMWARE_VERSION = "firmware_version"

UNIT_MICROROENTGEN_PER_HOUR = "µR/h"
UNIT_COUNT_PER_MINUTE = "count/min"

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
    cv.Optional(CONF_PULSES): sensor.sensor_schema(
        unit_of_measurement=UNIT_COUNT_PER_MINUTE,
        icon=ICON_RADIOACTIVE,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_FIRMWARE_VERSION): sensor.sensor_schema(
        unit_of_measurement="",
        icon=ICON_CHIP,
        accuracy_decimals=0,
        state_class=STATE_CLASS_NONE,
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
    
    if CONF_PULSES in config:
        sens = await sensor.new_sensor(config[CONF_PULSES])
        cg.add(parent.set_pulses_sensor(sens))
    
    if CONF_FIRMWARE_VERSION in config:
        sens = await sensor.new_sensor(config[CONF_FIRMWARE_VERSION])
        cg.add(parent.set_firmware_version_sensor(sens))
